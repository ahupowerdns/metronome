#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "md5.h"

#define MD5_TEST(x,y) { \
  memcpy(expected, y, 16); \
  MD5(x, sizeof(x)-1, result); \
  BOOST_CHECK_EQUAL(memcmp(result, expected, 16), 0); \
}
 
BOOST_AUTO_TEST_SUITE(test_md5)

BOOST_AUTO_TEST_CASE(test_md5) {
// MD5 test suite from RFC1321
  unsigned char result[16];
  unsigned char expected[16];

  MD5_TEST("","\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e");
  MD5_TEST("a","\x0c\xc1\x75\xb9\xc0\xf1\xb6\xa8\x31\xc3\x99\xe2\x69\x77\x26\x61");
  MD5_TEST("abc","\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72");
  MD5_TEST("message digest","\xf9\x6b\x69\x7d\x7c\xb7\x93\x8d\x52\x5a\x2f\x31\xaa\xf1\x61\xd0");
  MD5_TEST("abcdefghijklmnopqrstuvwxyz","\xc3\xfc\xd3\xd7\x61\x92\xe4\x00\x7d\xfb\x49\x6c\xca\x67\xe1\x3b");
  MD5_TEST("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789","\xd1\x74\xab\x98\xd2\x77\xd9\xf5\xa5\x61\x1c\x2c\x9f\x41\x9d\x9f");
  MD5_TEST("12345678901234567890123456789012345678901234567890123456789012345678901234567890","\x57\xed\xf4\xa2\x2b\xe3\xc9\x55\xac\x49\xda\x2e\x21\x07\xb6\x7a");
}

}
