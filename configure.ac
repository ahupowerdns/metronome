AC_PREREQ([2.61])

AC_INIT([yahttp], [git])

AC_CONFIG_SRCDIR([yahttp/reqresp.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([yahttp/yahttp-config.h])

AM_INIT_AUTOMAKE([foreign tar-ustar -Wno-portability subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([], [optional])
AM_PROG_CC_C_O
AC_PROG_MAKE_SET
LT_INIT
AC_CC_PIE
AC_LD_RELRO

AC_CHECK_FUNCS([localtime_r])

AC_CHECK_MEMBER(struct tm.tm_gmtoff,
  [AC_DEFINE(HAVE_TM_GMTOFF, 1,
     [Define if struct tm has the tm_gmtoff member.])],
     ,
     [#include <time.h>])

# test for boost anyways
BOOST_REQUIRE([1.36], [boost=no])
if test "$boost" == ""; then
  BOOST_TEST
fi

AC_CONFIG_FILES([Makefile yahttp/Makefile tests/Makefile examples/Makefile])
AC_OUTPUT
