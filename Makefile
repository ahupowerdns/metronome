CXX=g++
CXXFLAGS=-fpie -fPIC -DPIE -std=c++11

.PHONY: all build docs tests clean

all: build docs tests

clean:
	$(MAKE) -C yahttp clean
	$(MAKE) -C docs clean
	$(MAKE) -C tests clean

build:
	$(MAKE) -C yahttp all BOOST=$(BOOST) CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

docs:
	$(MAKE) -C docs all

tests:
	$(MAKE) -C tests all

test: 
	$(CXX) $(CXXFLAGS) -W -Wall -g -o test test.cc yahttp/reqresp.cpp 
