#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

BOOST_AUTO_TEST_SUITE(test_utility)

BOOST_AUTO_TEST_CASE(test_utility_datetime) {
  YaHTTP::DateTime dt;
  BOOST_CHECK_EQUAL(dt.rfc_str(), "Sun, 00 Jan 00 00:00:00 +0000");
  dt.parse822("Sun, 04 May 2014 13:27:36 +0000");
  BOOST_CHECK_EQUAL(dt.rfc_str(), "Sun, 04 May 2014 13:27:36 +0000");
  dt.parseCookie("04-May-2014 13:37:00 GMT");
  BOOST_CHECK_EQUAL(dt.cookie_str(), "04-May-2014 13:37:00 GMT");
}

BOOST_AUTO_TEST_CASE(test_utility_iequals) {
  BOOST_CHECK(YaHTTP::Utility::iequals("",""));
  BOOST_CHECK(YaHTTP::Utility::iequals("a","a"));
  BOOST_CHECK(YaHTTP::Utility::iequals("a","A"));
  BOOST_CHECK(YaHTTP::Utility::iequals("A","a"));
  BOOST_CHECK(YaHTTP::Utility::iequals("A","A"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("a","b"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("a","B"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("A","b"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("A","B"));
  BOOST_CHECK(YaHTTP::Utility::iequals("hello","HeLlO"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("hello","world"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("qUiCk","FoX"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("fOx","QuIcK"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("","HeLlO"));
  BOOST_CHECK(!YaHTTP::Utility::iequals("hello",""));
  BOOST_CHECK(YaHTTP::Utility::iequals("\x00\x00\x00\x00\x00\x00f","\x00\x00\x00\x00\x00\x00f"));
}

BOOST_AUTO_TEST_CASE(test_utility_encodeurl) {
  // first check that encodeurl works for non-urls 

  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("", false), "");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("hello world", false), "hello%20world");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("Kääkkä", false), "K%c3%a4%c3%a4kk%c3%a4");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("鳥の桜", false), "%e9%b3%a5%e3%81%ae%e6%a1%9c");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL(L"鳥の桜", false), "%e5%9c%00%00n0%00%00%5ch%00%00"); // as you can see it's not utf-8
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("https://test.org/(user)/michael/status.jsonp?merge=true", false), "https%3a%2f%2ftest%2eorg%2f%28user%29%2fmichael%2fstatus%2ejsonp%3fmerge%3dtrue");

  // then this same stuff should really work with urls, right? 
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("https://test.org/(user)/michael/status.jsonp?merge=true", true), "https://test.org/(user)/michael/status.jsonp?merge=true");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("https://test.org/(user)/kääkkä/status.jsonp?merge=true", true), "https://test.org/(user)/k%c3%a4%c3%a4kk%c3%a4/status.jsonp?merge=true");
}

BOOST_AUTO_TEST_CASE(test_utility_decodeurl) {
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL(""), "");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("hello%20world"), "hello world");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("K%c3%a4%c3%a4kk%c3%a4"), "Kääkkä");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("%e9%b3%a5%e3%81%ae%e6%a1%9c"), "鳥の桜");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("https%3a%2f%2ftest%2eorg%2f%28user%29%2fmichael%2fstatus%2ejsonp%3fmerge%3dtrue"), "https://test.org/(user)/michael/status.jsonp?merge=true");

  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("https://test.org/(user)/michael/status.jsonp?merge=true"), "https://test.org/(user)/michael/status.jsonp?merge=true");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("https://test.org/(user)/k%c3%a4%c3%a4kk%c3%a4/status.jsonp?merge=true"), "https://test.org/(user)/kääkkä/status.jsonp?merge=true");
}

BOOST_AUTO_TEST_CASE(test_utility_parseurlparameters) {
}

BOOST_AUTO_TEST_CASE(test_utility_trimright) {
}

BOOST_AUTO_TEST_CASE(test_utility_camelizeheader) {
}

}
