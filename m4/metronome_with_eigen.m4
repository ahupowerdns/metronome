AC_DEFUN([METRONOME_WITH_EIGEN],[
  EIGEN3_CFLAGS=
  AC_ARG_WITH([eigen],
    [AS_HELP_STRING([--with-eigen=DIR], [prefix for Eigen @<:@guess@:>@])])

  if test x"$with_eigen" != x; then
    AC_MSG_CHECKING([for Eigen/SVD in $with_eigen])
    if test -f $with_eigen/Eigen/SVD; then
      EIGEN3_CFLAGS='-I$with_eigen'
      AC_MSG_RESULT([yes])
    else
      AC_MSG_ERROR([please install eigen3 and set --with-eigen correctly])
    fi
  else
    PKG_CHECK_MODULES([EIGEN3], [eigen3], [], [])
  fi

  AC_SUBST([EIGEN3_CFLAGS])
])
