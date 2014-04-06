-include sysdeps/$(shell uname).inc

VERSION=0.1
CXXFLAGS?=-Wall -O3 -MMD -MP  $(CXX2011FLAGS) # -Wno-unused-local-typedefs 
LDFLAGS=$(CXX2011FLAGS)   

PROGRAMS=testrunner

all: $(PROGRAMS)

clean:
	-rm  -f *.d *.o *~ testrunner

testrunner: rfile.o testrunner.o
	$(CXX) $(CXXFLAGS) $^ -o $@

-include *.d