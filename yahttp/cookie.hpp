namespace YaHTTP {
  class Cookie {
  public:
     Cookie() {
       secure = false;
       httponly = false;
     };

     Cookie(const Cookie &rhs) {
       domain = rhs.domain;
       path = rhs.path;
       secure = rhs.secure;
       httponly = rhs.httponly;
       name = rhs.name;
       value = rhs.value;
     };

     DateTime expires;
     std::string domain;
     std::string path;
     bool httponly;
     bool secure;
 
     std::string name;
     std::string value;

     std::string str() const {
       std::ostringstream oss;
       oss << name << "=" << value;
       if (expires.isSet) 
         oss << "; expires=" << expires.cookie_str();
       if (domain.size()>0)
         oss << "; domain=" << domain;
       if (path.size()>0)
         oss << "; path=" << path;
       if (secure)
         oss << "; secure";
       if (httponly)
         oss << "; httpOnly";
       return oss.str();
     };
  };
};
