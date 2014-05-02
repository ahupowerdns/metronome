#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_request_get)

BOOST_AUTO_TEST_CASE(test_request_parse)
{
  std::ifstream ifs("request-002.txt");
  YaHTTP::Request req;
  YaHTTP::Response resp;
  ifs >> req;
  
  BOOST_CHECK_EQUAL(req.method, "GET");
  BOOST_CHECK_EQUAL(req.url.path, "/");
  BOOST_CHECK_EQUAL(req.url.host, "test.org");

  resp = req;
  resp.status = 200;
  std::cout << resp << std::endl;
  std::cout << resp.kind << std::endl;
}

}
