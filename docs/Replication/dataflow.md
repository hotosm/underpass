# Underpass Data Flow

Underpass is a data analytics engine for
[OpenStreetMap](https://www.openstreetmap.org). OpenStreetMap makes
changes made to the map available every
minute. [Overpass](https://overpass-turbo.eu/) takes this input and
makes it publically available. Underpass reads the same data imports
as Overpass, but processes the data differently. Overpass is focused
on small data extracts for display purposes. It can also be used to
download raw OSM data. Currently Overpass stores all the data as a
directory of text files, so can't do data analysis.

Underpass uses a postgres database to store data, and also does data
quality checks while processing the data. Underpass produces two
postgres databases, one for raw OSM data, the other for the results of
data quality analysis.

## Replication Files

There are two sets of data called replication files, and they are
updated every minute. One just documents the change itself, the other
is the features with all the new or modified tags. The replication
files live on a *planet* server, which contains all the raw change
files. They are brpken down into three categories, daily, hourly, or
minutely. Underpass supports processing all three, and defaults to the
minutely updates. Minutely updates don't get added to the replication
server exactly every minute, since the processing time varies.

Underpass stores all the downloaded files on disk as an aid to
development of the data quality checks to save on the networking
time. If you start at **000/000/000**, you can make a mirror of the
planet server. Note that this can consume much disk space.

The directory structure looks like this:

	replication/
		changesets/
		day/
			000/
			...
		hour/
			000/
			...
		minute/
			000/...
			001/...
			002/...
			003/...
			004/...
			005/...
			006/142/
				001.osc.gz
				001.state.txt
				002.osc.gz
				002.state.txt
				...

There are two files for each update. The **[0-9].osc.gz**, and the
state file. The **osc.gz** file is the actual changes. Since the
timestamp on the file is unreliable, the **[0-9].state.txt** file
contains the actual timestamp and a sequence number. When downloading
replication files for a specific timestamp, you first download a few
state.txt files, check the timestamps, and when you find the timestamp
you want, extract the numerical part of the file name, and then use
that to download the correct osc.gz file.
			
	#Thu Jun 20 22:26:08 UTC 2024
	sequenceNumber=6142001
	timestamp=2024-06-20T22\:25\:43Z		

Since Underpass runs continuously, the starting file doesn't have to
be exact. Underpass can also be started with the actual filename
instead of a timestamp.

### Changesets

A *changeset* file documents the change itself, but does not contain
the changed data. It contains the hashtag and comments used when the
changes are uploaded to OSM. Each changeset has a unique ID. Pre 2017,
there was no *hashtag* tag, so hashtags for organized mapathon are in
the *comment* tag. Underpass looks for hashtags in both.

	<changeset id="152984997" created_at="2024-06-21T09:57:46Z" closed_at="2024-06-21T09:58:40Z" open="false" num_changes="1" user="foo" uid="43065" min_lat="35.0039682" max_lat="35.0039682" min_lon="135.7773062" max_lon="135.7773062" comments_count="0">
		<tag k="comment" v="Specify whether pedestrian crossings have traffic signals"/>
		<tag k="locale" v="de-DE"/>
		<tag k="source" v="survey"/>
		<tag k="hashtags" v="#hotosm-project-130;#Mapping_emergency_response"/>
	</changeset>
	
The hashtags are used to identify an organized mapping campaign or a
data import. These both generate multiple changes, so the hashtag is
used to identify them. Underpass also uses these when generating a
data quality reports for the project or import. The bounding box is
also used for data quality, since a large change over a wide area may
be an accidental addition or deletion, or worse yet,
vandalism. Underpass stores the data for each changeset in the
**changesets** table in the database.

### Osmchange

An *osmchange* contains the OSM data that has changed. All the data
for any changes a feature has is included, so it doesn't have to be
conflated with any tags in an existing OSM feature.

	<delete>
		<node id="1373079576" version="2" timestamp="2024-05-10T06:27:11Z" uid="106722" user="the_editor" changeset="151128574" lat="37.0474915" lon="22.1773664"/>
		<node id="1373080330" version="2" timestamp="2024-05-10T06:27:11Z" uid="106722" user="the_editor" changeset="151128574" lat="37.0474815" lon="22.1774858"/>
	</delete>
	<modify>
		<way id="1545" version="18" timestamp="2024-05-10T06:27:07Z" uid="13579" user="mmmm" changeset="123456">
			<nd ref="153531465"/>
			<nd ref="9571419466"/>
			<nd ref="2507347003"/>
			<nd ref="9571419464"/>
			<nd ref="153452519"/>
			<tag k="highway" v="residential"/>
			<tag k="lanes" v="1"/>
			<tag k="lit" v="sunset-sunrise"/>
			<tag k="name" v="Via Francesco Caracciolo"/>
			<tag k="name:etymology:wikidata" v="Q342869"/>
			<tag k="oneway" v="yes"/>
		</way>
	</modify>

This file has three keywords, *delete*, *create*, and *modify*. If the
keyword is modify, then version of the feature has been
incremented. In this example, the *way* has had two nodes deleted.

## Data Quality Checks

Underpass currently does some validation of tag values and building
geometry. This is the default HOT plugin for validation. The
validation code can be an external plugin, no need to modify the core
Underpass code. 

# Implementation

Underpass is written in C++ for the best performance, and is a heavy
user of the [Boost libraries](https://www.boost.org/). It is heavily
multi-threaded. Packets in the two incoming data streams arrive at
slightly different times. 
