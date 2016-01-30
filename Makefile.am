AM_CPPFLAGS = $(EIGEN3_CFLAGS) $(YAHTTP_CFLAGS) $(BOOST_CPPFLAGS) $(SANITIZER_FLAGS) -O3 -Wall -pthread

AM_LDFLAGS = \
	$(PROGRAM_LDFLAGS) \
	$(THREADFLAGS) \
	$(BOOST_LDFLAGS)

ACLOCAL_AMFLAGS = -I m4

SUBDIRS=yahttp

bin_PROGRAMS = \
  metronome \
  msubmit \
  mmanage

metronome_SOURCES = \
  metronome.cc \
  iputils.cc iputils.hh \
  statstorage.cc statstorage.hh \
  interpolate.cc interpolate.hh

metronome_LDFLAGS = \
	$(AM_LDFLAGS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(YAHTTP_LIBS)

metronome_LDADD = \
	$(BOOST_PROGRAM_OPTIONS_LIBS)

msubmit_SOURCES = \
  msubmit.cc \
  iputils.cc iputils.hh

mmanage_SOURCES= \
  mmanage.cc \
  statstorage.cc statstorage.hh

#if DOXYGEN
#codedocs: codedocs/html/index.html

#codedocs/html/index.html:
#	doxygen
#endif

#check: testrunner
#	./testrunner

#testrunner: testrunner.o test-statstorage.o statstorage.o
#	$(CXX) $^ -lboost_unit_test_framework -o $@ 
