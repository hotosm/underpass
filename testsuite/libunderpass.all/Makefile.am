#
# Copyright (c) 2020, 2021, 2022 Humanitarian OpenStreetMap Team
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
#

AUTOMAKE_OPTIONS = dejagnu

check_PROGRAMS = \
	conflate-test \
	tm-test \
	pq-test \
	change-test \
	val-test \
	osm2pgsql-test \
	yaml-test \
	stats-test \
	planetreplicator-test \
	geo-test

TOPSRC := $(shell cd $(top_srcdir) && pwd)
AM_CPPFLAGS = -I$(TOPSRC) -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)/hottm
AM_LDFLAGS = -L../..

BOOST_LIBS = \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LOG_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_IOSTREAMS_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_LOCALE_LIB) \
	$(BOOST_TIMER_LIB) \
	$(BOOST_PYTHON_LIB)

AM_CXXFLAGS = \
	-fPIC \
        -DPKGLIBDIR=\"$(pkglibdir)\" \
        -DTOPSRCDIR=\"$(TOPSRC)\" \
        -DSRCDIR=\"$(srcdir)\" \
	-DBOOST_LOCALE_HIDE_AUTO_PTR \
	-Wno-deprecated-declarations

conflate_test_SOURCES = conflate-test.cc
conflate_test_LDFLAGS = -L../..
conflate_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
conflate_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

yaml_test_SOURCES = yaml-test.cc
yaml_test_LDFLAGS = -L../..
yaml_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
yaml_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

pq_test_SOURCES = pq-test.cc
pq_test_LDFLAGS = -L../..
pq_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
pq_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

geo_test_SOURCES = geo-test.cc
geo_test_LDFLAGS = -L../..
geo_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
geo_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

val_test_SOURCES = val-test.cc
val_test_LDFLAGS = -L../..
val_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS) ../../validate/hotosm.lo
val_test_CPPFLAGS = -I$(TOPSRC) -DDATADIR=\"$(TOPSRC)\"

# Test the replication classes
#replication_test_SOURCES = replication-test.cc
#replication_test_LDFLAGS = -L../..
#rplication_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
#replication_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the OSM DB classes
#hotosm_test_SOURCES = hotosm-test.cc
#hotosm_test_LDFLAGS = -L../..
#hotosm_test_LDADD = -lpqxx -lunderpass  $(BOOST_LIBS)

# Test the TM DB classes
tm_test_SOURCES = tm-test.cc
tm_test_LDFLAGS = -L../..
tm_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
tm_test_LDADD = -lpqxx -lunderpass  $(BOOST_LIBS)

# Test the OSM Stats class
stats_test_SOURCES = stats-test.cc
stats_test_LDFLAGS = -L../..
stats_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the PlanetReplicator class
planetreplicator_test_SOURCES = planetreplicator-test.cc
planetreplicator_test_LDFLAGS = -L../..
planetreplicator_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the Changeset class
change_test_SOURCES = change-test.cc
change_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
change_test_LDFLAGS = -L../..
change_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the usersync class
usersync_test_SOURCES = usersync-test.cc
usersync_test_CPPFLAGS = -DDATADIR=\"$(srcdir)\" -I$(TOPSRC)
usersync_test_LDFLAGS = -L../..
usersync_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test osm2pgsql class
osm2pgsql_test_SOURCES = osm2pgsql-test.cc
osm2pgsql_test_CPPFLAGS = -DDATADIR=\"$(srcdir)\" -I$(TOPSRC)
osm2pgsql_test_LDFLAGS = -L../..
osm2pgsql_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)


CLEANFILES = \
	underpass.log \
	tm-test.log \
	usersync-test.log \
	pq-test.log \
	change-test.log \
	geo-test.log \
	osm2pgsql-test.log \
	stats-test.log \
	planetreplicator-test.log \
	replication-test.log

RUNTESTFLAGS = -xml
