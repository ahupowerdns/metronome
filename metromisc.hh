#pragma once
#include <string>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <vector>
#include <unistd.h>

#include "dolog.hh"
using std::string;

inline void unixDie(const string &why)
{
  throw std::runtime_error(why+": "+strerror(errno));
}

template <typename Container>
void
stringtok (Container &container, string const &in,
           const char * const delimiters = " \t\n")
{
  const string::size_type len = in.length();
  string::size_type i = 0;
  
  while (i<len) {
    // eat leading whitespace
    i = in.find_first_not_of (delimiters, i);
    if (i == string::npos)
      return;   // nothing left but white space
    
    // find the end of the token
    string::size_type j = in.find_first_of (delimiters, i);
    
    // push token
    if (j == string::npos) {
      container.push_back (in.substr(i));
      return;
    } else
      container.push_back (in.substr(i, j-i));
    
    // set up for next loop
    i = j + 1;
  }
}

class Socket
{
public:
  Socket(int descriptor): d_sock(descriptor)
  {
  }

  ~Socket()
  {
    if (d_sock != -1) {
      close(d_sock);
      d_sock = -1;
    }
  }

  int getHandle() const
  {
    return d_sock;
  }

private:
  int d_sock{-1};
};
