dnl
dnl Copyright (c) 2015-2020, Humanitarian OpenStreetMap Team
dnl All rights reserved.
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions are met:
dnl
dnl * Redistributions of source code must retain the above copyright notice, this
dnl   list of conditions and the following disclaimer.
dnl
dnl * Redistributions in binary form must reproduce the above copyright notice,
dnl   this list of conditions and the following disclaimer in the documentation
dnl   and/or other materials provided with the distribution.
dnl
dnl * Neither the name of copyright holder nor the names of its
dnl   contributors may be used to endorse or promote products derived from
dnl   this software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
dnl AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
dnl IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
dnl DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
dnl FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
dnl DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
dnl SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
dnl CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
dnl OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
dnl OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


AC_CONFIG_HEADERS([hotconfig.h])

AC_PREREQ(2.59c)
AC_INIT(hotosm, 0.1-dev)
LT_INIT
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

dnl AC_CONFIG_HEADERS([hotconfig.h])

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL

CPPFLAGS=$(pkg-config --cflags python)
CPPFLAGS+=" $(pkg-config --cflags libpqxx)"
CPPFLAGS+=" $(pkg-config --cflags glibmm-2.4)"
CPPFLAGS+=" $(pkg-config --cflags gumbo)"
CPPFLAGS+=" $(pkg-config --cflags webkit2gtk-4.0)"

dnl This is a C++ project only
AC_LANG_PUSH(C++)

dnl pqxx requires C++17
CXXFLAGS="-std=c++17 -g -O2"

dnl Osmium headers
AC_CHECK_HEADERS([osmium/osm/node.hpp])

AM_PATH_PYTHON

dnl Swig is used to generate API bindings for other languages
AC_PATH_PROG(SWIG, swig)
if test ! -e swig; then
   mkdir -p swig
fi

AX_JNI_INCLUDE_DIR
CPPFLAGS+=$(echo ${JNI_INCLUDE_DIRS} | sed -e 's/^/-I/' -e 's/ / -I/')

LIBS=$(pkg-config --libs python)
LIBS+=" $(pkg-config --libs expat)"
LIBS+=" $(pkg-config --libs zlib)"
LIBS+=" $(pkg-config --libs bzip2)"
LIBS+=" $(pkg-config --libs libpqxx)"
LIBS+=" $(pkg-config --libs webkit2gtk-4.0)"
LIBS+=" $(pkg-config --libs openssl)"
LIBS+=" $(pkg-config --libs gumbo)"
LIBS+=" $(pkg-config --libs zlib)"
LIBS+=" $(pkg-config --libs glibmm-2.4)"
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
LIBS+=" -lpthread -ldl"

AX_BOOST_BASE(1.69)
AX_BOOST_SYSTEM
AX_BOOST_LOG
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM

dnl LIBS += "${BOOST_DATE_TIME} ${BOOST_SYSTEM} ${BOOST_FILESYSTEM} ${BOOST_LOG_LIB}"
AC_LANG_POP(C++)

AC_PATH_PROG(DOXYGEN, doxygen)
AM_CONDITIONAL(ENABLE_DOXYGEN, [ test x"$DOXYGEN" != x ])

AC_OUTPUT([
Makefile
doc/Makefile
doc/Doxyfile
testsuite/Makefile
testsuite/hotosm/Makefile
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
