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

Since the data file uses the same 3 digit prefix as the state file,
it's easy to get the right data for the timestamp. Because the size of
the data in the changefiles varies, plus CPU load, network latency,
etc... the time interval varies, so can't be calculated accurately. It
is possible to get a rough idea. Underpass makes the best guess it
can, and downloads a *state.txt* file that is close to the desired
timestamp. Using that initial state.txt file it's possible to
increment or decrement the prefix till the proper timestamp is
found. Then the prefix is used to download the data file for
processing.
