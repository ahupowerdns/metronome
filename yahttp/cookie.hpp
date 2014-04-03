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

  class CookieJar {
    public:
    std::map<std::string, Cookie> cookies; 
  
    CookieJar() {};
    CookieJar(const CookieJar & rhs) {
      this->cookies = rhs.cookies;
    }
  
    void keyValuePair(const std::string &keyvalue, std::string &key, std::string &value) {
      size_t pos;
      pos = keyvalue.find("=");
      if (pos == std::string::npos) throw "Not a Key-Value pair (cookie)";
      key = std::string(keyvalue.begin(), keyvalue.begin()+pos);
      value = std::string(keyvalue.begin()+pos+1, keyvalue.end());
    }
  
    void parseCookieHeader(const std::string &cookiestr) {
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
        this->cookies[i->name] = *i;
      }
    };
  };
};
