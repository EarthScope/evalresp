
AC_INIT([evalresp], [4.0.7], [andrewcooke@isti.com])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile tests/c/Makefile tests/java/Makefile tests/fortran/Makefile tests/robot/Makefile doc/Makefile mxml/Makefile])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL

dnl To select dynamic and/or static libraries use
dnl --enable-shared or --disable-shared
dnl --enable-static or --disable-static
dnl by default, all enabled

dnl Checks for libraries.
AC_CHECK_LIB(m, fabs)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/time.h unistd.h malloc.h stdlib.h)

dnl Checks for library functions.
AC_FUNC_VFORK
AC_CHECK_FUNCS(getcwd regcomp strcspn strstr)

CFLAGS=""

dnl Options processing
AC_ARG_ENABLE(phase_unwrap,
  [  --enable-phase-unwrap   unwrap phase],
  [CFLAGS="$CFLAGS -DUNWRAP_PHASE"
   echo ==== BEWARE: EVALRESP WILL UNWRAP PHASE  ======])

AC_ARG_ENABLE(lib-mode,
  [  --enable-lib-mode       functions of evalresp return, not exit],
  [CFLAGS="$CFLAGS -DLIB_MODE"
   echo ==== evalresp will be compiled in library mode  ======])

AC_ARG_ENABLE(log-label,
  [  --enable-log-label      add NSLC as a label to log messages],
  [CFLAGS="$CFLAGS -DLOG_LABEL"
   echo ==== enabling log-label mode  ======])

AC_ARG_ENABLE(debug,
  [  --enable-debug          enable debug],
  [CFLAGS="$CFLAGS -g"
   echo ==== Enabling DEBUG options ======])

AC_ARG_ENABLE(optimization,
  [  --enable-optimization   enable optimization],
  [CFLAGS="$CFLAGS -O2"
   echo ==== Enabling -O2 option ======],
  [CFLAGS="$CFLAGS -O0"])

AC_ARG_ENABLE(unsafe,
  [  --enable-unsafe         remove compiler checks],
  [echo ==== Disabling -Wall -Werror options ======],
  [CFLAGS="$CFLAGS -Wall -Werror -Wunused-but-set-variable"])

AC_ARG_ENABLE(check,
  [  --enable-check          enable check (unit tests)],
  [with_check="yes"
   echo ==== Enabling tests - make check ======])

if test "x$with_check" = "xyes"; then
  dnl The check library for tests
  PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi

# from mxml configure.ac
if test x$enable_shared != xno; then
	AC_MSG_CHECKING(for shared library support)
	PICFLAG=1

	case "$uname" in
		SunOS* | UNIX_S*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxml.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-h,libmxml.so.1 -G -R\$(libdir) \$(OPTIM)"
			LDFLAGS="$LDFLAGS -R\$(libdir)"
                	;;

		HP-UX*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxml.sl.1"
			DSO="ld"
			DSOFLAGS="$DSOFLAGS -b -z +h libmxml.sl.1 +s +b \$(libdir)"
			LDFLAGS="$LDFLAGS -Wl,+s,+b,\$(libdir)"
			;;

		IRIX)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxml.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-rpath,\$(libdir),-set_version,sgi1.0,-soname,libmxml.so.1 -shared \$(OPTIM)"
			;;

		OSF1* | Linux | GNU)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxml.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-soname,libmxml.so.1,-rpath,\$(libdir) -shared \$(OPTIM)"
                        LDFLAGS="$LDFLAGS -Wl,-rpath,\$(libdir)"
			;;

		*BSD*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxml.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-soname,libmxml.so.1,-R\$(libdir) -shared \$(OPTIM)"
			LDFLAGS="$LDFLAGS -Wl,-R\$(libdir)"
                        ;;

		Darwin*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxml.1.dylib"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS \$(RC_CFLAGS) -dynamiclib -lc"
			;;

		*)
			AC_MSG_RESULT(no)
			AC_MSG_WARN(shared libraries not supported on this platform.)
			PICFLAG=0
			LIBMXML="libmxml.a"
			;;
	esac
else
	PICFLAG=0
	LIBMXML="libmxml.a"
fi
# end of inclusions from mxml

AM_CONDITIONAL([USE_CHECK], [test "x$with_check" = "xyes"])
AC_OUTPUT
