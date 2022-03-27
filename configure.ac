
AC_INIT([evalresp],[5.0.1],[software-owner@iris.washington.edu])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([Makefile libsrc/Makefile src/Makefile tests/Makefile tests/c/Makefile tests/java/Makefile tests/fortran/Makefile tests/robot/Makefile tests/jenkins/Makefile doc/Makefile libsrc/mxml/Makefile libsrc/evalresp_log/Makefile libsrc/spline/Makefile libsrc/evalresp/Makefile])
AM_CONFIG_HEADER([config.h libsrc/mxml/config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects])

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
AC_CHECK_HEADERS(sys/time.h unistd.h malloc.h stdlib.h getopt.h)

dnl Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS(getcwd regcomp strcspn strstr)


# from mxml configure.ac

uname=`uname`
uversion=`uname -r | sed -e '1,$s/[[^0-9]]//g'`
if test x$uname = xIRIX64; then
	uname="IRIX"
fi

dnl Flags for "ar" command...
case "$uname" in
        Darwin* | *BSD*)
                ARFLAGS="-rcv"
                ;;
        *)
                ARFLAGS="crvs"
                ;;
esac

AC_SUBST(ARFLAGS)

if test x$enable_shared != xno; then
	AC_MSG_CHECKING(for shared library support)
	PICFLAG=1

	case "$uname" in
		SunOS* | UNIX_S*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxmlev.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-h,libmxmlev.so.1 -G -R\$(libdir) \$(OPTIM)"
			LDFLAGS="$LDFLAGS -R\$(libdir)"
                	;;

		HP-UX*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxmlev.sl.1"
			DSO="ld"
			DSOFLAGS="$DSOFLAGS -b -z +h libmxmlev.sl.1 +s +b \$(libdir)"
			LDFLAGS="$LDFLAGS -Wl,+s,+b,\$(libdir)"
			;;

		IRIX)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxmlev.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-rpath,\$(libdir),-set_version,sgi1.0,-soname,libmxmlev.so.1 -shared \$(OPTIM)"
			;;

		OSF1* | Linux | GNU)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxmlev.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-soname,libmxmlev.so.1,-rpath,\$(libdir) -shared \$(OPTIM)"
                        LDFLAGS="$LDFLAGS -Wl,-rpath,\$(libdir)"
			;;

		*BSD*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxmlev.so.1.5"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS -Wl,-soname,libmxmlev.so.1,-R\$(libdir) -shared \$(OPTIM)"
			LDFLAGS="$LDFLAGS -Wl,-R\$(libdir)"
                        ;;

		Darwin*)
			AC_MSG_RESULT(yes)
			LIBMXML="libmxmlev.1.dylib"
			DSO="\$(CC)"
			DSOFLAGS="$DSOFLAGS \$(RC_CFLAGS) -dynamiclib -lc"
			;;

		*)
			AC_MSG_RESULT(no)
			AC_MSG_WARN(shared libraries not supported on this platform.)
			PICFLAG=0
			LIBMXML="libmxmlev.a"
			;;
	esac
else
	PICFLAG=0
	LIBMXML="libmxmlev.a"
fi

dnl Add -Wall for GCC...
if test -n "$GCC"; then
	CFLAGS="-Wall $CFLAGS"

	if test "x$OPTIM" = x; then
		OPTIM="-Os -g"
	fi

	if test "x$use_ansi" = xyes; then
		CFLAGS="-ansi -pedantic $CFLAGS"
	fi

	if test $PICFLAG = 1 -a $uname != AIX; then
    		OPTIM="-fPIC $OPTIM"
	fi
else
	case $uname in
		HP-UX*)
			CFLAGS="-Ae $CFLAGS"

			if test "x$OPTIM" = x; then
				OPTIM="-O"
			fi

			OPTIM="+DAportable $OPTIM"

			if test $PICFLAG = 1; then
				OPTIM="+z $OPTIM"
			fi
			;;

		UNIX_SVR* | SunOS*)
			if test "x$OPTIM" = x; then
				OPTIM="-O"
			fi

			if test $PICFLAG = 1; then
				OPTIM="-KPIC $OPTIM"
			fi
			;;

		*)
			if test "x$OPTIM" = x; then
				OPTIM="-O"
			fi
			;;
	esac
fi

AC_CHECK_FUNCS(snprintf vsnprintf)
AC_CHECK_FUNCS(strdup)

dnl don't support multithread use
PTHREAD_FLAGS=""
AC_SUBST(PTHREAD_FLAGS)
dnl don't support these options
ARCHFLAGS=""
AC_SUBST(ARCHFLAGS)

AC_SUBST(DSO)
AC_SUBST(DSOFLAGS)
AC_SUBST(PICFLAG)
AC_SUBST(OPTIM)
AC_SUBST(LIBMXML)

AC_PATH_PROG(AR,ar)
AC_PATH_PROG(CP,cp)
AC_PATH_PROG(LN,ln)
AC_PATH_PROG(MKDIR,mkdir)
AC_PATH_PROG(RM,rm)

# end of inclusions from mxml


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

AM_CONDITIONAL([USE_CHECK], [test "x$with_check" = "xyes"])
AC_OUTPUT
