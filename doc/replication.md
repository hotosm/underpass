# Replication

Replication is the process of appyling a set changes to data. There
are two types of replication files available for OpenStreetMap data,
[changesets and changefiles](changefile.md) from their
[planet](https://planet.openstreetmap.org/replication) server. The
level directory contains several subdirectories. All the
subdirectory structures are the same.

Each of the directories contains a 3 digit name. A new
directory is created whenever there are 1 million data files
created. Each of these top level's subdirectories contains 1000
subdirectories, also named with a 3 digit value. Under those
subdirectories, there are 1000 entries. The time interval between map
updates is not consistent, so a *state.txt* file is used. This file
contains the timestamp of the accompanying data file. The data file is
compressed, and contains all the changed data.

For the change files, the directory structure looks like this:

	minute/001/001/001/001.state.txt
	minute/001/001/001/001.osc.gz.
	minute/001/001/001/002.state.txt
	minute/001/001/001/002.osc.gz.
	etc...

Changesets are slightly different, as they contain no map data, just
data on the changes made when uploaded. 

	changesets/001/001/001.osm.gz
	changesets/001/001/001.state.txt
	etc...
