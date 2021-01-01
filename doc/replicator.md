# Replicator

Replicator is a command line utility for updating databases from
changesets and change files, and runs as a system daemon. Both change
files and changesets need to be applied to the database to stay in
sync. Change data can be downloaded from planet.openstreetmap.org
every minute, and has to be applied to the databases. Data is
processed as a stream, so a large disk for temporary files or much
memory isn’t needed.

The primary mode is to monitor the upstream change data for new files,
and to apply them. If the timestamp specifed is in the past, the
change data files are downloaded continuously until caught up with the
current time.

Each data source uses a different thread for multi-core support and
better performance. As files are downloaded from the different
sources, they are analyzed, and applied to the appropriate
database. Most of the work is done by the Underpass
library. Replicator is the utility program that a uses the library. 

