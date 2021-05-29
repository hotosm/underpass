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

# All of these are generated files
CLEANFILES = all-includes.h.gch all-includes.h

SUBDIRS = testsuite

lib_LTLIBRARIES = libhotosm.la
instdir = /tmp

BOOST_LIBS = $(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LOG_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_IOSTREAMS_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_SERIALIZATION_LIB) \
	$(BOOST_LOCALE_LIB)

# inst_HEADERS = hotosm.hh hottm.hh
libhotosm_la_SOURCES = \
	hotosm.cc hottm.cc \
	hottm/tmusers.cc \
	hottm/tmteams.cc \
	hottm/tmprojects.cc \
	osmstats/osmstats.cc \
	osmstats/changeset.cc \
	osmstats/osmchange.cc \
	osmstats/replication.cc \
	data/geoutil.cc \
	data/pgsnapshot.cc \
	data/import.cc \
	data/osmobjects.cc \
	data/validate.cc \
	data/threads.cc \
	data/underpass.cc
libhotosm_la_LDFLAGS = -avoid-version

bin_PROGRAMS = replicator
replicator_SOURCES = replicator.cc
replicator_LDADD = libhotosm.la $(BOOST_LIBS)

if ENABLE_SWIG
SWIGdir = swig

SWIG_LANGS = -python -javascript
SWIG_FLAGS = -Wall -outcurrentdir -c++ -cppext cc -module hotosm
EXTRA_DIST = swig/hotosm.py swig/hotosm.java

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

SUFFIXES = .py .cc .java .js .hh
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

CLEANFILES += swig/* swig/*/*.class

endif

if ENABLE_DOXYGEN
apidoc:
	$(MAKE) -C doc $@
endif

if ENABLE_PCH
# yes, a little consistency on the suffix would be nice.
.hh.gch:
	$(CXX) $(PCHFLAGS) $(CPPFLAGS) $(CXXFLAGS) $< -o all.h.gch/`basename $<`
.hpp.gch:
	$(CXX) $(PCHFLAGS) $(CPPFLAGS) $(CXXFLAGS) $< -o all.h.gch/`basename $<`
.hxx.gch:
	$(CXX) $(PCHFLAGS) $(CPPFLAGS) $(CXXFLAGS) $< -o all.h.gch/`basename $<`

PCHFLAGS = \
        -I$(top_builddir) \
        -I$(top_srcdir) \
        -I$(top_srcdir)/osmstats \
        -I$(top_srcdir)/data \
        -I$(top_srcdir)/hottm

PCHHEADERS = \
        $(top_srcdir)/hotosm.hh \
        $(top_srcdir)/timer.hh \
        $(top_srcdir)/hottm.hh \
        $(top_srcdir)/osmstats/osmstats.hh \
        $(top_srcdir)/osmstats/replication.hh \
        $(top_srcdir)/osmstats/osmchange.hh \
        $(top_srcdir)/osmstats/changeset.hh \
        $(top_srcdir)/data/import.hh \
        $(top_srcdir)/data/validate.hh \
        $(top_srcdir)/data/pgsnapshot.hh \
        $(top_srcdir)/data/osmobjects.hh \
        $(top_srcdir)/data/geoutil.hh \
        $(top_srcdir)/data/threads.hh \
        $(top_srcdir)/data/underpass.hh \
        $(top_srcdir)/hottm/tmusers.hh \
        $(top_srcdir)/hottm/tmteams.hh \
        $(top_srcdir)/hottm/tmprojects.hh

PCHOTHER = \
	array \
	iostream \
	vector \
	string \
	pqxx/pqxx \
	libxml++/libxml++.h \
	boost/beast.hpp

precomp:=$(subst .hh,.gch,$(PCHHEADERS))

# foo: $(precomp) pqxx.gch
foo: pqxx/pqxx.hxx.gch

all-includes.h: $(PCHHEADERS)
	for i in  $(PCHHEADERS); do \
	    file=`basename $$i`; \
	    echo "#include \"$$file\"" >> all-includes.h ; \
	done;

all-includes.h.gch: all-includes.h
	$(CXX) $(DEFAULT_INCLUDES) $(CPPFLAGS) $(CXXFLAGS) $(PCHHEADERS) \
	/usr/include/boost/*.hpp \
	-o all-includes.h.gch

# $(PCHHEADERS): all-includes.h.gch
endif
