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

AM_CPPFLAGS = -I$(top_srcdir) \
	-I$(top_srcdir)/hottm \
	-I$(top_srcdir)/osmstats

check_PROGRAMS =  \
	hotosm-test \
	tm-test \
	stats-test \
	change-test \
	geo-test \
	val-test

BOOST_LIBS = $(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_LOG_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_IOSTREAMS_LIB) $(BOOST_THREAD_LIB)

geo_test_SOURCES = geo-test.cc
geo_test_LDFLAGS = -L../..
geo_test_CPPFLAGS = -DDATADIR=\"$(top_srcdir)\" -I$(top_srcdir)
geo_test_LDADD = -lpqxx -lhotosm $(BOOST_LIBS)

val_test_SOURCES = val-test.cc
val_test_LDFLAGS = -L../..
val_test_CPPFLAGS = -DDATADIR=\"$(top_srcdir)\" -I$(top_srcdir)
val_test_LDADD = -lpqxx -lhotosm $(BOOST_LIBS)

# Test the OSM DB classes
hotosm_test_SOURCES = hotosm-test.cc
hotosm_test_LDFLAGS = -L../..
hotosm_test_LDADD = -lpqxx -lhotosm  $(BOOST_LIBS)

# Test the TM DB classes
tm_test_SOURCES = tm-test.cc
tm_test_LDFLAGS = -L../..
tm_test_LDADD = -lpqxx -lhotosm  $(BOOST_LIBS)

# Test the OSM Stats class
stats_test_SOURCES = stats-test.cc
stats_test_LDFLAGS = -L../..
stats_test_LDADD = -lpqxx -lhotosm $(BOOST_LIBS)

# Test the Changeset class
change_test_SOURCES = change-test.cc
change_test_CPPFLAGS = -DDATADIR=\"$(srcdir)\" -I$(top_srcdir)
change_test_LDFLAGS = -L../..
change_test_LDADD = -lpqxx -lhotosm $(BOOST_LIBS)

RUNTESTFLAGS = -xml
