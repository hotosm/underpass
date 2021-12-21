dnl
dnl Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
dnl
dnl This file is part of Underpass.
dnl
dnl     Underpass is free software: you can redistribute it and/or modify
dnl     it under the terms of the GNU General Public License as published by
dnl     the Free Software Foundation, either version 3 of the License, or
dnl     (at your option) any later version.
dnl
dnl     Underpass is distributed in the hope that it will be useful,
dnl     but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl     GNU General Public License for more details.
dnl
dnl     You should have received a copy of the GNU General Public License
dnl     along with Underpass.  If not, see <https:dnlwww.gnu.org/licenses/>.
dnl

AC_PREREQ(2.59c)
AC_INIT(underpass, 0.3_dev)
LT_INIT
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([unconfig.h])

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL
AC_CHECK_HEADER_STDBOOL

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl AC_DISABLE_STATIC
LT_INIT
DLOPEN="-dlopen"
DLPREOPEN="-dlpreopen"
AC_SUBST(DLOPEN)
AC_SUBST(DLPREOPEN)

AC_PATH_PROG(GIT, git)
AC_SUBST(GIT)

build_single_loader=yes
build_multi_loader=no
AC_ARG_ENABLE(threads,
  AS_HELP_STRING([--enable-downloader], [Enable support for multi-threaded downloader (default=single)]),
  [if test -n ${enableval}; then
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    case "${val}" in
        single|s)
            build_single_loader=yes
        ;;
      multi|m)
        build_multi_loader=yes
        ;;
     *) AC_MSG_ERROR([invalid option ${enableval} given (accept: single, multi)])
         ;;
      esac
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done]
])
if test x"${build_single_loader}" = x"yes"; then
  AC_DEFINE(USE_SINGLE_LOADER, [1], [Don't use multiple threaded file downloader])
else
  AC_DEFINE(USE_MULTI_LOADER, [1], [Use multiple threaded downloader])
fi

dnl RapidXML is the default, as it's used by boost
build_rapidxml=no
build_libxml=yes
AC_ARG_ENABLE(parser,
  AS_HELP_STRING([--enable-parser], [Enable support for the specified XML parser (default=libxml++)]),
  [if test -n ${enableval}; then
    enableval=`echo ${enableval} | tr '\054' ' ' `
  fi
  while test -n "${enableval}" ; do
    val=`echo ${enableval} | cut -d ' ' -f 1`
    case "${val}" in
      rapidxml|rapid|r)
        build_rapidxml=yes
        ;;
      libxml++|lib|l)
        build_libxml=yes
        ;;
     *) AC_MSG_ERROR([invalid XML parser ${enableval} given (accept: rapidxml, libxml++)])
         ;;
      esac
    enableval=`echo ${enableval} | cut -d ' ' -f 2-6`
    if test "x$val" = "x$enableval"; then
      break;
    fi
  done]
])

dnl Conflation can be a computationally expensive calulation,
dnl so make it optional. Works best with smaller datasets.
conflation=no
AC_ARG_ENABLE(conflation,
  AS_HELP_STRING([--enable-conflation], [Enable support for expensive conflation (default=no)]),
  [case "${enableval}" in
     yes) conflation=yes ;;
     no)  conflation=no ;;
     *)   AC_MSG_ERROR([bad value ${enableval} for enable-conflation option]) ;;
   esac], conflation=no
)
if test x"${conflation}" = x"yes"; then
  AC_DEFINE(USE_CONFLATION, [1], [Do additional conflation calculations ])
fi

jemalloc=yes
AC_ARG_ENABLE(jemalloc,
  AS_HELP_STRING([--enable-jemalloc], [Enable support for Jemalloc (default=yes)]),
  [case "${enableval}" in
     yes) jemalloc=yes ;;
     no)  jemalloc=no ;;
     *)   AC_MSG_ERROR([bad value ${enableval} for enable-jemalloc option]) ;;
   esac], jemalloc=yes
)
AM_CONDITIONAL([JEMALLOC], [ test x"${jemalloc}" = x"yes" ])
if test x"${jemalloc}" = x"yes"; then
   AC_MSG_NOTICE([Using Jemalloc instead of system malloc])
fi

memory__debug=no
AC_ARG_ENABLE(memory-debug,
  AS_HELP_STRING([--enable-memory-debug], [Enable support for Memory debugging (default=no)]),
  [case "${enableval}" in
     yes) memory_debug=yes ;;
     no)  memory_debug=no ;;
     *)   AC_MSG_ERROR([bad value ${enableval} for enable-memory-debug option]) ;;
   esac]
)
if test x"${memory_debug}" = x"yes"; then
   AC_MSG_NOTICE([Enabling Memory memory debugging for leaks])
   AC_DEFINE(MEMORY_DEBUG, [1], [Enable memory debugging])
fi

conflation=no
AC_ARG_ENABLE(conflation,
  AS_HELP_STRING([--enable-conflation], [Enable support for expensive conflation (default=no)]),
  [case "${enableval}" in
     yes) conflation=yes ;;
     no)  conflation=no ;;
     *)   AC_MSG_ERROR([bad value ${enableval} for enable-conflation option]) ;;
   esac], conflation=no
)
if test x"${conflation}" = x"yes"; then
  AC_DEFINE(USE_CONFLATION, [1], [Do additional conflation calculations ])
fi

dnl range-v3
CPPFLAGS+=" $(pkg-config --cflags libpqxx)"
CPPFLAGS+=" $(pkg-config --cflags gumbo)"
CPPFLAGS+=" $(pkg-config --cflags gdal)"
CPPFLAGS+=" $(pkg-config --cflags ompi)"
CPPFLAGS+=" $(pkg-config --cflags python)"

dnl This is a C++ project only
AC_LANG_PUSH(C++)

dnl pqxx requires C++17
CXXFLAGS+=" -std=c++17 "

dnl Osmium headers
AC_CHECK_HEADERS([osmium/osm/node.hpp])

LIBS+=" $(pkg-config --libs expat)"
LIBS+=" $(pkg-config --libs zlib)"
LIBS+=" $(pkg-config --libs bzip2)"
LIBS+=" $(pkg-config --libs libpqxx)"
LIBS+=" $(pkg-config --libs openssl)"
LIBS+=" $(pkg-config --libs gumbo)"
LIBS+=" $(pkg-config --libs zlib)"
LIBS+=" $(pkg-config --libs gdal)"
LIBS+=" $(pkg-config --libs ompi)"
LIBS+=" $(pkg-config --libs python)"
if test x"${build_libxml}" = x"yes"; then
dnl Debian Buster and Ubuntu Focal ship 2.6, Fedora ships 3,.0
dnl The version in Focal is broken, so either build libxml++-3.0
dnl from source, or wait for 20.10 to be released.
    $(pkg-config --exists libxml++-3.0)
    if test $? -gt 0 ; then
       CPPFLAGS+=" $(pkg-config --cflags libxml++-2.6)"
       LIBS+=" $(pkg-config --libs libxml++-2.6)"
    else
      CPPFLAGS+=" $(pkg-config --cflags libxml++-3.0)"
      LIBS+=" $(pkg-config --libs libxml++-3.0)"
    fi
    AC_DEFINE([LIBXML], [1], [Use libxml++ library])
else
    dnl RapidXML is header only, and we access it through boost, so this
    dnl is unnecessary
    AC_DEFINE([RAPIDXML], [1], [Use rapidxml library in boost])
fi
dnl AM_CONDITIONAL(BUILD_RAPIDXML, [ test x$build_rapidxml = xyes ])
LIBS+=" -lpthread -ldl"

dnl 1.69 or newer
AX_BOOST_BASE(1.69)
AX_BOOST_SYSTEM
AX_BOOST_SERIALIZATION
AX_BOOST_LOG
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM
AX_BOOST_IOSTREAMS
AX_BOOST_THREAD
AX_BOOST_LOCALE
AX_BOOST_TIMER
dnl For some reason this test works on Fedora, and fails on Debian. For now
dnl hardcode it.
dnl AX_BOOST_REGEX
LIBS+=" -lboost_regex"

dnl LIBS += "${BOOST_DATE_TIME} ${BOOST_SYSTEM} ${BOOST_FILESYSTEM} ${BOOST_LOG_LIB}"
AC_LANG_POP(C++)

AC_ARG_ENABLE([python],
  AS_HELP_STRING([--enable-python],[Enable Python binding]), [], [enable_python=yes])

if test x"${enable_python}" = x"yes"; then
  LIBS+=" $(python3-config --libs)"
  LIBS+=" $(pkg-config --libs python3-embed)"
  CPPFLAGS+=" $(python3-config --cflags)"
  AX_BOOST_PYTHON
  AC_DEFINE(USE_PYTHON, [1], [Enable Python binding])
fi
AM_CONDITIONAL(ENABLE_PYTHON, [ test x"${enable_python}" != x ])

AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])
if test x"$DOXYGEN" != x; then
  CPPFLAGS+=" $(pkg-config --cflags webkit2gtk-4.0)"
  CPPFLAGS+=" $(pkg-config --cflags glibmm-2.4)"
  LIBS+=" $(pkg-config --libs webkit2gtk-4.0)"
  LIBS+=" $(pkg-config --libs glibmm-2.4)"
fi

AC_ARG_ENABLE([pch],
  AS_HELP_STRING([--enable-pch],[Enable precompiled header support]), [], [enable_pch=no])

AC_ARG_ENABLE([cache],
  AS_HELP_STRING([--enable-cache],[Enable storing downloaded filea to disk]), [], [enable_cache=yes])
if test x"${enable_cache}" = x"yes"; then
  AC_DEFINE(USE_CACHE, [1], [Store downloaded files to disk])
fi
AC_ARG_ENABLE([tmp],
  AS_HELP_STRING([--enable-tmp],[Enable using a tmp file instead of memory]), [], [enable_tmp=no])
if test x"${enable_tmp}" = x"yes"; then
  AC_DEFINE(USE_TMPFILE, [1], [Use tmp file for downloaded files])
fi

ARCH=${host_cpu}
AC_SUBST(ARCH)
AM_CONDITIONAL([ENABLE_PCH], [test x"$enable_pch" != x"no"])

PCH_FLAGS="-include all-includes.h -Winvalid-pch"
AC_SUBST(PCH_FLAGS)

AM_GNU_GETTEXT([external])
AM_CONDITIONAL([HAS_GETTEXT], [test x"$gt_use_preinstalled_gnugettext" != x"no"])
AC_SUBST([POMAKEFILEDEPS])

SRCDIR=${srcdir}
AC_SUBST(SRCDIR)

AC_CONFIG_FILES([
Makefile
doc/Doxyfile
validate/Makefile
testsuite/Makefile
testsuite/libunderpass.all/Makefile
redhat/underpass.spec
po/Makefile
])

AC_OUTPUT

if test x"${build_libxml}" = x"yes"; then
   echo "Using libxml for XML parsing"
else
   echo "Using RaqpidXML for XML parsing, which is used by boost::parse_tree"
fi
# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
