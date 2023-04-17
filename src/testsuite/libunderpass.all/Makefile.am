#
# Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
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
	pq-test \
	change-test \
	yaml-test \
	statsconfig-test \
	planetreplicator-test \
	geo-test \
	areafilter-test \
	hashtags-test \
	stats-test \
	val-test \
	val-unsquared-test \
	test-playground

TOPSRC := $(shell cd $(top_srcdir) && pwd)/src
AM_CPPFLAGS = -I$(TOPSRC) -DDATADIR=\"$(TOPSRC)\"
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
val_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
val_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS) ../../validate/hotosm.lo

val_unsquared_test_SOURCES = val-unsquared-test.cc
val_unsquared_test_LDFLAGS = -L../..
val_unsquared_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
val_unsquared_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS) ../../validate/hotosm.lo

# Test the replication classes
#replication_test_SOURCES = replication-test.cc
#replication_test_LDFLAGS = -L../..
#rplication_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
#replication_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the OSM DB classes
#hotosm_test_SOURCES = hotosm-test.cc
#hotosm_test_LDFLAGS = -L../..
#hotosm_test_LDADD = -lpqxx -lunderpass  $(BOOST_LIBS)

# Test the OSM Stats class
stats_test_SOURCES = stats-test.cc
stats_test_LDFLAGS = -L../..
stats_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
stats_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the OSM StatsConfig
statsconfig_test_SOURCES = statsconfig-test.cc
statsconfig_test_LDFLAGS = -L../..
statsconfig_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the PlanetReplicator class
planetreplicator_test_SOURCES = planetreplicator-test.cc
planetreplicator_test_LDFLAGS = -L../..
planetreplicator_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the Changeset class
change_test_SOURCES = change-test.cc
change_test_CPPFLAGS = -DDATADIR=\"$(TOPSRC)\" -I$(TOPSRC)
change_test_LDFLAGS = -L../..
change_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test areaFilter
areafilter_test_SOURCES = areafilter-test.cc
areafilter_test_LDFLAGS = -L../..
areafilter_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Hashtags test
hashtags_test_SOURCES = hashtags-test.cc
hashtags_test_LDFLAGS = -L../..
hashtags_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test playground
test_playground_SOURCES = test-playground.cc
test_playground_LDFLAGS = -L../..
test_playground_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)


CLEANFILES = \
	underpass.log \
	pq-test.log \
	change-test.log \
	geo-test.log \
	stats-test.log \
	val-test.log \
	val-unsquared-test \
    statsconfig-test.log \
	planetreplicator-test.log \
	areafilter-test.log \
	hashtags-test.log \
	replication-test.log

RUNTESTFLAGS = -xml
