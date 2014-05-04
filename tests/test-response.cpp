#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

#include "md5.h"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_response)

BOOST_AUTO_TEST_CASE(test_response_parse_ok)
{
  std::ifstream ifs("response-google-200.txt");
  YaHTTP::Response resp;
  ifs >> resp;

  BOOST_CHECK_EQUAL(resp.status, 200);
}

BOOST_AUTO_TEST_CASE(test_response_parse_arl_ok)
{
  std::ifstream ifs("response-google-200.txt");
  YaHTTP::Response resp;

  YaHTTP::AsyncResponseLoader arl;
  arl.initialize(&resp); 

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

  BOOST_CHECK_EQUAL(resp.status, 200);
}

BOOST_AUTO_TEST_CASE(test_response_parse_incomplete)
{
  std::ifstream ifs("response-incomplete.txt");
  YaHTTP::Response resp;
  BOOST_CHECK_THROW(ifs >> resp, YaHTTP::ParseError);
}

BOOST_AUTO_TEST_CASE(test_response_parse_binary) {
  unsigned char result[16];
  unsigned char expected[16] = { 0xb6, 0x37, 0x1c, 0x8a, 0xc6, 0xa6, 
                                 0xa0, 0xfb, 0x10, 0x4e, 0x3d, 0x64,
                                 0x79, 0xb4, 0xd4, 0x7a };
  std::ifstream ifs("response-binary.txt");
  YaHTTP::Response resp;

  ifs >> resp;

  BOOST_CHECK_EQUAL(resp.status, 200);
  BOOST_CHECK_EQUAL(resp.body.size(), 258504);
  
  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, resp.body.c_str(), resp.body.size());
  MD5_Final(result, &ctx);

  BOOST_CHECK_EQUAL(::memcmp(result, expected, 16), 0);
}

}
