CXX=g++
CXXFLAGS=-fpie -fPIC -DPIE -std=c++11

.PHONY: all basic_webserver

all:
	echo Used per-example

basic_webserver: basic_webserver.cpp ../yahttp/reqresp.cpp
	$(CXX) $(CXXFLAGS) -g -Wall -o basic_webserver basic_webserver.cpp ../yahttp/reqresp.cpp
