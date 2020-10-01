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

dnl AC_CONFIG_HEADERS([hotconfig.h])

AC_PROG_CC
AC_PROG_CXX
AC_EXEEXT
AC_PROG_INSTALL

CPPFLAGS=$(pkg-config -cflags python)

dnl This is a C++ project only
AC_LANG_PUSH(C++)

dnl pqxx requires C++17
CXXFLAGS="-std=c++17 -g -O2"
AC_CHECK_HEADERS([pqxx/pqxx])
dnl AC_CHECK_LIB(pqxx, [])

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

# CPPFLAGS+=$(pkg-config -cflags webkit2gtk-4.0)
# #include <boost/date_time.hpp>
# /usr/lib64/libboost_date_time.so
CPPFLAGS+=" "
CPPFLAGS+=$(pkg-config --cflags libxml++-3.0)
AC_CHECK_HEADERS([libxml++/libxml++.h])
LIBS+=$(pkg-config --libx libxml++-3.0)

AC_LANG_POP(C++)


AC_OUTPUT([
Makefile
testsuite/Makefile
testsuite/hotosm/Makefile
])

# Local Variables:
# c-basic-offset: 2
# tab-width: 2
# indent-tabs-mode: nil
# End:
