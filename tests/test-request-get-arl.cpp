#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_request_get_arl)

BOOST_AUTO_TEST_CASE(test_request_parse)
{
  std::ifstream ifs("request-002.txt");
  YaHTTP::Request req;
  YaHTTP::AsyncRequestLoader arl;
  arl.initialize(&req);

  while(ifs.good()) {
    char buf[1024];
    ifs.read(buf, 1024);
    if (ifs.gcount()) { // did we actually read anything
      ifs.clear();
      if (arl.feed(std::string(buf, ifs.gcount())) == true) break; // completed
    }
  }
  BOOST_CHECK(arl.ready());

  arl.finalize(); 
  
  BOOST_CHECK_EQUAL(req.method, "GET");
  BOOST_CHECK_EQUAL(req.url.path, "/");
  BOOST_CHECK_EQUAL(req.url.host, "test.org");

  std::cout << req << std::endl;
}

}
