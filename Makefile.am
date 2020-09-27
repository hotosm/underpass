#
# Copyright (c) 2020, Humanitarian OpenStreetMap Team
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# * Neither the name of copyright holder nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

AUTOMAKE_OPTIONS = dejagnu

SUBDIRS = testsuite

lib_LTLIBRARIES = libhotosm.la
instdir = /tmp

# inst_HEADERS = hotosm.hh hottm.hh
libhotosm_la_SOURCES = hotosm.cc hottm.cc \
	hottm/tmusers.cc \
	hottm/tmteams.cc \
	hottm/tmprojects.cc
libhotosm_la_LDFLAGS = -avoid-version
SWIGdir = swig

SWIG_LANGS = -python -javascript
SWIG_FLAGS = -Wall -outcurrentdir -c++ -cppext cc -module hotosm
EXTRA_DIST = swig/hotosm.py swig/hotosm.java

# FIXME: add configure test for this
AM_CPPFLAGS = -I/usr/include/webkitgtk-4.0/ -I..

SWIG_INTERFACE = $(libhotosm_la_SOURCES:.cc=.i)
SWIG_OBJECTS = $(libhotosm_la_SOURCES:.cc=.o)
SWIG_PYTHON = $(libhotosm_la_SOURCES:.cc=.py)
SWIG_JAVA = # $(libhotosm_la_SOURCES:.cc=.java)
SWIG_JAVASCRIPT = $(libhotosm_la_SOURCES:.cc=.js)
SWIG_WRAPPERS = $(libhotosm_la_SOURCES:.cc=_wrap.cc)
hotswig_SOURCES := $(foreach dir,swig,$(wildcard $(dir)/*wrap_*.cc))
hotswig_DEPS = swig

swig: libhotosm.la $(SWIG_PYTHON) $(SWIG_JAVASCRIPT) # $(SWIG_JAVA)

AM_JAVACFLAGS=-sourcepath swig/
JAVAROOT = swig

SUFFIXES = .py .cc .java .js
# Generate the wrapper for swig, then build the module
.cc.py:
	$(SWIG) $(SWIG_FLAGS) -python -o swig/$*_wrap_py.cc $(top_srcdir)/$*.i
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I.. -I/usr/include/python3.8 -fPIC -shared swig/hotosm_wrap_py.cc -lpqxx -L.libs .libs/hotosm.o -o swig/_hotosm.so

.cc.java:
	$(SWIG) $(SWIG_FLAGS) -java -package hotosm -o swig/$*_wrap_java.cc $(top_srcdir)/$*.i
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I.. -I/usr/include/python3.8 -fPIC -shared swig/hotosm_wrap_py.cc -lpqxx -L.libs .libs/hotosm.o -o swig/_hotosm.so

.cc.js:
	$(SWIG) $(SWIG_FLAGS) -javascript -node -o swig/$*_wrap_js.cc $(top_srcdir)/$*.i
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I.. -I/usr/include/python3.8 -fPIC -shared swig/hotosm_wrap_js.cc -lpqxx -L.libs .libs/hotosm.o -o swig/_hotosmjs.so

.PHONY: swig

# All of these are generated files
CLEANFILES = swig/* swig/*/*.class

