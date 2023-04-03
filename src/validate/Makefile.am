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
#     Foobar is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.

# AUTOMAKE_OPTIONS =  -Wno-portability dejagnu subdir-objects

lib_LTLIBRARIES = libhotosm.la

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

libhotosm_la_SOURCES = \
	hotosm.cc hotosm.hh \
	validate.hh

libhotosm_la_LDFLAGS = -module -avoid-version

AM_CXXFLAGS = \
	-rdynamic \
	-Wno-deprecated-declarations

AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-DPKGLIBDIR=\"$(pkglibdir)\" \
	-DSRCDIR=\"$(srcdir)\" \
	-DBOOST_LOCALE_HIDE_AUTO_PTR

install-data-hook:
	$(MKDIR_P) $(DESTDIR)/$(pkglibdir)
	cp -uvp .libs/libhotosm.so $(DESTDIR)/$(pkglibdir)
	cp -uvp $(top_srcdir)/config/validate/*.yaml $(DESTDIR)/$(pkglibdir)
