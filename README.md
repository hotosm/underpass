# Mainpage

Underpass is a C++ API and utility programs for mainipulating
OpenStreetMap data at the database and raw data file level. It can
download replication files from the OSM planet server and use these
files to update a local copy of the OSM database, or analysze the
changes to generate statistics. It is designed to be high
performance on modest hardware.

Currently this is usually done using Overpass, which can be
self-hosted or accessed remotely. Overpass has major performance
issues though, namely it’s single threaded, and doesn’t use a real
database. What Overpass does is support a wide range of querying OSM
data, and can handle rather complicated filters. Much of that
functionality is not needed for supporting statistics collection and
simple validation.

Underpass is designed to function as a replacement for a subset of
Overpass’s functionality. It is focused on analyzing the change data
every minute, and generating statistics or doing validation of the
metadata. This can be used for the Missing Maps Leaderboard, and the
Red Cross OSM Stats. It is designed to be able to process large files
by streaming the data.

Rather than using disk based tempfiles, it has a self-hosted database
centric design for better performance. It uses Postgresql with the
Postgis extension, both commonly used for core OSM infrastructure. One
big advantage of using postgres is it supports utilizing multi-core
processors for faster SQL queries. It can optionally use [GPU
support](https://heterodb.github.io/pg-strom/) for faster geospatial
queries. 

The other primary design goal of Underpass is to be maintainable for
the long term. It uses common open source infrastructure to make it
accessible to community developers. The primary dependencies, which
are used by multiple other core OSM projects are these:

* Uses the GNU [autotools](https://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html) for multi-platform support
* Uses [SWIG](http://www.swig.org/) for bindings to multiple languages
* Uses [Boost](https://www.boost.org/) for additional C++ libraries
* Uses [GDAL](https://www.gdal.org) for reading Geospatial files
* Uses [PQXX](http://www.pqxx.org/development/libpqxx/) for accessing Postgres
* Uses [Libxml++](http://libxmlplusplus.sourceforge.net/) For parsing XML files

Ideally Underpass can be used by other projects needing to do similar
tasks without Overpass. It should be able to support collecting more
statistics than are currently used. Since much of Underpass deals with
the low level data flow of OpenStreetMap, long-term it can be used to
support future projects as a common infrastructure for database
oriented mapping tasks.

A critical future project is conflation and validation of existing
data and mapathons. Underpass will support the database management
tasks those projects will need. The same database can also be used for
data exports, and Underpass can be used to keep that data up to
date. What the Overpass data store contains is history information,
change information, and the actual OSM data. Underpass can access this
same information by processing the change data itself.

# Dependencies

Underpass requires a modern C++ compiler that supports the 2011 C++
standard. The 2011 standard simplified the syntax by adding the *auto*
keyword, and thread support became part of the standard C++
library. Underpass is a heavy user of of the *boost* libraries, and
also requires reasonably up to date C++ libraries for other
dependencies. Underpass also uses the ranges-v3 library. This will be
in the 2020 C++ standard, but hasn't been released yet.

Fedora 33, Debian Buster, and Ubuntu Groovy contain a package for
libranges-v3, which is the code base for what will be in
C++ 2020. Both Debian Buster and Ubuntu Groovy ship *libpqxx 6.x*,
which has a bug which has been fixed in *libpqxx 7.x*. Fedora 33 ships
the newer version.

# More Information

If you want to know more about Underpass, the [project
Documentation](https://robsavoye.github.io/underpass/pages.html) is
here. This includes the internal documentation of all the classes. The
source code is [available here](https://github.com/robsavoye/underpass).
