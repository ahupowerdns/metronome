#include <map>
#include <iostream>
#include <locale>
#include <algorithm>

#include "url.hpp"
#include "utility.hpp"
#include "exception.hpp"

#ifndef YAHTTP_MAX_REQUEST_SIZE 
#define YAHTTP_MAX_REQUEST_SIZE 2097152
#endif

namespace YaHTTP {
  typedef std::map<std::string,std::string> strstr_map_t;

  class Response;
  class Request;
  class AsyncResponseLoader;
  class AsyncRequestLoader;

  class Request {
  public:
     Request();
     Request(const Response &resp);
     Request(const Request &req);
     ~Request();

     void build(const std::string &method, const std::string &url, const std::string &params) {
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

     void load(std::istream &is);
     void write(std::ostream &os) const;

     strstr_map_t headers;
     strstr_map_t parameters;
     strstr_map_t cookies;

     URL url;
     std::string method;
     std::string body;

     friend std::istream& operator>>(std::istream& os, const Request &req);
     friend std::ostream& operator<<(std::ostream& os, const Request &req);
     friend class AsyncRequestLoader;
  };

  class Response {
  public:
     Response();
     Response(const Request &req);
     Response(const Response &resp);
     ~Response();
     void load(std::istream &is);
     void write(std::ostream &os) const;

     strstr_map_t headers;
     strstr_map_t parameters;
     strstr_map_t cookies;

     URL url;
     int status;
     std::string statusText;
     std::string method;
     std::string body;

     friend std::istream& operator>>(std::istream& is, Response &resp);
     friend std::ostream& operator<<(std::ostream& os, const Response &resp);
     friend class AsyncResponseLoader;
  };

  class AsyncResponseLoader {
  public:
    AsyncResponseLoader(Response *response) {
      state = 0;
      chunked = false;
      chunk_size = 0;
      this->response = response;
    };
    bool feed(const std::string &somedata);
  private:
    Response *response;
    int state;
    std::string buffer;
    bool chunked;
    int chunk_size;
    std::ostringstream bodybuf;
  };

  class AsyncRequestLoader {
  public:
    AsyncRequestLoader(Request *request) {
      state = 0;
      chunked = false;
      chunk_size = 0;
      maxbody = 0;
      this->request = request;
    };
    bool feed(const std::string &somedata);
  private:
    Request *request;
    int state;
    std::string buffer;
    bool chunked;
    int chunk_size;
    std::ostringstream bodybuf;
    long maxbody;
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
     // FIXME: parse cookies
  };

  void Response::write(std::ostream &os) const { 
     os << "HTTP/1.1 " << status << " ";
     if (statusText.empty()) 
       os << Utility::status2text(status);
     else
       os << statusText;
     os << std::endl;

     // write headers
     strstr_map_t::const_iterator iter = headers.begin();
     while(iter != headers.end()) {
        os << Utility::camelizeHeader(iter->first) << ": " << iter->second << std::endl;
        iter++;
     }
     os << std::endl;
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
     os << method << " " << url.path << " HTTP/1.1" << std::endl;
     strstr_map_t::const_iterator iter = headers.begin();
     while(iter != headers.end()) {
        os << Utility::camelizeHeader(iter->first) << ": " << iter->second << std::endl;
        iter++;
     }
     os << std::endl;
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
          // need to find newline in buffer
          if ((pos = buffer.find('\n')) == std::string::npos) return false;
          std::string line(buffer.begin(), buffer.begin()+pos); // exclude newline
          buffer.erase(buffer.begin(), buffer.begin()+pos+1); // remove line from buffer including newline
          if (state == 0) { // startup line
             std::string ver;
             std::string tmpurl;
             std::istringstream iss(line);
             iss >> request->method >> tmpurl >> ver;
             if (ver != "HTTP/1.1")
                throw ParseError("Not a HTTP 1.1 request");
             // uppercase the request method
             std::transform(request->method.begin(), request->method.end(), request->method.begin(), ::toupper);
             request->url.parse(tmpurl);
             state = 1;
          } else if (state == 1) {
             std::string key,value;
             size_t pos;
             if (line.empty()) {
               chunked = (request->headers.find("transfer-encoding") != request->headers.end() && request->headers["transfer-encoding"] == "chunked");
               // validate certain headers
               if (request->headers.find("host") == request->headers.end()) 
                  throw ParseError("Missing Host header");
               else 
                  request->url.host = request->headers["host"];
               // parse url
               
               state = 2;
               break;
             }
             // split headers
             if ((pos = line.find_first_of(": ")) == std::string::npos)
               throw ParseError("Malformed hader");
             key = line.substr(0, pos);
             value = line.substr(pos+2);
             std::transform(key.begin(), key.end(), key.begin(), ::tolower);
             request->headers[key] = value;
          }
       }

       if (request->method != "POST") 
          return true;
       
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
          // need to find newline in buffer
          if ((pos = buffer.find('\n')) == std::string::npos) return false;
          std::string line(buffer.begin(), buffer.begin()+pos); // exclude newline
          buffer.erase(buffer.begin(), buffer.begin()+pos+1); // remove line from buffer including newline
          if (state == 0) { // startup line
             std::string ver;
             std::istringstream iss(line);
             iss >> ver >> response->status >> response->statusText;
             if (ver != "HTTP/1.1") 
                throw ParseError("Not a HTTP response");
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
               throw ParseError("Malformed hader");
             key = line.substr(0, pos);
             value = line.substr(pos+2);
             std::transform(key.begin(), key.end(), key.begin(), ::tolower);
             response->headers[key] = value;
          }
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
             if (buffer.size() == 0) return false; // just in case
           }
         } else {
             bodybuf << buffer;
             buffer = "";
         }
       }

       if (chunk_size!=0) return false; // need more data

       bodybuf.flush();
       response->body = bodybuf.str();
       bodybuf.str("");
       return true;
  };
};
