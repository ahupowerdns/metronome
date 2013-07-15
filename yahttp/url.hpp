#ifndef _YAHTTP_URL_HPP
#define _YAHTTP_URL_HPP 1
#include <sstream>
#include <string>

namespace YaHTTP {
  class URL {
  public:
      URL() {};
      URL(const std::string& url) {
          parse(url);
      }
      URL(const char *url) { 
          parse(std::string(url));
      }
      bool parse(const std::string& url) {
          // get protocol first
          size_t pos1,pos2;
          if ( (pos1 = url.find_first_of(":")) == std::string::npos ) return false;
          protocol = url.substr(0,pos1);
          if (protocol == "http") port = 80;
          if (protocol == "https") port = 443;

          if (pos1+1 == url.length()) return true; // just protocol
  
          // get the part after proto, before host
          if (url.compare(pos1, 3, "://") == 0) {
             if ( (pos2 = url.find_first_of("/", pos1+3)) == std::string::npos ) return false;
             host = url.substr(pos1+3,pos2-pos1-3);
          } else {
             if ( (pos2 = url.find_first_of("/", pos1)) == std::string::npos ) {
                host = url.substr(pos1+1);
                return true;
             }
             host = url.substr(pos1+1,pos2);
          }
  
          path = url.substr(pos2);
  
          // now we have host, see if it contains username or port
          if ( (pos1 = host.find_first_of("@")) != std::string::npos ) {
             // yep.
             username = host.substr(0,pos1);
             host = host.substr(pos1+1);
             // does username contain password?
             if ( (pos1 = username.find_first_of(":")) != std::string::npos ) {
               password = username.substr(pos1+1);
               username = username.substr(0,pos1);
             }
          }
          // see if there is port
          if ( (pos1 = host.find_first_of(":")) != std::string::npos ) {
             port = atoi(host.substr(pos1+1).c_str());
             host = host.substr(0,pos1);
          }
          // see if path has parameters
          if ( (pos1 = path.find_first_of("?")) != std::string::npos ) {
             parameters = path.substr(pos1+1);
             path = path.substr(0,pos1);
          }
          // see if parameters has anchors
          if ( (pos1 = parameters.find_first_of("#")) != std::string::npos) {
             anchor = parameters.substr(pos1+1);
             parameters = parameters.substr(0,pos1);
          }
  
          // decode some parts
          username = Utility::decodeURL(username);
          password = Utility::decodeURL(password);
          path = Utility::decodeURL(path);
          return true;
      }
  
      std::string to_string() const {
          std::string tmp;
          std::ostringstream oss;
          oss << protocol << "://";

          if (username.empty() == false) {
           if (password.empty() == false)
             oss << Utility::encodeURL(username) << ":" << Utility::encodeURL(password) << "@";
           else
             oss << Utility::encodeURL(username) << "@";
          }

          oss << host;
          if (!(protocol == "http" && port == 80) &&
              !(protocol == "https" && port == 443)) 
            oss << ":" << port;
          oss << Utility::encodeURL(path, true);
          if (parameters.empty() == false)
             oss << "?" << parameters;
          if (anchor.empty() == false)
             oss << "#" << anchor;
          return oss.str();
      }

      std::string protocol;
      std::string host;
      int port;
      std::string username;
      std::string password;
      std::string path;
      std::string parameters;
      std::string anchor;

      friend std::ostream & operator<<(std::ostream& os, const URL& url) {
         os<<url.to_string();
         return os;
      }
  };
};
#endif
