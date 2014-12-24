#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

BOOST_AUTO_TEST_SUITE(test_url) 

BOOST_AUTO_TEST_CASE(test_url_complete) {
  YaHTTP::URL url("https://shaun:sheep@test.org:62362/something/somewhere+and+another/%CA%CF.json?boo=baa&faa=fii#anchor1234");
  BOOST_CHECK_EQUAL(url.protocol, "https");
  BOOST_CHECK_EQUAL(url.host, "test.org");
  BOOST_CHECK_EQUAL(url.port, 62362);
  BOOST_CHECK_EQUAL(url.username, "shaun");
  BOOST_CHECK_EQUAL(url.password, "sheep");
  BOOST_CHECK_EQUAL(url.path, "/something/somewhere+and+another/%CA%CF.json");
  BOOST_CHECK_EQUAL(url.parameters, "boo=baa&faa=fii");
  BOOST_CHECK_EQUAL(url.anchor, "anchor1234");
  BOOST_CHECK_EQUAL(url.to_string(), "https://shaun:sheep@test.org:62362/something/somewhere+and+another/%CA%CF.json?boo=baa&faa=fii#anchor1234"); 
}

BOOST_AUTO_TEST_CASE(test_url_path) {
  YaHTTP::URL url("/hello/world");
  BOOST_CHECK_EQUAL(url.path, "/hello/world");
}

BOOST_AUTO_TEST_CASE(test_url_parameters) {
  YaHTTP::URL url("/hello/world?pass=foo&");
  BOOST_CHECK_EQUAL(url.parameters, "pass=foo");
  url.parse("/hello/world?");
  BOOST_CHECK_EQUAL(url.parameters, "");
};

BOOST_AUTO_TEST_CASE(test_url_root) {
  YaHTTP::URL url("http://test.org");
  BOOST_CHECK_EQUAL(url.protocol, "http");
  BOOST_CHECK_EQUAL(url.host, "test.org");
  BOOST_CHECK_EQUAL(url.port, 80);
  BOOST_CHECK_EQUAL(url.path, "/");
  url.parse("http://test.org/");
  BOOST_CHECK_EQUAL(url.protocol, "http");
  BOOST_CHECK_EQUAL(url.host, "test.org");
  BOOST_CHECK_EQUAL(url.path, "/");
  url.parse("/");
  BOOST_CHECK_EQUAL(url.path, "/");
}

BOOST_AUTO_TEST_CASE(test_url_data) {
  YaHTTP::URL url("data:base64:9vdas9t64gadsf=");
  BOOST_CHECK_EQUAL(url.protocol, "data");
  BOOST_CHECK_EQUAL(url.parameters, "base64:9vdas9t64gadsf=");
}

BOOST_AUTO_TEST_CASE(test_url_invalid) {
  YaHTTP::URL url;
  BOOST_CHECK(!url.parse("http")); // missing : 
}

};
