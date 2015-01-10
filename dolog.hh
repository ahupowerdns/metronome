#pragma once
#include <iostream>
#include <sstream>
#include <syslog.h>

inline void dolog(std::ostream& os, const char*s)
{
  os<<s;
  // test
}

template<typename T, typename... Args>
void dolog(std::ostream& os, const char* s, T value, Args... args)
{
  while (*s) {
    if (*s == '%') {
      if (*(s + 1) == '%') {
	++s;
      }
      else {
	os << value;
	s += 2;
	dolog(os, s, args...); // call even when *s == 0 to detect extra arguments
	return;
      }
    }
    os << *s++;
  }    
}

template<typename... Args>
void genlog(int level, const char* s, Args... args)
{
  extern bool g_console;
  std::ostringstream str;
  dolog(str, s, args...);
  syslog(level, "%s", str.str().c_str());
  if(g_console) 
    std::cout<<str.str()<<std::endl;
}

template<typename... Args>
void infolog(const char* s, Args... args)
{
  genlog(LOG_INFO, s, args...);
}

template<typename... Args>
void warnlog(const char* s, Args... args)
{
  genlog(LOG_WARNING, s, args...);
}

template<typename... Args>
void errlog(const char* s, Args... args)
{
  genlog(LOG_ERR, s, args...);
}
