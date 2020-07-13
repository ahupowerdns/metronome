#include "iputils.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using std::cin;
using std::cerr;
using std::endl;


// syntax: msubmit address
int main(int argc, char** argv)
{
  try
  {
    if(argc!=2) {
      cerr<<"Syntax: (echo metric1 value1 ; echo metric2 value2 $(date +%s) ) |  msubmit carbon-server-address"<<endl;
      exit(EXIT_FAILURE);
    }
    ComboAddress remote(argv[1], 2003);
    time_t now = time(nullptr);

    int s = SSocket(remote.sin4.sin_family, SOCK_STREAM, 0);
    SConnect(s, remote);
    int ret;
    std::string line;
    while(getline(cin, line)) {
      boost::trim_right(line);
 
      std::vector<std::string> parts;
      stringtok(parts, line, " \t\n");
      if(parts.size() < 2)
        throw std::runtime_error("Line '"+line+"' contained less than 2 parts");

      string output = parts[0];
      output+=' ';
      output += parts[1];
      output+=' ';
      if(parts.size() > 2)
        output += parts[2];
      else
        output += boost::lexical_cast<string>(now);
      output += "\r\n";

      ret = writen(s, output.c_str(), output.size()); //  format: name value timestamp
      if(!ret)
        throw std::runtime_error("Carbon server '"+remote.toStringWithPort()+"' closed socket while feeding data");
      if(ret < 0)
        throw std::runtime_error("Error writing to Carbon server '"+remote.toStringWithPort()+"': "+strerror(errno));
    }
    shutdown(s, SHUT_WR);
    char c;
    ret=read(s, &c, 1);
    if(ret < 0)
      throw std::runtime_error("Improper shutdown of Carbon session with '"+remote.toStringWithPort()+"': "+strerror(errno));
    if(ret == 1)
      throw std::runtime_error("Improper shutdown of Carbon session with '"+remote.toStringWithPort()+"': they sent us data");
  }
  catch(const std::exception& e)
  {
    cerr<<"Error: "<<e.what()<<endl;
    exit(EXIT_FAILURE);
  }
}
