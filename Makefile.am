#
# Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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

AUTOMAKE_OPTIONS = dejagnu subdir-objects
ACLOCAL_AMFLAGS = -I m4

# All of these are generated files
CLEANFILES = \
	all-includes.h.gch \
	all-includes.h

DISTCLEANFILES = doc/html doc/latex .lastmod
MAINTAINERCLEANFILES = revno.h .lastmod

SUBDIRS = \
	validate
# 	po

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
	data/underpass.sql \
	data/pgsnapshot.sql \
	data/galaxy.sql \
	data/geoboundaries.sql

YAML_FILES = \
	validate/building.yaml \
	validate/highway.yaml \
	validate/place.yaml \
	validate/statistics.yaml \
	validate/wastepoints.yaml

# inst_HEADERS = hotosm.hh hottm.hh
libunderpass_la_SOURCES = \
	log.cc log.hh \
	dsodefs.hh gettext.h \
	hottm.cc hottm.hh \
	replicatorconfig.hh \
	hottm/tmdefs.hh \
	hottm/tmusers.cc hottm/tmusers.hh \
	hottm/tmteams.cc hottm/tmteams.hh \
	hottm/tmprojects.cc hottm/tmprojects.hh \
	galaxy/galaxy.cc galaxy/galaxy.hh \
	galaxy/changeset.cc galaxy/changeset.hh \
	galaxy/osmchange.cc galaxy/osmchange.hh \
	galaxy/replication.cc galaxy/replication.hh \
	data/geoutil.cc data/geoutil.hh \
	data/pgsnapshot.cc data/pgsnapshot.hh\
	data/import.cc data/import.hh \
	data/osmobjects.cc data/osmobjects.hh \
	data/osm2pgsql.cc data/osm2pgsql.hh \
	data/threads.cc data/threads.hh \
	data/underpass.cc data/underpass.hh \
	data/yaml.hh data/yaml.cc \
	data/pq.hh data/pq.cc \
	data/setupdb.sh \
	external/thread_pool.hpp

if JEMALLOC
libunderpass_la_LDFLAGS = -avoid-version -ljemalloc
else
libunderpass_la_LDFLAGS = -avoid-version
endif

bin_PROGRAMS = replicator
replicator_SOURCES = replicator.cc 
replicator_LDADD = libunderpass.la $(BOOST_LIBS)

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
#	-DTIMING_DEBUG

pkgdata_DATA = $(SQL_FILES) data/priority.geojson data/setupdb.sh

EXTRA_DIST = \
	$(SQL_FILES) \
	$(YAML_FILES) \
	testsuite \
	data/priority.geojson \
	po \
	doc \
	debian \
	redhat

DEJATOOL = libunderpass

if ENABLE_PYTHON
pkglib_LTLIBRARIES = underpass.la
underpass_la_SOURCES = data/python.cc $(libunderpass_la_SOURCES)
underpass_la_LDFADD = $(BOOST_LIBS) libunderpass.la validate/libhotosm.la -lboost_python310
underpass_la_LDFLAGS = -module -avoid-version -no-undefined -rpath /usr/lib64/ validate/libhotosm.la $(BOOST_LIBS)
PY_OBJECTS = $(libunderpass_la_OBJECTS:.lo=.o)
endif

if ENABLE_DOXYGEN
apidoc:
	(cd doc && $(DOXYGEN))
endif

install-data-hook:
	$(MKDIR_P) $(DESTDIR)/$(pkglibdir)
	cp -uvp $(srcdir)/data/priority.geojson $(DESTDIR)/$(pkglibdir)
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
        -I$(top_srcdir)/galaxy \
        -I$(top_srcdir)/data \
        -I$(top_srcdir)/hottm

PCHHEADERS = \
        $(top_srcdir)/timer.hh \
        $(top_srcdir)/hottm.hh \
        $(top_srcdir)/galaxy/galaxy.hh \
        $(top_srcdir)/galaxy/replication.hh \
        $(top_srcdir)/galaxy/osmchange.hh \
        $(top_srcdir)/galaxy/changeset.hh \
        $(top_srcdir)/data/import.hh \
        $(top_srcdir)/data/validate.hh \
        $(top_srcdir)/data/pgsnapshot.hh \
        $(top_srcdir)/data/osmobjects.hh \
        $(top_srcdir)/data/osm2pgsql.hh \
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

check: all
	cd testsuite; \
	$(MAKE) check $(RUNTESTFLAGS)
