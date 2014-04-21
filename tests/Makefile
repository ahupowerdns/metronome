CXXFLAGS=-g -fpie -fPIC -DPIE -std=c++11
LDFLAGS=-fpie -g 
.PHONY: all clean

.cpp.o:
	$(CXX) $(CXXFLAGS) -I.. -c -o $@ $< 

all: test
	./test

test: ../yahttp/libyahttp.a test-main.o test-response-simple.o test-request-simple.o 
	$(CXX) $(LDFLAGS) -o test *.o -lboost_unit_test_framework ../yahttp/libyahttp.a

clean:
	rm test
