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
  BOOST_CHECK_EQUAL(YaHTTP::Utility::encodeURL("DzE6fx2EtlPz3B787qYV/ezKBbIoihZpSb8DB3/ro1yuVniU4g2p3s6rnQZP66Rve6SNPtHwlr7FAAO27dzkBDikNVJxABoN7oYnqZ2QGVrb49tN10PaSDUlzPuDCV0UUjUY/KYz6gGpnBsU5l6G4zgF+m08bP0gqeJ00+iQ7EwGRpMbi1Yh04uj4T+mVObgj1EhYJm/Oq0nNQKyto7i3YtukjdtKrYEWDNdqPcMOHK7fu+LuMAGoziNbh0H9g/p8Zlx4TkElszvqPDpFHbz2rHME/ZA92VrfVeVDc9/2C2JzTjmB0FO4ml32TEj9TA2fFFiSGlbF3pVq1pvmnWVQ==\nPublicExponent: AQAB\nPrivateExponent: Xw+t8Gc/nCKKLPJa2TO0kguTNIfPvyBk9B5LFVLgVNjtmmqwWM5K+JcfdZVDxxH0UA/hjQGVgq6q6kD9F/mn094Fq8lNKPdqkeQcuJdocn+0mP4769zA+iOZ/QtY8/ixhGCXZrVYi5dzcmq1psGGeIk5Z5nSOJPcnZ9rsGr4uq/QyPZe6K3SM6oOn/e6Wn/8uXjqgMkdA3+50ur/tApyufol03wcakWke2kRAI3OGiJJIB7QphPfC0h7ZPF+RlRKXbNFhQaDB7B0A98A9WFQppY5ghhhOKHSEMI4xoQSWK/HIsJ+JBRPlsOYAeoBSVnqOiPHxgNNQs+UrNpavYlwAQ==", false), "DzE6fx2EtlPz3B787qYV%2fezKBbIoihZpSb8DB3%2fro1yuVniU4g2p3s6rnQZP66Rve6SNPtHwlr7FAAO27dzkBDikNVJxABoN7oYnqZ2QGVrb49tN10PaSDUlzPuDCV0UUjUY%2fKYz6gGpnBsU5l6G4zgF%2bm08bP0gqeJ00%2biQ7EwGRpMbi1Yh04uj4T%2bmVObgj1EhYJm%2fOq0nNQKyto7i3YtukjdtKrYEWDNdqPcMOHK7fu%2bLuMAGoziNbh0H9g%2fp8Zlx4TkElszvqPDpFHbz2rHME%2fZA92VrfVeVDc9%2f2C2JzTjmB0FO4ml32TEj9TA2fFFiSGlbF3pVq1pvmnWVQ%3d%3d%0aPublicExponent%3a%20AQAB%0aPrivateExponent%3a%20Xw%2bt8Gc%2fnCKKLPJa2TO0kguTNIfPvyBk9B5LFVLgVNjtmmqwWM5K%2bJcfdZVDxxH0UA%2fhjQGVgq6q6kD9F%2fmn094Fq8lNKPdqkeQcuJdocn%2b0mP4769zA%2biOZ%2fQtY8%2fixhGCXZrVYi5dzcmq1psGGeIk5Z5nSOJPcnZ9rsGr4uq%2fQyPZe6K3SM6oOn%2fe6Wn%2f8uXjqgMkdA3%2b50ur%2ftApyufol03wcakWke2kRAI3OGiJJIB7QphPfC0h7ZPF%2bRlRKXbNFhQaDB7B0A98A9WFQppY5ghhhOKHSEMI4xoQSWK%2fHIsJ%2bJBRPlsOYAeoBSVnqOiPHxgNNQs%2bUrNpavYlwAQ%3d%3d");
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
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("DzE6fx2EtlPz3B787qYV%2fezKBbIoihZpSb8DB3%2fro1yuVniU4g2p3s6rnQZP66Rve6SNPtHwlr7FAAO27dzkBDikNVJxABoN7oYnqZ2QGVrb49tN10PaSDUlzPuDCV0UUjUY%2fKYz6gGpnBsU5l6G4zgF%2bm08bP0gqeJ00%2biQ7EwGRpMbi1Yh04uj4T%2bmVObgj1EhYJm%2fOq0nNQKyto7i3YtukjdtKrYEWDNdqPcMOHK7fu%2bLuMAGoziNbh0H9g%2fp8Zlx4TkElszvqPDpFHbz2rHME%2fZA92VrfVeVDc9%2f2C2JzTjmB0FO4ml32TEj9TA2fFFiSGlbF3pVq1pvmnWVQ%3d%3d%0aPublicExponent%3a%20AQAB%0aPrivateExponent%3a%20Xw%2bt8Gc%2fnCKKLPJa2TO0kguTNIfPvyBk9B5LFVLgVNjtmmqwWM5K%2bJcfdZVDxxH0UA%2fhjQGVgq6q6kD9F%2fmn094Fq8lNKPdqkeQcuJdocn%2b0mP4769zA%2biOZ%2fQtY8%2fixhGCXZrVYi5dzcmq1psGGeIk5Z5nSOJPcnZ9rsGr4uq%2fQyPZe6K3SM6oOn%2fe6Wn%2f8uXjqgMkdA3%2b50ur%2ftApyufol03wcakWke2kRAI3OGiJJIB7QphPfC0h7ZPF%2bRlRKXbNFhQaDB7B0A98A9WFQppY5ghhhOKHSEMI4xoQSWK%2fHIsJ%2bJBRPlsOYAeoBSVnqOiPHxgNNQs%2bUrNpavYlwAQ%3d%3d"), "DzE6fx2EtlPz3B787qYV/ezKBbIoihZpSb8DB3/ro1yuVniU4g2p3s6rnQZP66Rve6SNPtHwlr7FAAO27dzkBDikNVJxABoN7oYnqZ2QGVrb49tN10PaSDUlzPuDCV0UUjUY/KYz6gGpnBsU5l6G4zgF+m08bP0gqeJ00+iQ7EwGRpMbi1Yh04uj4T+mVObgj1EhYJm/Oq0nNQKyto7i3YtukjdtKrYEWDNdqPcMOHK7fu+LuMAGoziNbh0H9g/p8Zlx4TkElszvqPDpFHbz2rHME/ZA92VrfVeVDc9/2C2JzTjmB0FO4ml32TEj9TA2fFFiSGlbF3pVq1pvmnWVQ==\nPublicExponent: AQAB\nPrivateExponent: Xw+t8Gc/nCKKLPJa2TO0kguTNIfPvyBk9B5LFVLgVNjtmmqwWM5K+JcfdZVDxxH0UA/hjQGVgq6q6kD9F/mn094Fq8lNKPdqkeQcuJdocn+0mP4769zA+iOZ/QtY8/ixhGCXZrVYi5dzcmq1psGGeIk5Z5nSOJPcnZ9rsGr4uq/QyPZe6K3SM6oOn/e6Wn/8uXjqgMkdA3+50ur/tApyufol03wcakWke2kRAI3OGiJJIB7QphPfC0h7ZPF+RlRKXbNFhQaDB7B0A98A9WFQppY5ghhhOKHSEMI4xoQSWK/HIsJ+JBRPlsOYAeoBSVnqOiPHxgNNQs+UrNpavYlwAQ==");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("https://test.org/(user)/michael/status.jsonp?merge=true"), "https://test.org/(user)/michael/status.jsonp?merge=true");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::decodeURL("https://test.org/(user)/k%c3%a4%c3%a4kk%c3%a4/status.jsonp?merge=true"), "https://test.org/(user)/kääkkä/status.jsonp?merge=true");
}

BOOST_AUTO_TEST_CASE(test_utility_parseurlparameters) {
  YaHTTP::strstr_map_t parameters;
  parameters = YaHTTP::Utility::parseUrlParameters("Hi=Moi&M=B%C3%A4%C3%A4&Bai=Kai&Li=Ann");
  BOOST_CHECK_EQUAL(parameters["Hi"], "Moi");
  BOOST_CHECK_EQUAL(parameters["M"], "Bää");
  BOOST_CHECK_EQUAL(parameters["Bai"], "Kai");
  BOOST_CHECK_EQUAL(parameters["Li"], "Ann"); 
}

BOOST_AUTO_TEST_CASE(test_utility_trimright) {
  std::string str = "";
  YaHTTP::Utility::trimRight(str);
  BOOST_CHECK_EQUAL(str, "");
  str = "   ";
  YaHTTP::Utility::trimRight(str);
  BOOST_CHECK_EQUAL(str, "");
  str = "  hello  ";
  YaHTTP::Utility::trimRight(str);
  BOOST_CHECK_EQUAL(str, "  hello");
  str = "hello  ";
  YaHTTP::Utility::trimRight(str);
  BOOST_CHECK_EQUAL(str, "hello");
  str = "  hello";
  YaHTTP::Utility::trimRight(str);
  BOOST_CHECK_EQUAL(str, "  hello");
  str = "  h e l l o  ";
  YaHTTP::Utility::trimRight(str);
  BOOST_CHECK_EQUAL(str, "  h e l l o");
}

BOOST_AUTO_TEST_CASE(test_utility_camelizeheader) {
  BOOST_CHECK_EQUAL(YaHTTP::Utility::camelizeHeader("x-forwarded-for"),"X-Forwarded-For");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::camelizeHeader("x-my-ip"),"X-My-Ip");
  BOOST_CHECK_EQUAL(YaHTTP::Utility::camelizeHeader("x------y"),"X------Y");
}

BOOST_AUTO_TEST_CASE(test_strstr_map_t) {
  YaHTTP::strstr_map_t test;
  test["HELLO"] = "world";
  BOOST_CHECK_EQUAL(test["HELLO"], "world");
  test["hello"] = "WORLD";
  BOOST_CHECK_EQUAL(test["hello"], "WORLD");
  BOOST_CHECK_EQUAL(test["HeLlO"], "WORLD");
}

}
