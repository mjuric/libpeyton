dnl RS_BOOST([MINIMUM-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost C++ libraries of a particular version (or newer)
dnl Defines:
dnl   BOOST_CPPFLAGS to the set of flags required to compiled Boost
AC_DEFUN([RS_BOOST], 
[
  AC_SUBST(BOOST_CPPFLAGS)
  AC_SUBST(BOOST_LIBS)
  BOOST_CPPFLAGS=""
  path_given="no"

dnl Extract the path name from a --with-boost=PATH argument
  AC_ARG_WITH(boost,
    AC_HELP_STRING([--with-boost=PATH],[absolute path name where the Boost C++ libraries reside, or `int', for internal library. Alternatively, the BOOST_ROOT environment variable will be used]),
    [
    if test "$withval" != yes ; then
        BOOST_ROOT="$withval"
    fi
    ]
  )

  if test "x$BOOST_ROOT" = x ; then
    BOOST_ROOT="/usr/local"
  fi

  boost_min_version=ifelse([$1], ,1.20.0,[$1])
  WANT_BOOST_MAJOR=`expr $boost_min_version : '\([[0-9]]\+\)'`
  WANT_BOOST_MINOR=`expr $boost_min_version : '[[0-9]]\+\.\([[0-9]]\+\)'`
  WANT_BOOST_SUB_MINOR=`expr $boost_min_version : '[[0-9]]\+\.[[0-9]]\+\.\([[0-9]]\+\)'`

  BOOST_CPPFLAGS="-I$BOOST_ROOT/include/boost-${WANT_BOOST_MAJOR}_$WANT_BOOST_MINOR"

  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
  AC_MSG_CHECKING([for the Boost C++ libraries, version $boost_min_version or newer])
  AC_TRY_COMPILE(
    [
#include <boost/version.hpp>
],
    [],
    [
      have_boost="yes"
    ],
    [
      AC_MSG_RESULT(no)
      have_boost="no"
      ifelse([$3], , :, [$3])
    ])

  if test "$have_boost" = "yes"; then
    WANT_BOOST_VERSION=`expr $WANT_BOOST_MAJOR \* 100000 \+ $WANT_BOOST_MINOR \* 100 \+ $WANT_BOOST_SUB_MINOR`

    AC_TRY_COMPILE(
      [
#include <boost/version.hpp>
],
      [
#if BOOST_VERSION >= $WANT_BOOST_VERSION
// Everything is okay
#else
#  error Boost version is too old
#endif

],
      [
	AC_MSG_RESULT(yes)
	if test "$target_os" = "mingw32"; then
	   boost_libsuff=mgw
	else
	   boost_libsuff=gcc
	fi
	boost_libsuff_r=$boost_libsuff-mt;
	if test "x$enable_debug" = xyes ; then
	    boost_libsuff=$boost_libsuff-d;
	    boost_libsuff_r=$boost_libsuff_r-d;
	fi
	boost_libsuff=$boost_libsuff-${WANT_BOOST_MAJOR}_$WANT_BOOST_MINOR
	boost_libsuff_r=$boost_libsuff_r-${WANT_BOOST_MAJOR}_$WANT_BOOST_MINOR
        ifelse([$2], , :, [$2])
      ],
      [
        AC_MSG_RESULT([no, version of installed Boost libraries is too old])
        ifelse([$3], , :, [$3])
      ])
  fi
  CPPFLAGS="$OLD_CPPFLAGS"
  AC_LANG_RESTORE
])


dnl RS_BOOST_THREAD([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost thread library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_thread
AC_DEFUN([RS_BOOST_THREAD], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_thread library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS -D_REENTRANT"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_thread-$boost_libsuff_r"
    AC_TRY_COMPILE(
	    [ 
		#include <boost/thread.hpp> 
		bool bRet = 0;
		void thdfunc() { bRet = 1; }
	    ],
	    [
		boost::thread thrd(&thdfunc);
		thrd.join();
		return bRet == 1;
	    ], 
	    [
		AC_MSG_RESULT([yes])
		ifelse([$1], , :, [$1])
	    ],
	    [ 
		AC_MSG_RESULT([no])
		ifelse([$2], , :, [$2])
	    ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])
	    
dnl RS_BOOST_DATETIME([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost datetime library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_datetime
AC_DEFUN([RS_BOOST_DATETIME], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_datetime library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_date_time-$boost_libsuff"
    AC_TRY_COMPILE(
	    [ 
		#include <boost/date_time/gregorian/gregorian.hpp> 
	    ],
	    [
		using namespace boost::gregorian;
		date d = from_string("1978-01-27");
		return d == boost::gregorian::date(1978, Jan, 27);
	    ], 
	    [
		AC_MSG_RESULT([yes])
		ifelse([$1], , :, [$1])
	    ],
	    [ 
		AC_MSG_RESULT([no])
		ifelse([$2], , :, [$2])
	    ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS=$OLD_LIBS
    AC_LANG_RESTORE
])
	    
dnl RS_BOOST_REGEX([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost regex library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_datetime
AC_DEFUN([RS_BOOST_REGEX], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_regex library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_regex-$boost_libsuff"
    AC_TRY_COMPILE(
	    [ 
		#include <boost/regex.hpp> 
	    ],
	    [
		using namespace boost;
		cmatch what;
		if(!regex_match("27/01/78",what,regex("(\\\d+)/(\\\d+)/(\\\d+)")))
		    return 0;

		return what[1]=="27" && what[2]=="01" && what[3]=="78";
	    ], 
	    [
		AC_MSG_RESULT([yes])
		ifelse([$1], , :, [$1])
	    ],
	    [ 
		AC_MSG_RESULT([no])
		ifelse([$2], , :, [$2])
	    ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl RS_BOOST_PROGRAM_OPTIONS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost regex library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_datetime
AC_DEFUN([RS_BOOST_PROGRAM_OPTIONS], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_program_options library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_program_options-$boost_libsuff"
    AC_TRY_COMPILE(
	    [ 
		#include <boost/program_options.hpp> 
	    ],
	    [
		using namespace boost::program_options;
		return 0;
	    ], 
	    [
		AC_MSG_RESULT([yes])
		ifelse([$1], , :, [$1])
	    ],
	    [ 
		AC_MSG_RESULT([no])
		ifelse([$2], , :, [$2])
	    ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl RS_BOOST_IOSTREAMS([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl Test for the Boost iostreams library
dnl Defines
dnl   BOOST_LDFLAGS to the set of flags required to compile boost_iostreams
AC_DEFUN([RS_BOOST_IOSTREAMS], 
[
    AC_REQUIRE([RS_BOOST])
  AC_MSG_CHECKING([whether we can use boost_iostreams library])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  OLD_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$BOOST_CPPFLAGS"
  OLD_LIBS="$LIBS"
  LIBS="-lboost_iostreams-$boost_libsuff"
    AC_TRY_COMPILE(
	    [ 
		#include <boost/iostreams/device/file.hpp> 
	    ],
	    [
		using namespace boost::iostreams;
		file_sink fs("bla.bin");
		return 0;
	    ], 
	    [
		AC_MSG_RESULT([yes])
		ifelse([$1], , :, [$1])
	    ],
	    [ 
		AC_MSG_RESULT([no])
		ifelse([$2], , :, [$2])
	    ])

    AC_SUBST(BOOST_CPPFLAGS)
    AC_SUBST(BOOST_LIBS)
    AC_SUBST(BOOST_LIBS_R)
    BOOST_CPPFLAGS="$CPPFLAGS"
    BOOST_LIBS="$LIBS $BOOST_LIBS"
    BOOST_LIBS_R="$LIBS $BOOST_LIBS_R"
    CPPFLAGS="$OLD_CPPFLAGS"
    LIBS="$OLD_LIBS"
    AC_LANG_RESTORE
])

dnl @synopsis AC_caolan_CHECK_PACKAGE(PACKAGE, FUNCTION, LIBRARY , HEADERFILE [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Provides --with-PACKAGE, --with-PACKAGE-include and --with-PACKAGE-libdir
dnl options to configure. Supports the now standard --with-PACKAGE=DIR 
dnl approach where the package's include dir and lib dir are underneath DIR,
dnl but also allows the include and lib directories to be specified seperately
dnl
dnl adds the extra -Ipath to CFLAGS if needed 
dnl adds extra -Lpath to LD_FLAGS if needed
dnl searches for the FUNCTION in the LIBRARY with 
dnl AC_CHECK_LIBRARY and thus adds the lib to LIBS
dnl
dnl defines HAVE_PKG_PACKAGE if it is found, (where PACKAGE in the 
dnl HAVE_PKG_PACKAGE is replaced with the actual first parameter passed)
dnl note that autoheader will complain of not having the HAVE_PKG_PACKAGE and you 
dnl will have to add it to acconfig.h manually
dnl
dnl @version $Id: acinclude.m4,v 1.3 2005/08/29 02:33:11 mjuric Exp $
dnl @author Caolan McNamara <caolan@skynet.ie>
dnl
dnl with fixes from...
dnl Alexandre Duret-Lutz <duret_g@lrde.epita.fr>

AC_DEFUN(AC_caolan_CHECK_PACKAGE,
[

AC_ARG_WITH($1,
[  --with-$1[=DIR]	root directory of $1 installation],
with_$1=$withval 
if test "${with_$1}" != yes; then
	$1_include="$withval/include" 
	$1_libdir="$withval/lib"
fi
)

AC_ARG_WITH($1-include,
[  --with-$1-include=DIR        specify exact include dir for $1 headers],
$1_include="$withval")

AC_ARG_WITH($1-libdir,
[  --with-$1-libdir=DIR        specify exact library dir for $1 library
  --without-$1        disables $1 usage completely], 
$1_libdir="$withval")

if test "${with_$1}" != no ; then
	OLD_LIBS=$LIBS
	OLD_LDFLAGS=$LDFLAGS
	OLD_CFLAGS=$CFLAGS
	OLD_CPPFLAGS=$CPPFLAGS

	if test "${$1_libdir}" ; then
		LDFLAGS="$LDFLAGS -L${$1_libdir}"
	fi
	if test "${$1_include}" ; then
		CPPFLAGS="$CPPFLAGS -I${$1_include}"
		CFLAGS="$CFLAGS -I${$1_include}"
	fi

	AC_CHECK_LIB($3,$2,,no_good=yes)
	AC_CHECK_HEADER($4,,no_good=yes)
	if test "$no_good" = yes; then
dnl	broken
		ifelse([$6], , , [$6])
		
		LIBS=$OLD_LIBS
		LDFLAGS=$OLD_LDFLAGS
		CPPFLAGS=$OLD_CPPFLAGS
		CFLAGS=$OLD_CFLAGS
	else
dnl	fixed
		ifelse([$5], , , [$5])

		AC_DEFINE(HAVE_PKG_$1, 1, Define if you have the package )
	fi

fi

])
