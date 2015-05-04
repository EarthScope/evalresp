
AC_INIT([evalresp], [4.0.0], [andrewcooke@isti.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile doc/Makefile])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL

dnl select dynamic and/or static libraries here
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(yes)

dnl Checks for libraries.
AC_CHECK_LIB(m, fabs)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h malloc.h stdlib.h)

dnl Checks for library functions.
AC_FUNC_VFORK
AC_CHECK_FUNCS(getcwd regcomp strcspn strstr)

dnl Options processing
AC_ARG_ENABLE(phase_unwrap,
  [  --enable-phase-unwrap   unwrap phase],
  [CFLAGS="$CFLAGS -DUNWRAP_PHASE"
   echo ==== BEWARE: EVALRESP WILL UNWRAP PHASE  ======],
  []) 

AC_ARG_ENABLE(lib-mode,
  [  --enable-lib-mode       functions of evalresp return, not exit],
  [CFLAGS="$CFLAGS -DLIB_MODE"
   echo ==== evalresp will be compiled in library mode  ======],
  [])

AC_ARG_ENABLE(log-label,
  [  --enable-log-label      add NSLC as a label to log messages],
  [CFLAGS="$CFLAGS -DLOG_LABEL"
   echo ==== enabling log-label mode  ======],
  [])

AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug],
  [CFLAGS="$CFLAGS -g"
   echo ==== Enabling DEBUG options ======],
  [])

AC_ARG_ENABLE(optimization,
  [  --enable-optimization   enable optimization],
  [CFLAGS="$CFLAGS -02"
   echo ==== Enabling -O2 option ======],
  [])

AC_ARG_ENABLE(strict,
  [  --enable-strict         enable strict mode],
  [CFLAGS="$CFLAGS -Wall"
   echo ==== Enabling -Wall option ======],
  [])

AC_ARG_ENABLE(check,
  [  --enable-check          enable check (unit tests)],
  [echo ==== Enabling tests - make check ======],
  [])

if test "x$with_check" = "xyes"; then
  dnl The check library for tests
  PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

AC_OUTPUT
