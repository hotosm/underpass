# Underpass

# Overview

Underpass is a C++ API and utility programs for mainipulating
OpenStreetMap data at the database and raw data file level. It can
download replication file from the OSM planet server and use these
files to update a local copy of the OSM database, or analysze the
changes to generate statistics.

Currently this is usually done using Overpass, which can be
self-hosted or accessed remotely. Overpass has major performance
issues though, namely it’s single threaded, and doesn’t use a real
database. What Overpass does is support a wide range of querying OSM
data, and can handle rather complicated filters. Much of that
functionality is not needed for supporting statistics collection.

Underpass is designed to function as a replacement for a subset of
Overpass’s functionality. It is focused on producing the statistics
for the Missing Maps Leaderboard, and the Red Cross OSM Stats in a
timely fashion. 

Rather than using disk based files, it has a self-hosted database
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
* Uses [GDAL](https://www.gdal.org) for reading Geospatial files)

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
change information, and the actual OSM data. 


# Replicator

Replicator is a command line utility for updating a database from
changeset and change files, suitable for being run as a cron job or
daemon. Both change files and changesets need to be updated to stay in
sync. Updated data can be downloaded from the OSM
[planet](https://planet.openstreetmap.org), server, and has to be
applied to the database. Data is processed as a stream, so a large
disk for temporary files or much memory isn’t needed.

Remote directories on planet are named with a 3 digit number. The top
level directory covers many years in only a few directories. Each
subdirectory is also named with 3 digits, and covers several months. 

Under each following subdirectory, are more 3 digit
subdirectories. Each one of these contains the actual data. The data
comprises of files created with a minute interval. Each minute update
consists of two files. One is the state file, which has the starting
timestamp of the data file. The data file has the changes. To update
the data, each file needs to be applied in sequence. Both the state
file and the data file also start with a 3 digit number. Once there
are 1000 entries in the subdirectory containing the data files, a new
directory is created for the next 1000 entries. 

Currently there is no utility to import Changeset files, which contain
hashtags and comments, into a database. The replicator utility parses
the changeset information and imports it into the database. The data
fields in the changeset file that get imported are the changeset ID,
the User ID, the hashtag or comment, and the creation and closed
timestamp. 
