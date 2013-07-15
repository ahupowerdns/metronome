#ifndef _YAHTTP_UTILITY_HPP
#define _YAHTTP_UTILITY_HPP 1

#include <string>
#include <algorithm>
#include <cstdio>

namespace YaHTTP {
  class Utility {
  public:
    static std::string decodeURL(const std::string& component) {
        std::string result = component;
        size_t pos1,pos2;
        pos2 = 0;
        while((pos1 = result.find_first_of("%", pos2))!=std::string::npos) {
           std::string code;
           char a,b,c;
           if (pos1 + 2 > result.length()) return result; // end of result
           code = result.substr(pos1+1, 2);
           a = std::tolower(code[0]); b = std::tolower(code[1]);

           if ((( '0' > a || a > '9') && ('a' > a || a > 'f')) ||
              (( '0' > b || b > '9') && ('a' > b || b > 'f'))) {
              pos2 = pos1+3;
              continue;
           }

           if ('0' <= a && a <= '9') a = a - '0';
           if ('a' <= a && a <= 'f') a = a - 'a' + 0x0a;
           if ('0' <= b && b <= '9') b = b - '0';
           if ('a' <= b && b <= 'f') b = b - 'a' + 0x0a;

           c = (a<<4)+b;
           result = result.replace(pos1,3,1,c);
           pos2=pos1;
        }
        return result;
    };
    
    static std::string encodeURL(const std::string& component, bool encodeSlash = true) {
      std::string result = component;
      char repl[3];
      size_t pos;
      for(std::string::iterator iter = result.begin(); iter != result.end(); iter++) {
        if (*iter != '+' && !(encodeSlash == false || *iter == '/') && !std::isalnum(*iter)) {
          // replace with different thing
          pos = std::distance(result.begin(), iter);
          std::snprintf(repl,3,"%02x", *iter);
          result = result.replace(pos, 1, "%", 1).insert(pos+1, repl, 2);
          iter = result.begin() + pos + 2;
        }
      }
      return result;
    };

    static std::string status2text(int status) {
       switch(status) {
       case 200: 
          return "OK";
       case 304:
           return "Not Modified";
       case 401:
           return "Authentication Required";
       case 403: 
           return "Authorization Required";
       case 404:
           return "Object not found";
       case 406:
           return "Bad Request";
       case 500:
           return "Internal Server Error";
       default:
           return "Unspecified Status";
       }
    };

    static void trim_right(std::string &str) {
       const std::locale &loc = std::locale::classic();
       std::string::reverse_iterator iter = str.rbegin();
       while(iter != str.rend() && std::isspace(*iter, loc)) iter++;
       str.erase(iter.base(), str.end());
    };

    static std::string camelizeHeader(const std::string &str) {
       std::string::const_iterator iter = str.begin();
       std::string result;
       const std::locale &loc = std::locale::classic();

       bool doNext = true;

       while(iter != str.end()) {
         if (doNext) 
            result.insert(result.end(), std::toupper(*iter, loc));
         else 
            result.insert(result.end(), std::tolower(*iter, loc)); 
         doNext = (*(iter++) == '-');
       }

       return result;
     };
   };
};
#endif
