#include "yahttp.hpp"

namespace YaHTTP {
  void Request::build(const std::string &method, const std::string &url, const std::string &params) {
    this->method = method;
    std::transform(this->method.begin(), this->method.end(), this->method.begin(), ::toupper);
    this->url.parse(url);
    this->headers["host"] = this->url.host;
    this->headers["connection"] = "close";
    this->headers["user-agent"] = "yahttp 0.1";
    this->headers["accept"] = "*/*";
    if (params.empty() == false) {
      this->headers["content-type"] = "application/x-www-form-urlencoded; charset=utf-8";
      this->headers["content-length"] = params.size();
      this->body = params;
    } else {
      this->body = "";
    }
  };

  Request::Request() {};
  Request::Request(const Response &resp) {
    method = resp.method;
    url = resp.url;
    cookies = resp.cookies;
  };
  Request::Request(const Request &req) {
    method = req.method;
    url = req.url;
    parameters = req.parameters;
    headers = req.headers;
    cookies = req.cookies;
    body = req.body;
  };
  Request::~Request() {};

  Response::Response() {};
  Response::Response(const Request &req) {
    headers["connection"] = "close";
    method = req.method;
    url = req.url;
    cookies = req.cookies;
    status = 200;
  };
  Response::Response(const Response &resp) {
    method = resp.method;
    url = resp.url;
    parameters = resp.parameters;
    headers = resp.headers;
    cookies = resp.cookies;
    body = resp.body;
    status = resp.status;
    statusText = resp.statusText;
  };
  Response::~Response() {};

  void Response::load(std::istream &is) {
    AsyncResponseLoader arl(this);
    while(is.good()) {
      char buf[1024];
      is.read(buf, 1024);
      if (is.gcount()) { // did we actually read anything 
        is.clear();
        if (arl.feed(std::string(buf, is.gcount())) == true) return; // completed
      }
    }
    if (arl.ready()) arl.finalize();
  };

  void Response::write(std::ostream &os) const { 
    os << "HTTP/1.1 " << status << " ";
    if (statusText.empty()) 
      os << Utility::status2text(status);
    else
      os << statusText;
    os << "\r\n";

    // write headers
    strstr_map_t::const_iterator iter = headers.begin();
    while(iter != headers.end()) {
      os << Utility::camelizeHeader(iter->first) << ": " << iter->second << "\r\n";
      iter++;
    }
    if (cookies.size() > 0) { // write cookies
      for(strcookie_map_t::const_iterator i = cookies.begin(); i != cookies.end(); i++)
        os << "Set-Cookie: " << i->second.str() << "\r\n";
    }
    os << "\r\n";
    os << body;
  };
 
  void Request::load(std::istream &is) {
    AsyncRequestLoader arl(this);
    while(is.good()) {
      char buf[1024];
      is.read(buf, 1024);
      if (is.gcount()) { // did we actually read anything
        is.clear();
        if (arl.feed(std::string(buf, is.gcount())) == true) return; // completed
      }
    }
  };

  void Request::write(std::ostream &os) const {
    os << method << " " << url.path << " HTTP/1.1" << "\r\n";
    strstr_map_t::const_iterator iter = headers.begin();
    while(iter != headers.end()) {
      os << Utility::camelizeHeader(iter->first) << ": " << iter->second << "\r\n";
      iter++;
    }
    if (cookies.size() > 0) { // write cookies
      for(strcookie_map_t::const_iterator i = cookies.begin(); i != cookies.end(); i++) 
        os << "Cookie: " << i->second.str() << "\r\n";
    } 
    os << "\r\n";
    if (body.size()>0) {
      os << body;
    }
  };

  std::ostream& operator<<(std::ostream& os, const Response &resp) {
    resp.write(os);
    return os;
  };

  std::istream& operator>>(std::istream& is, Response &resp) {
    resp.load(is);
    return is;
  };

  std::ostream& operator<<(std::ostream& os, const Request &req) {
    req.write(os);
    return os;
  };

  std::istream& operator>>(std::istream& is, Request &req) {
    req.load(is);
    return is;
  };

  bool AsyncRequestLoader::feed(const std::string &somedata) {
    size_t pos;

    buffer.append(somedata);
    while(state < 2) {
      int cr=0;
      // need to find CRLF in buffer
      if ((pos = buffer.find_first_of("\n")) == std::string::npos) return false;
      if (buffer[pos-1]=='\r')
        cr=1;
      std::string line(buffer.begin(), buffer.begin()+pos-cr); // exclude CRLF
      buffer.erase(buffer.begin(), buffer.begin()+pos+1+cr); // remove line from buffer including CRLF

      if (state == 0) { // startup line
        std::string ver;
        std::string tmpurl;
        std::istringstream iss(line);
        iss >> request->method >> tmpurl >> ver;
        if (ver.find("HTTP/1.") != 0)
          throw ParseError("Not a HTTP 1.x request");
        // uppercase the request method
        std::transform(request->method.begin(), request->method.end(), request->method.begin(), ::toupper);
        request->url.parse(tmpurl);
        request->parameters = Utility::parseUrlParameters(request->url.parameters);
        state = 1;
      } else if (state == 1) {
        std::string key,value;
        size_t pos;
        if (line.empty()) {
          chunked = (request->headers.find("transfer-encoding") != request->headers.end() && request->headers["transfer-encoding"] == "chunked");
          // host header is optional
          if (request->headers.find("host") != request->headers.end())
            request->url.host = request->headers["host"];
               
          state = 2;
          break;
        }
        // split headers
        if ((pos = line.find_first_of(": ")) == std::string::npos)
          throw ParseError("Malformed line");
        key = line.substr(0, pos);
        value = line.substr(pos+2);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        request->headers[key] = value;
      }
    }

    // do we have content-length? 
    if (!chunked) {
      if (request->headers.find("content-length") != request->headers.end()) {
        std::istringstream maxbodyS(request->headers["content-length"]);
        maxbodyS >> maxbody;
      }
      if (maxbody < 1) return true; // guess there isn't anything left.
      if (maxbody > YAHTTP_MAX_REQUEST_SIZE) 
        throw ParseError("Request size exceeded");
    }

    if (buffer.size() == 0) return false;

    while(buffer.size() > 0 && bodybuf.str().size() < static_cast<size_t>(maxbody)) {
      char buf[1024] = {0};

      if (chunked) {
        if (chunk_size == 0) {
          // read chunk length
          if ((pos = buffer.find('\n')) == std::string::npos) return false;
          if (pos > 1023)
            throw ParseError("Impossible chunk_size");
          buffer.copy(buf, pos);
          buf[pos]=0; // just in case...
          buffer.erase(buffer.begin(), buffer.begin()+pos+1); // remove line from buffer
          sscanf(buf, "%x", &chunk_size);
          if (!chunk_size) break; // last chunk
        } else {
          if (buffer.size() < static_cast<size_t>(chunk_size+1)) return false; // expect newline
          if (buffer.at(chunk_size) != '\n') return false; // there should be newline.
          buffer.copy(buf, chunk_size);
          buffer.erase(buffer.begin(), buffer.begin()+chunk_size+1);
          bodybuf << buf;
          chunk_size = 0;
          if (buffer.size() == 0) return false; // just in case
        }
      } else {
        bodybuf << buffer;
        buffer = "";
      }
    }

    if (chunk_size!=0) return false; // need more data

    bodybuf.flush();
    request->body = bodybuf.str();
    bodybuf.str("");

    return true;
  };

  bool AsyncResponseLoader::feed(const std::string &somedata) {
    size_t pos;
    buffer.append(somedata);
    while(state < 2) {
      int cr=0;
      // need to find CRLF in buffer
      if ((pos = buffer.find_first_of("\n")) == std::string::npos) return false;
      if (buffer[pos-1]=='\r') 
        cr=1;
      std::string line(buffer.begin(), buffer.begin()+pos-cr); // exclude CRLF
      buffer.erase(buffer.begin(), buffer.begin()+pos+cr); // remove line from buffer including CRLF

      if (state == 0) { // startup line
        std::string ver;
        std::istringstream iss(line);
        iss >> ver >> response->status >> response->statusText;
        if (ver.find("HTTP/1.") != 0)
          throw ParseError("Not a HTTP 1.x response");
        state = 1;
      } else if (state == 1) {
        std::string key,value;
        size_t pos;
        if (line.empty()) {
          chunked = (response->headers.find("transfer-encoding") != response->headers.end() && response->headers["transfer-encoding"] == "chunked");
          state = 2;
          break;
        }
        // split headers
        if ((pos = line.find_first_of(": ")) == std::string::npos)
          throw ParseError("Malformed header line");
        key = line.substr(0, pos);
        value = line.substr(pos+2);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower); 
        // is it already defined

        if (key == "set-cookie") {
          parseCookies(value);
        } else {
          if (response->headers.find(key) != response->headers.end()) {
            response->headers[key] = response->headers[key] + ";" + value;
          } else {
            response->headers[key] = value;
          }
        }
      }
    }

    // check for expected body size
    maxbody = -1;
    if (!chunked) {
      if (response->headers.find("content-length") != response->headers.end()) {
        std::istringstream maxbodyS(response->headers["content-length"]);
        maxbodyS >> maxbody;
      }
      if (maxbody < 1) return true; // guess there isn't anything left.
      if (maxbody > YAHTTP_MAX_RESPONSE_SIZE)
        throw ParseError("Respnse size exceeded");
    }

    if (buffer.size() == 0) return false;
      
    while(buffer.size() > 0) {
      char buf[1024] = {0};

      if (chunked) {
        if (chunk_size == 0) {
          // read chunk length
          if ((pos = buffer.find('\n')) == std::string::npos) return false;
          if (pos > 1023) 
            throw ParseError("Impossible chunk_size");
          buffer.copy(buf, pos);
          buf[pos]=0; // just in case...
          buffer.erase(buffer.begin(), buffer.begin()+pos+1); // remove line from buffer
          sscanf(buf, "%x", &chunk_size);
          if (!chunk_size) break; // last chunk
        } else {
          if (buffer.size() < static_cast<size_t>(chunk_size+1)) return false; // expect newline
          if (buffer.at(chunk_size) != '\n') return false; // there should be newline.
          buffer.copy(buf, chunk_size);
          buffer.erase(buffer.begin(), buffer.begin()+chunk_size+1);
          bodybuf << buf;
          chunk_size = 0;
          if (buffer.size() == 0) break; // just in case
        }
      } else {
        bodybuf << buffer;
        buffer = "";
      }
    }

    if (chunk_size!=0) return false; // need more data

    return ready();
  };

  void AsyncResponseLoader::finalize() {
    bodybuf.flush();
    response->body = bodybuf.str();
    bodybuf.str("");
  };

  void AsyncResponseLoader::keyValuePair(const std::string &keyvalue, std::string &key, std::string &value) {
    size_t pos;
    pos = keyvalue.find("=");
    if (pos == std::string::npos) throw "Not a Key-Value pair (cookie)";
    key = std::string(keyvalue.begin(), keyvalue.begin()+pos);
    value = std::string(keyvalue.begin()+pos+1, keyvalue.end());
  }

  void AsyncResponseLoader::parseCookies(const std::string &cookiestr) {
    std::list<Cookie> cookies;
    int cstate = 0; //cookiestate
    size_t pos,npos;
    pos = 0;
    cstate = 0;
    while(pos < cookiestr.size()) {
      if (cookiestr.compare(pos, 7, "expires") ==0 ||
          cookiestr.compare(pos, 6, "domain")  ==0 ||
          cookiestr.compare(pos, 4, "path")    ==0) {
        cstate = 1;
        // get the date
        std::string key, value, s;
        npos = cookiestr.find("; ", pos);
        if (npos == std::string::npos) {
          // last value
          s = std::string(cookiestr.begin() + pos + 1, cookiestr.end());
          pos = cookiestr.size();
        } else {
          s = std::string(cookiestr.begin() + pos + 1, cookiestr.begin() + npos - 1);
          pos = npos+2;
        }
        keyValuePair(s, key, value);
        if (s == "expires") {
          DateTime dt;
          dt.parseCookie(value);
          for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
            i->expires = dt;
        } else if (s == "domain") {
          for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
            i->domain = value;
        } else if (s == "path") {
          for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
            i->path = value;
        }
      } else if (cookiestr.compare(pos, 8, "httpOnly")==0) {
        cstate = 1;
        for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
          i->httponly = true;
      } else if (cookiestr.compare(pos, 6, "secure")  ==0) {
        cstate = 1; 
        for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
          i->secure = true;
      } else if (cstate == 0) { // expect cookie
        Cookie c;
        std::string s;
        npos = cookiestr.find("; ", pos);
        if (npos == std::string::npos) {
          // last value
          s = std::string(cookiestr.begin() + pos, cookiestr.end());
          pos = cookiestr.size();
        } else {
          s = std::string(cookiestr.begin() + pos, cookiestr.begin() + npos);
          pos = npos+2;
        }
        keyValuePair(s, c.name, c.value);
        cookies.push_back(c);
      } else if (cstate == 1) {
        // ignore crap
        break;
      }
    } 

    // store cookies
    for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++) {
      response->cookies[i->name] = *i;
    }
  };

  void AsyncRequestLoader::keyValuePair(const std::string &keyvalue, std::string &key, std::string &value) {
    size_t pos;
    pos = keyvalue.find("=");
    if (pos == std::string::npos) throw "Not a Key-Value pair (cookie)";
    key = std::string(keyvalue.begin(), keyvalue.begin()+pos);
    value = std::string(keyvalue.begin()+pos+1, keyvalue.end());
  }

  void AsyncRequestLoader::parseCookies(const std::string &cookiestr) {
    std::list<Cookie> cookies;
    int cstate = 0; //cookiestate
    size_t pos,npos;
    pos = 0;
    cstate = 0;
    while(pos < cookiestr.size()) {
      if (cookiestr.compare(pos, 7, "expires") ==0 ||
          cookiestr.compare(pos, 6, "domain")  ==0 ||
          cookiestr.compare(pos, 4, "path")    ==0) {
        cstate = 1;
        // get the date
        std::string key, value, s;
        npos = cookiestr.find("; ", pos);
        if (npos == std::string::npos) {
          // last value
          s = std::string(cookiestr.begin() + pos + 1, cookiestr.end());
          pos = cookiestr.size();
        } else {
          s = std::string(cookiestr.begin() + pos + 1, cookiestr.begin() + npos - 1);
          pos = npos+2;
        }
        keyValuePair(s, key, value);
        if (s == "expires") {
          DateTime dt;
          dt.parseCookie(value);
          for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
            i->expires = dt;
        } else if (s == "domain") {
          for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
            i->domain = value;
        } else if (s == "path") {
          for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
            i->path = value;
        }
      } else if (cookiestr.compare(pos, 8, "httpOnly")==0) {
        cstate = 1;
        for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
          i->httponly = true;
      } else if (cookiestr.compare(pos, 6, "secure")  ==0) {
        cstate = 1;
        for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++)
          i->secure = true;
      } else if (cstate == 0) { // expect cookie
        Cookie c;
        std::string s;
        npos = cookiestr.find("; ", pos);
        if (npos == std::string::npos) {
          // last value
          s = std::string(cookiestr.begin() + pos, cookiestr.end());
          pos = cookiestr.size();
        } else {
          s = std::string(cookiestr.begin() + pos, cookiestr.begin() + npos);
          pos = npos+2;
        }
        keyValuePair(s, c.name, c.value);
        cookies.push_back(c);
      } else if (cstate == 1) {
        // ignore crap
        break;
      }
    }

    // store cookies
    for(std::list<Cookie>::iterator i = cookies.begin(); i != cookies.end(); i++) {
      request->cookies[i->name] = *i;
    }
  };


};
