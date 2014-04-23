EXTRA_PROGRAMS=test
test_CXXFLAGS=$(RELRO_CFLASG) $(PIE_CFLAGS) -I..
test_LDADD=$(RELRO_LDFLAGS) $(PIE_LDFLAGS) -lboost_unit_test_framework ../yahttp/libyahttp.a
test_SOURCES=test-main.cpp test-request-simple.cpp test-response-simple.cpp

TESTS=test
TESTS_ENVIRONMENT = env BOOST_TEST_LOG_LEVEL=all
