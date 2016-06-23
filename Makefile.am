metronomehtmldir = @datarootdir@/metronome
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
  mmanage \
  mdump

metronome_SOURCES = \
  metronome.cc \
  iputils.cc iputils.hh \
  statstorage.cc statstorage.hh \
  interpolate.cc interpolate.hh \
  metromisc.hh \
  dolog.hh

metronome_LDFLAGS = \
	$(AM_LDFLAGS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS) \
	$(YAHTTP_LIBS)

metronome_LDADD = \
	$(BOOST_PROGRAM_OPTIONS_LIBS)

msubmit_SOURCES = \
  msubmit.cc \
  iputils.cc iputils.hh \
  metromisc.hh \
  dolog.hh

mmanage_SOURCES= \
  mmanage.cc \
  statstorage.cc statstorage.hh \
  metromisc.hh \
  dolog.hh

mdump_SOURCES= \
  mdump.cc \
  statstorage.cc statstorage.hh \
  metromisc.hh \
  dolog.hh


nobase_dist_metronomehtml_DATA = \
  html/*.* \
  html/js/*

dist_doc_DATA = \
  README.md

EXTRA_DIST = \
  LICENSE \
  build-aux/gen-version \
  .version

#if DOXYGEN
#codedocs: codedocs/html/index.html

#codedocs/html/index.html:
#	doxygen
#endif

#check: testrunner
#	./testrunner

#testrunner: testrunner.o test-statstorage.o statstorage.o
#	$(CXX) $^ -lboost_unit_test_framework -o $@ 
