#
# Copyright (c) 2020, 2021, 2022, 2023, 2024 Humanitarian OpenStreetMap Team
#
# This file is part of Underpass.
#
#     Underpass is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Underpass is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.


DEFAULT_INCLUDES = -I . -I $(srcdir)/src

AUTOMAKE_OPTIONS = dejagnu subdir-objects
ACLOCAL_AMFLAGS = -I m4

# All of these are generated files
CLEANFILES = \
	all-includes.h.gch \
	all-includes.h

DISTCLEANFILES = docs/html docs/latex .lastmod
MAINTAINERCLEANFILES = revno.h .lastmod

SUBDIRS = \
	src/validate

lib_LTLIBRARIES = libunderpass.la
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
	$(BOOST_LOCALE_LIB) \
	$(BOOST_TIMER_LIB) \
	$(BOOST_PYTHON_LIB)

SQL_FILES = \
	setup/db/underpass.sql

libunderpass_la_SOURCES = \
	src/utils/log.cc src/utils/log.hh \
	src/dsodefs.hh src/gettext.h \
	src/underpassconfig.hh \
	src/stats/querystats.cc src/stats/querystats.hh \
	src/raw/queryraw.cc src/raw/queryraw.hh \
	src/stats/statsconfig.hh src/stats/statsconfig.cc \
	src/validate/queryvalidate.cc src/validate/queryvalidate.hh \
	src/osm/changeset.cc src/osm/changeset.hh \
	src/osm/osmchange.cc src/osm/osmchange.hh \
	src/osm/osmobjects.cc src/osm/osmobjects.hh \
	src/replicator/replication.cc src/replicator/replication.hh \
	src/replicator/planetreplicator.cc src/replicator/planetreplicator.hh \
	src/replicator/threads.cc src/replicator/threads.hh \
	src/bootstrap/bootstrap.cc src/bootstrap/bootstrap.hh \
	src/utils/geoutil.cc src/utils/geoutil.hh \
	src/utils/geo.cc src/utils/geo.hh \
	src/utils/yaml.hh src/utils/yaml.cc \
	src/data/pq.hh src/data/pq.cc \
	setup/db/setupdb.sh

if JEMALLOC
libunderpass_la_LDFLAGS = -avoid-version -ljemalloc
else
libunderpass_la_LDFLAGS = -avoid-version
endif

bin_PROGRAMS = underpass
underpass_SOURCES = src/underpass.cc 
underpass_LDADD = libunderpass.la $(BOOST_LIBS)

if JEMALLOC
AM_CXXFLAGS = \
	-rdynamic \
	-fPIC \
	-Wno-deprecated-declarations \
	-fno-builtin-malloc \
	-fno-builtin-calloc \
	-fno-builtin-realloc \
	-fno-builtin-free
else
AM_CXXFLAGS = \
	-rdynamic \
	-fPIC \
	-Wno-deprecated-declarations
endif

AM_CPPFLAGS = \
	-DPKGLIBDIR=\"$(pkglibdir)\" \
	-DSRCDIR=\"$(srcdir)\" \
	-DBOOST_LOCALE_HIDE_AUTO_PTR
# Optionally timing can be turned on, which produces volumes of output which adds too much
# clutter to the output. This should only be enabled when doing performance tuning.
	-DTIMING_DEBUG

pkgdata_DATA = $(SQL_FILES) setup/db/setupdb.sh $(PYTHON_UTILS)

EXTRA_DIST = \
	$(SQL_FILES) \
	$(PYTHON_UTILS) \
	src/testsuite \
	config/priority.geojson \
	config/replicator/planetreplicator.yaml \
	doc \
	dist/debian \
	dist/redhat \
	utils
	

DEJATOOL = libunderpass

if ENABLE_PYTHON
EXT := $(shell python3-config --extension-suffix)
DIR := $(shell python3-config --configdir)
noinst_LTLIBRARIES = underpass.la
underpass_la_SOURCES = src/wrappers/python.cc $(libunderpass_la_SOURCES)
underpass_la_LDFADD = $(BOOST_LIBS) libunderpass.la src/validate/libunderpass.la
underpass_la_LDFLAGS = -module -avoid-version -no-undefined -rpath /usr/lib64/ src/validate/libunderpass.la $(BOOST_LIBS)
PY_OBJECTS = $(libunderpass_la_OBJECTS:.lo=.o)
# /usr/lib/python3.9/lib-dynload/underpass.cpython-39-x86_64-linux-gnu.so
install-python: underpass.la
	@if test x"$(prefix)" = x"/usr/lib" -a "$(shell whoami)" != "root"; then \
	   echo "Need to be root!"; \
	else \
	   moddir=$(shell dirname $(DIR))/lib-dynload; \
	   if test ! -e $${moddir}; then \
	     $(MDIR_P) $${moddir}; \
	   fi; \
	   cp .libs/libunderpass.so $${moddir}/libunderpass$(EXT); \
	fi

endif

if ENABLE_DOXYGEN
apidoc:
	(cd docs && $(DOXYGEN))
endif

install-data-hook:
	$(MKDIR_P) $(DESTDIR)/$(pkglibdir)
	$(MKDIR_P) /etc/underpass
	cp -rvp $(srcdir)/config/priority.geojson /etc/underpass/
	cp -rvp $(srcdir)/config/replicator /etc/underpass/
	cp -rvp $(srcdir)/config/stats /etc/underpass/
	cp -rvp $(srcdir)/config/default.yaml /etc/underpass/
	$(MKDIR_P) $(DESTDIR)/$(docdir)
	cp -rvp $(srcdir)/docs/*.md $(DESTDIR)/$(docdir)
	cp -rvp $(srcdir)/setup/service $(DESTDIR)/$(pkglibdir)

dist-hook: apidoc
	$(MKDIR_P) $(DESTDIR)/$(docdir)
	cp -rvp docs/html $(DESTDIR)/$(docdir)

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
        -I$(top_srcdir)/src \
        -I$(top_srcdir)/src/stats \
		-I$(top_srcdir)/src/validate \
        -I$(top_srcdir)/src/data

PCHHEADERS = \
        $(top_srcdir)/src/replicator/planetreplicator.hh \
        $(top_srcdir)/src/stats/querystats.hh \
		$(top_srcdir)/src/raw/queryraw.hh \
        $(top_srcdir)/src/replicator/replication.hh \
        $(top_srcdir)/src/osm/osmchange.hh \
        $(top_srcdir)/src/osm/changeset.hh \
        $(top_srcdir)/src/osm/osmobjects.hh \
        $(top_srcdir)/src/validate/validate.hh \
        $(top_srcdir)/src/utils/geoutil.hh \
		$(top_srcdir)/src/utils/geo.hh \
        $(top_srcdir)/src/replicator/threads.hh

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
include $(srcdir)/dist/redhat/rpm.am
include $(srcdir)/dist/debian/deb.am

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
BUILT_SOURCES = 
#revno.h .configline
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
	  touch .lastmod;
	fi

.configline: revno.h
	-@rm -f .configline
	@head config.log | grep " .*/configure " | sed -e 's:^	. .*configure ::' > .configline

check: all
	cd src/testsuite; \
	$(MAKE) check $(RUNTESTFLAGS)
