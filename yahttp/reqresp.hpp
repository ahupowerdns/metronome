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
     int state = 0;
     std::ostringstream bodybuf;

     while(is.good()) {
        std::stringbuf buf;
        is.get(buf, '\n');
        if (is.fail() && !is.bad()) is.clear(); // just didn't read anything
        is.get();
        std::string line=buf.str();
        Utility::trim_right(line);

        if (state == 0) { // startup line
           std::string ver;
           std::istringstream iss(line);
           iss >> ver >> status >> statusText;
           if (ver != "HTTP/1.1") 
              throw ParseError("Not a HTTP response");
           state = 1;
        } else if (state == 1) {
           std::string key,value;
           size_t pos;
           if (line.empty()) {
               state = 2;
               break;
           }
           // split headers
           if ((pos = line.find_first_of(": ")) == std::string::npos) 
             throw ParseError("Malformed header found in response");
           key = line.substr(0, pos);
           value = line.substr(pos+2);
           std::transform(key.begin(), key.end(), key.begin(), ::tolower);
           headers[key] = value;
        }
     }

     if (!is.good() || state != 2) throw ParseError("Unexpected end of data");

     bool chunked = (headers.find("transfer-encoding") != headers.end() && headers["transfer-encoding"] == "chunked");
     int chunk_size = 0;
     while(is.good()) {
         size_t n;
         char buf[1024] = {0};

         if (chunked) {
           if (chunk_size == 0) {
             // read chunk length
             is.get(buf, 1024);
             is.get();
             sscanf(buf, "%x", &chunk_size);
             if (!chunk_size) break;
           } else {
             is.read(buf, chunk_size);
             is.get();
             bodybuf << buf;
             chunk_size = 0;
           }
         } else {
           is.readsome(buf,sizeof buf);
           n = is.gcount();
           if (n > 0) {
             bodybuf << buf;
           } else break;
         }
     }

     body = bodybuf.str();
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
};
