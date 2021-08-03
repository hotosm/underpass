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
#

AUTOMAKE_OPTIONS = dejagnu

AM_CPPFLAGS = -I$(top_srcdir) \
	-I$(top_srcdir)/hottm \
	-I$(top_srcdir)/osmstats

check_PROGRAMS =  \
	stats-test \
	change-test \
	val-test \
	rep-test \
	under-test
#	hotosm-test
#	tm-test
#	geo-test 

BOOST_LIBS = \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_LOG_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_IOSTREAMS_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_LOCALE_LIB)

AM_CXXFLAGS = \
        -DPKGLIBDIR=\"$(pkglibdir)\" \
        -DTOPSRCDIR=\"$(top_srcdir)\" \
        -DSRCDIR=\"$(srcdir)\"

# _test_SOURCES = geo-test.cc
# geo_test_LDFLAGS = -L../..
# geo_test_CPPFLAGS = -DDATADIR=\"$(top_srcdir)\" -I$(top_srcdir)
# geo_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

val_test_SOURCES = val-test.cc
val_test_LDFLAGS = -L../..
val_test_CPPFLAGS = -DDATADIR=\"$(top_srcdir)\" -I$(top_srcdir)
val_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

rep_test_SOURCES = rep-test.cc
rep_test_LDFLAGS = -L../..
rep_test_CPPFLAGS = -DDATADIR=\"$(top_srcdir)\" -I$(top_srcdir)
rep_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

under_test_SOURCES = under-test.cc
under_test_LDFLAGS = -L../..
under_test_CPPFLAGS = -DDATADIR=\"$(top_srcdir)\" -I$(top_srcdir)
under_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the OSM DB classes
hotosm_test_SOURCES = hotosm-test.cc
hotosm_test_LDFLAGS = -L../..
hotosm_test_LDADD = -lpqxx -lunderpass  $(BOOST_LIBS)

# Test the TM DB classes
tm_test_SOURCES = tm-test.cc
tm_test_LDFLAGS = -L../..
tm_test_LDADD = -lpqxx -lunderpass  $(BOOST_LIBS)

# Test the OSM Stats class
stats_test_SOURCES = stats-test.cc
stats_test_LDFLAGS = -L../..
stats_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

# Test the Changeset class
change_test_SOURCES = change-test.cc
change_test_CPPFLAGS = -DDATADIR=\"$(srcdir)\" -I$(top_srcdir)
change_test_LDFLAGS = -L../..
change_test_LDADD = -lpqxx -lunderpass $(BOOST_LIBS)

RUNTESTFLAGS = -xml
