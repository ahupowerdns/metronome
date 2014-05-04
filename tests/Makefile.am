EXTRA_PROGRAMS=test
test_CFLAGS=$(RELRO_CFLASG) $(PIE_CFLAGS) -I..
test_CXXFLAGS=$(RELRO_CFLASG) $(PIE_CFLAGS) -I..
test_LDADD=$(RELRO_LDFLAGS) $(PIE_LDFLAGS) -lboost_unit_test_framework ../yahttp/libyahttp.la
test_SOURCES=md5.h md5.c test-main.cpp test-md5.cpp test-utility.cpp test-url.cpp test-cookie.cpp test-request.cpp test-response.cpp test-router.cpp

TESTS=test
TESTS_ENVIRONMENT = env BOOST_TEST_LOG_LEVEL=all
