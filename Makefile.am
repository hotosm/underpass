#
# Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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
ACLOCAL_AMFLAGS = -I m4

# All of these are generated files
CLEANFILES = \
	all-includes.h.gch \
	all-includes.h

DISTCLEANFILES = doc/html doc/latex .lastmod
MAINTAINERCLEANFILES = revno.h .lastmod

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

SQL_FILES = \
	data/underpass.sql \
	data/osmstats.sql \
	data/geoboundaries.sql

# inst_HEADERS = hotosm.hh hottm.hh
libhotosm_la_SOURCES = \
	timer.hh \
	hotosm.cc hotosm.hh \
	hottm.cc hottm.hh \
	hottm/tmusers.cc hottm/tmusers.hh \
	hottm/tmteams.cc hottm/tmteams.hh \
	hottm/tmprojects.cc hottm/tmprojects.hh \
	osmstats/osmstats.cc osmstats/osmstats.hh \
	osmstats/changeset.cc osmstats/changeset.hh \
	osmstats/osmchange.cc osmstats/osmchange.hh \
	osmstats/replication.cc osmstats/replication.hh \
	data/geoutil.cc data/geoutil.hh \
	data/pgsnapshot.cc data/pgsnapshot.hh\
	data/import.cc data/import.hh \
	data/osmobjects.cc data/osmobjects.hh \
	data/validate.cc data/validate.hh \
	data/threads.cc data/threads.hh \
	data/underpass.cc data/underpass.hh
libhotosm_la_LDFLAGS = -avoid-version

bin_PROGRAMS = replicator
replicator_SOURCES = replicator.cc
replicator_LDADD = libhotosm.la $(BOOST_LIBS)

pkgdata_DATA =$(SQL_FILES)

EXTRA_DIST = \
	$(SQL_FILES) \
	doc \
	revno.h \
	redhat/rpm.am

# if ENABLE_SWIG
# SWIGdir = swig

# SWIG_LANGS = -python -javascript
# SWIG_FLAGS = -Wall -outcurrentdir -c++ -cppext cc -module hotosm
# EXTRA_DIST = swig/hotosm.py swig/hotosm.java

# SWIG_INTERFACE = $(libhotosm_la_SOURCES:.cc=.i)
# SWIG_OBJECTS = $(libhotosm_la_SOURCES:.cc=.o)
# SWIG_PYTHON = $(libhotosm_la_SOURCES:.cc=.py)
# SWIG_JAVA = # $(libhotosm_la_SOURCES:.cc=.java)
# SWIG_JAVASCRIPT = $(libhotosm_la_SOURCES:.cc=.js)
# SWIG_WRAPPERS = $(libhotosm_la_SOURCES:.cc=_wrap.cc)
# hotswig_SOURCES := $(foreach dir,swig,$(wildcard $(dir)/*wrap_*.cc))
# hotswig_DEPS = swig

# swig: libhotosm.la $(SWIG_PYTHON) $(SWIG_JAVASCRIPT) # $(SWIG_JAVA)

# AM_JAVACFLAGS=-sourcepath swig/
# JAVAROOT = swig

# SUFFIXES = .py .cc .java .js .hh
# # Generate the wrapper for swig, then build the module
# .cc.py:
# 	$(SWIG) $(SWIG_FLAGS) -python -o swig/$*_wrap_py.cc $(top_srcdir)/$*.i
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I.. -I/usr/include/python3.8 -fPIC -shared swig/hotosm_wrap_py.cc -lpqxx -L.libs .libs/hotosm.o -o swig/_hotosm.so

# .cc.java:
# 	$(SWIG) $(SWIG_FLAGS) -java -package hotosm -o swig/$*_wrap_java.cc $(top_srcdir)/$*.i
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I.. -I/usr/include/python3.8 -fPIC -shared swig/hotosm_wrap_py.cc -lpqxx -L.libs .libs/hotosm.o -o swig/_hotosm.so

# .cc.js:
# 	$(SWIG) $(SWIG_FLAGS) -javascript -node -o swig/$*_wrap_js.cc $(top_srcdir)/$*.i
# 	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I.. -I/usr/include/python3.8 -fPIC -shared swig/hotosm_wrap_js.cc -lpqxx -L.libs .libs/hotosm.o -o swig/_hotosmjs.so

# CLEANFILES += swig/* swig/*/*.class doc/html doc/latex

# endif

if ENABLE_DOXYGEN
apidoc:
	(cd doc && $(DOXYGEN))
endif

install-data-hook:
	$(MKDIR_P) $(DESTDIR)/$(docdir)
	cp -ruvp $(srcdir)/doc/*.md $(DESTDIR)/$(docdir)

dist-hook: apidoc
	$(MKDIR_P) $(DESTDIR)/$(docdir)
	cp -ruvp doc/html $(DESTDIR)/$(docdir)

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

#
# The rest of this file is only used to build snapshots and releases
#
include $(srcdir)/redhat/rpm.am
include $(srcdir)/debian/deb.am

NOW := $(shell date "+%Y%m%d")

# The branch nickname and revision number must be set before including
# the other Makefile fragments used for package building, as they use
# these values.
BRANCH_REVNO  = $(shell if test -f revno.h; then grep "REVNO" revno.h | cut -d '"' -f 2; fi)
BRANCH_NICK   = $(shell if test -f revno.h; then grep "NICK" revno.h | cut -d '"' -f 2; fi)
BRANCH_COMMIT = $(shell if test -f revno.h; then grep "COMMIT" revno.h | cut -d '"' -f 2; fi)

# this is used for Debian style naming conventions
SNAPSHOT_VERSION = $(BRANCH_NICK)$(BRANCH_COMMIT)

# FIXME: for now use the commit instead of the revision while under heavy
# deelopment since the version won't be changing as frequently as the
# commits.
distdir = $(PACKAGE)-$(BRANCH_COMMIT)
# these is the directories where the snapshot gets built. Sources are
# in the top level, the build goes in a _build subdirectory
SNAPSHOT_DIR = $(distdir)
SNAPSHOT_BUILD = $(SNAPSHOT_DIR)/_build
SNAPSHOT_NAME = $(SNAPSHOT_DIR)

# this is the name of the actual tarball
SNAPSHOT_TAR = underpass-$(SNAPSHOT_VERSION).tar.gz
# this is the temporary directory used for DESTDIR
SNAPSHOT_TMP = /tmp/$(SNAPSHOT_DIR)

# Do the entire process to build a binary tarball
snapshot: snapshot-src snapshot-configure snapshot-build snapshot-install snapshot-bundle

# start by creating the source tree using the distdir target,
# which builds a source tree only using what's in the DIST variables
# from Automake. This uses the default version for Underpass. which is
# 'master'. Then it gets renamed from the default version from 'master'
# to a branch-revno stamped version instead of master.
snapshot-src: revno.h distdir

snapshot-configure:
	-@now=`date "+%Y%m%d"`; \
	pkgname="underpass-$(BRANCH_NICK)-${BRANCH_COMMIT}"; \
	if test ! -d $(SNAPSHOT_BUILD); then \
	  $(MKDIR_P) $(SNAPSHOT_BUILD); \
	fi; \
	cd $(SNAPSHOT_BUILD) ; ../configure \
	$(SNAPSHOT_CONFIGURE_FLAGS) \
	  --disable-dependency-tracking \
	  --prefix=/usr \
	  --mandir=/usr/share/man \
	  --infodir=/usr/share/info \
	  --disable-static \
	  --enable-shared

snapshot-build:
	$(MAKE) $(AM_MAKEFLAGS) -C $(SNAPSHOT_BUILD)

# Install a package. Afterwards we link to the prebuilt man pages incase
# DocBook and DocBook2X aren't installed on this machine.
snapshot-install:
	$(MAKE) $(AM_MAKEFLAGS) -C $(SNAPSHOT_BUILD) apidoc install DESTDIR=$(SNAPSHOT_TMP)

# We only want to bundle an installed underpass, so all the linker paths are
# correct
snapshot-bundle:
	snapshot_tar="$(SNAPSHOT_TMP)-$(host_cpu)-$(host_os).tar.gz"; \
	if test ! -d $${snapshot_tmp}; then \
	  $(mkinstalldirs) $${snapshot_tmp}; \
	fi; \
	rm -f $${snapshot_dest}/usr/lib/*.la; \
	cp @srcdir@/config.guess $${snapshot_tmp}; \
	cp @srcdir@/packaging/install-underpass.sh $${snapshot_tmp}; \
	cp -rp $${snapshot_dest}/usr/bin $${snapshot_tmp}; \
	cp -rp $${snapshot_dest}/usr/lib $${snapshot_tmp}; \
	cp -rp $${snapshot_dest}/usr/share $${snapshot_tmp}; \
	strip $${snapshot_tmp}/bin/*-underpass; \
	tar zcvCf /tmp $${snapshot_tar} $(SNAPSHOT_NAME)

snapshot-clean:
	-rm -fr $(distdir) $(SNAPSHOT_TMP)

.PHONY : snapshot snapshot-build snapshot-install snapshot-clean snapshot-src snapshot-configure

#
# Extract info from the repository to include in the build.
# If top source dir is not a git repository or git is not installed,
# keep the current revno.h file (which should be part of any distribution).
# It's primarily used by the testsuites and when building binary packages
# from multiple branches.
#
BUILT_SOURCES = revno.h .configline
revno.h:
	@echo "Getting build info for revno.h"; \
	revno="`cd $(top_srcdir) && $(GIT) rev-list HEAD | wc -l`"; \
	comm_id="`cd $(top_srcdir) && $(GIT) rev-parse --short HEAD`"; \
	nick="`cd $(top_srcdir) && $(GIT) symbolic-ref -q HEAD`"; \
	if test \! -z "$${revno}"; then \
	  echo "Generating revno.h ($${nick} $${revno} $${comm_id})"; \
	  echo "#define BRANCH_REVNO \"$${revno}\"" > revno.h; \
	  echo "#define BRANCH_NICK \"$${nick}\"" >> revno.h; \
	  echo "#define COMMIT_ID \"$${comm_id}\"" >> revno.h; \
	  echo "#define VERSION \"$(VERSION)\"" >> revno.h; \
	  touch .lastmod; \
	fi

.configline: revno.h
	-@rm -f .configline
	@head config.log | grep " .*/configure " | sed -e 's:^	. .*configure ::' > .configline
