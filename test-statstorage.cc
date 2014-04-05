#include "statstorage.hh"
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

BOOST_AUTO_TEST_SUITE(statstorage_hh)
BOOST_AUTO_TEST_CASE(test_simple) {
  struct timeval tv;
  gettimeofday(&tv, 0);
  srandom(tv.tv_usec);
  string dirname="stat-unit-test."+boost::lexical_cast<string>(random()+getpid());
  mkdir(dirname.c_str(), 0700);
  StatStorage ss(dirname);
  vector<vector<StatStorage::Datum> > in, out;
  vector<string> names;
  for(unsigned int i=0; i < 100; ++i) {
    string name = "stat." + boost::lexical_cast<string>(i);
    names.push_back(name);
    time_t now=time(0)-10000;
    vector<StatStorage::Datum> input;
    for(unsigned int n=0; n < 10000; ++n) {
      ss.store(name, now+n, n*n);
      input.push_back({(uint32_t)(now+n), (float)(n*n)});
    }
    in.push_back(input);
  }
  StatStorage ss2(dirname);
  for(unsigned int i=0; i < 100; ++i) {
    string name = "stat." + boost::lexical_cast<string>(i);
    out.push_back(ss2.retrieve(name));
  }

  BOOST_CHECK_MESSAGE(in==out, "Different data came out than we put in");
  auto newnames=ss2.getMetrics();
  sort(newnames.begin(), newnames.end());
  sort(names.begin(), names.end());
  BOOST_CHECK_MESSAGE(newnames==names, "getMetrics() returned something different");
}
BOOST_AUTO_TEST_SUITE_END()
