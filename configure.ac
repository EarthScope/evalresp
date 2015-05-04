
AC_INIT([evalresp], [4.0.0], [andrewcooke@isti.com])
AM_INIT_AUTOMAKE([foreign])
AM_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile doc/Makefile])

dnl The check library for tests
AM_PATH_CHECK()

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
  [  --enable-phase-unwrap   Unwrap phase],
  [CFLAGS="$CFLAGS -DUNWRAP_PHASE"
   echo ==== BEWARE: EVALRESP WILL UNWRAP PHASE  ======],
  []) 

AC_ARG_ENABLE(lib-mode,
 [  --enable-lib-mode        Functions of evalresp return, not exit],
 [CFLAGS="$CFLAGS -DLIB_MODE"
  echo ==== evalresp will be compiled in library mode  ======],
 [])

AC_ARG_ENABLE(log-label,
  [  --enable-log-label      Add NSLC as a label to log messages],
  [CFLAGS="$CFLAGS -DLOG_LABEL"
   echo ==== enabling log-label mode  ======],
  [])

AC_ARG_ENABLE(debug,
  [  --enable-debug          Enable debug],
  [CFLAGS="$CFLAGS -g"
   echo ==== Enabling DEBUG options ======],
  [])

AC_ARG_ENABLE(optimization,
  [  --enable-optimization   Enable optimization],
  [CFLAGS="$CFLAGS -02"
   echo ==== Enabling -O2 option ======],
  [])

AC_ARG_ENABLE(strict,
  [  --enable-strict        Enable strict mode],
  [CFLAGS="$CFLAGS -Wall"
   echo ==== Enabling -Wall option ======],
  [])


AC_OUTPUT
