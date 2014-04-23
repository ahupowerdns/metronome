#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_request_simple)

BOOST_AUTO_TEST_CASE(test_request_parse)
{
  std::ifstream ifs("request-001.txt");
  YaHTTP::Request req;
  YaHTTP::Response resp;
  ifs >> req;
  
  BOOST_CHECK_EQUAL(req.method, "POST");
  BOOST_CHECK_EQUAL(req.url.path, "/test");
  BOOST_CHECK_EQUAL(req.url.host, "test.org");

  BOOST_CHECK_EQUAL(req.POST()["Hi"], "Moi");
  BOOST_CHECK_EQUAL(req.POST()["M"], "Bää");
  BOOST_CHECK_EQUAL(req.POST()["Bai"], "Kai");
  BOOST_CHECK_EQUAL(req.POST()["Li"], "Ann"); 

  resp = req;
  resp.status = 200;
  std::cout << resp << std::endl;
  std::cout << resp.kind << std::endl;
}

}
