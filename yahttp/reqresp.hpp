#include <map>
#include <iostream>
#include <locale>
#include <algorithm>

#include "url.hpp"
#include "utility.hpp"
#include "exception.hpp"

namespace YaHTTP {
  typedef std::map<std::string,std::string> strstr_map_t;

  class Response;
  class Request;
  class AsyncResponseLoader;

  class Request {
  public:
     Request();
     Request(const Response &resp);
     Request(const Request &req);
     ~Request();
     void load(std::istream &is);
     void write(std::ostream &os) const;

     strstr_map_t headers;
     strstr_map_t parameters;
     strstr_map_t cookies;

     URL url;
     std::string method;
     std::string body;
     friend std::ostream& operator<<(std::ostream& os, const Request &req);
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

  void Request::load(std::istream &is) {  };
  void Request::write(std::ostream &os) const { os << "FIXME"; };

  std::ostream& operator<<(std::ostream& os, const Request &req) {
    req.write(os);
    return os;
  };

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

  std::ostream& operator<<(std::ostream& os, Response &resp) {
     resp.write(os);
     return os;
  };

  std::istream& operator>>(std::istream& is, Response &resp) {
     resp.load(is);
     return is;
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
