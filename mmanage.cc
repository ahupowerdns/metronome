#include "statstorage.hh"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using std::cin;
using std::cerr;
using std::cout;
using std::endl;


// syntax: mmanage
int main(int argc, char** argv)
try
{
  StatStorage ss("./stats");
  auto metrics=ss.getMetrics();
  cout<<"Have "<<metrics.size()<<" metrics"<<endl;
  for(const auto& m: metrics) {
    auto vals=ss.retrieve(m);
    cout<<"Have "<<vals.size()<<" values for "<<m<<endl;
    ss.store(m, vals);
  }

}
catch(std::exception& e)
{
  cerr<<"Error: "<<e.what()<<endl;
  exit(EXIT_FAILURE);
}
