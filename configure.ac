AC_INIT([metronome], m4_esyscmd(build-aux/gen-version))
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip tar-ustar -Wno-portability subdir-objects parallel-tests 1.11])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([metronome.cc])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_CANONICAL_HOST
: ${CFLAGS="-Wall -g -O2"}
: ${CXXFLAGS="-Wall -g -O2"}

AC_PROG_CXX
AS_IF([test "x$CXX" = "xno" || test "x$CXX:x$GXX" = "xg++:x"],
  AC_MSG_ERROR([no C++ compiler found])
)
AX_CXX_COMPILE_STDCXX_11(ext,mandatory)
AC_PROG_LIBTOOL

#AC_LANG([C++])

AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_DEFINE([_GNU_SOURCE], [1],
  [Define _GNU_SOURCE so that we get all necessary prototypes]
)

# Warn when pkg.m4 is missing
m4_pattern_forbid([^_?PKG_[A-Z_]+$],[*** pkg.m4 missing, please install pkg-config])

PDNS_CHECK_OS

METRONOME_WITH_EIGEN

BOOST_REQUIRE([1.35])
BOOST_PROGRAM_OPTIONS([mt])

AC_MSG_CHECKING([whether we should enable hardening])
AC_ARG_ENABLE([hardening],
  [AS_HELP_STRING([--disable-hardening],[disable compiler security checks @<:@default=no@:>@])],
  [enable_hardening=$enableval],
  [enable_hardening=yes]
)
AC_MSG_RESULT([$enable_hardening])

AS_IF([test "x$enable_hardening" != "xno"], [
  AC_CC_PIE
  AC_CC_STACK_PROTECTOR
  AC_CC_PARAM_SSP_BUFFER_SIZE([4])
  AC_CC_D_FORTIFY_SOURCE
  AC_LD_RELRO
])

PDNS_ENABLE_SANITIZERS
PDNS_ENABLE_MALLOC_TRACE

AC_SUBST(LIBS)

AC_SUBST([AM_CPPFLAGS],
  ["AS_ESCAPE([-I$(top_builddir) -I$(top_srcdir)]) $THREADFLAGS $BOOST_CPPFLAGS"]
)

AC_SUBST([YAHTTP_CFLAGS], ['-I$(top_srcdir)/yahttp/yahttp'])
AC_SUBST([YAHTTP_LIBS], ['-L$(top_builddir)/yahttp/yahttp -lyahttp'])

CXXFLAGS="$SANITIZER_FLAGS $CXXFLAGS"
LDFLAGS="$RELRO_LDFLAGS $LDFLAGS"

AC_SUBST([PROGRAM_LDFLAGS])

AC_CONFIG_FILES([Makefile
	yahttp/Makefile
	yahttp/yahttp/Makefile])

AC_OUTPUT
