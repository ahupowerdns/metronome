.PHONY: all clean

CXXFLAGS=-fpie -fPIC -DPIE -std=c++11

ifeq ($(BOOST),yes)
CXXFLAGS += -DHAVE_BOOST
endif

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

all: libyahttp.a

clean:
	-rm -f *.o *.a

reqresp.o: *.cpp *.hpp

libyahttp.a: reqresp.o
	echo AR $@
	$(AR) rcs $@ $<
