-include sysdeps/$(shell uname).inc
VERSION=0.1

ifeq ($(CXX),clang++)
	CXX2011FLAGS?=-std=c++11 -stdlib=libc++
endif

CXXFLAGS?=-Wall -O3 -ggdb -I. -I yahttp/yahttp  -pthread -MMD -MP -DYAHTTP_MAX_URL_LENGTH=16384 $(CXX2011FLAGS) # -Wno-unused-local-typedefs 
CFLAGS=-Wall -I.  -O3 -MMD -MP
LDFLAGS+=$(CXX2011FLAGS) -pthread  $(STATICFLAGS)
CHEAT_ARG := $(shell ./update-git-hash-if-necessary)

SHIPPROGRAMS=metronome msubmit
PROGRAMS=$(SHIPPROGRAMS)

all: $(PROGRAMS)

-include *.d

.PHONY:	check

metronome: metronome.o yahttp/yahttp/reqresp.o iputils.o statstorage.o interpolate.o
	$(CC) $^ $(LDFLAGS) -o $@

msubmit: msubmit.o iputils.o
	$(CC) $^ $(LDFLAGS) -o $@

mmanage: mmanage.o statstorage.o
	$(CC) $^ $(LDFLAGS) -o $@

install: metronome
	mkdir -p $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/share/doc/metronome
	install -s $(SHIPPROGRAMS) $(DESTDIR)/usr/bin/

clean:
	rm -f *~ *.o *.d yahttp/yahttp/*.o $(PROGRAMS) githash.h

package: all
	rm -rf dist
	DESTDIR=dist make install
	fpm -s dir -f -t rpm -n metronome -v ${VERSION}.$(shell date +%Y%m%d).g$(shell cat githash) --iteration 1 -C dist .
	fpm -s dir -f -t deb -n metronome -v ${VERSION}+$(shell date +%Y%m%d).g$(shell cat githash) --iteration 1 -C dist .
	rm -rf dist

codedocs: codedocs/html/index.html

codedocs/html/index.html: 	
	doxygen

check: testrunner
	./testrunner

testrunner: testrunner.o test-statstorage.o statstorage.o
	$(CXX) $^ -lboost_unit_test_framework -o $@ 
