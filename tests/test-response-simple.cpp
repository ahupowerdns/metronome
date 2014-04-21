#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "yahttp/yahttp.hpp"

using namespace boost;


BOOST_AUTO_TEST_SUITE(test_response_simple)

BOOST_AUTO_TEST_CASE(test_response_parse)
{
   BOOST_CHECK(true);
}

}
