# Replication Files

There are two types of data files related to changes in the map
data. These files contain all the changes made during a time interval,
every minute, hour, or daily data files are available from the
OpenStreetMap [planet](https://planet.openstreetmap.org/replication/)
server. 

There are two different formats of change data, one for changesets,
and the other for the actual changes. Both are needed for filtering
changes to produce statistics.

## Changeset

A changeset file contains only the data about the change, and not the
actual change itself. It contains the data of the change at the time
it's uploaded to OpenStreetMap. Each change has an action; create,
delete, modify. Each action then contains the changed objects in the
action. As this file contains the data created when uploading it to
OpenStreetMap, it's the only way to access the commit hashtags or
comments. 

Hashtags didn’t exist until late 2014, so between 2014 and 2017,
hashtags were contained in the comment field. In 2017, the official
hashtag tag was added. The first mentions of the hashtags
_#missingmaps_ and _#hotosm_ starts 2017-10-20. A typical changefile
entry looks like this:

```xml
	<changeset id="12345" created_at="2014-10-10T01:57:09Z" closed_at="2014-10-10T01:57:23Z" open="false" user="foo" uid="54321" min_lat="-2.8042325" min_lon="29.5842812" max_lat="-2.7699398" max_lon="29.6012844" num_changes="569" comments_count="0">
	    <tag k="source" v="Bing"/>
	    <tag k="comment" v="#hotosm-task-001 #redcross #missingmaps"/>
	    <tag k="created_by" v="JOSM/1.5 (7182 en)"/>
	</changeset>
```

The **created_by** and **source** fields can be used to generate
statistics of editor choices and imagery sources. Underpass uses only
the comment and hashtag fields currently as a way to group sets of
changes by organization, or mapping campaign.

## OsmChange

And OsmChange file contains the data of the actual change. It uses the
same syntax as an OSM data file plus the addition of one of the three
actions. Nodes, ways, and relations can be created, deleted, or
modified. Multiple OSM object types can be in the same action. As this
data contains the actual change, it is used to filter by tag, or used
to do calculations, like the length of roads added.

```xml
	<modify>
		<node id="12345" version="7" timestamp="2020-10-30T20:40:38Z" uid="111111" user="foo" changeset="93310152" lat="50.9176152" lon="-1.3751891"/>
	</modify>
	<delete>
		<node id="23456" version="7" timestamp="2020-10-30T20:40:38Z" uid="22222" user="foo" changeset="93310152" lat="50.9176152" lon="-1.3751891"/>
	</delete> 
	<create> 
		<node id="34567" version="1" timestamp="2020-10-30T20:15:24Z" uid="3333333" user="bar" changeset="93309184" lat="45.4303763" lon="10.9837526"/>\n 
	</create> 
```

## State Files

There are two types of state files, one for changesets and the other
for change files. The changeset one is the simplest, only containing
the last_run timestamp and a changeset sequence number. The first
state file starts *2016-09-07 10:45*. An example changeset state file
is:

	\---
	last_run: 2020-10-08 16:41:01.863533000 +00:00
	sequence: 4139643

The state file for change files contains more fields, but the only
ones needed for database updates are the timestamp of the data file,
and the sequence number. An example change file state file is: 

	#Thu Oct 08 16:38:04 UTC 2020
	sequenceNumber=4229951
	txnMaxQueried=3081409719
	txnActiveList=
	txnReadyList=
	txnMax=3081409719
	timestamp=2020-10-08T16\:38\:02Z

As the 3 digit prefix in the state filename matches the data file,
this is used to get the right data that matches that state. Since
the replication files are fetched via HTTP/HTTPS, there is no
timestamp available. To find a specific replication file, a state file
close to the timestamp is downloaded. Then the timestamp in the state
file is checked, and if it matches, the the 3 digit part of the state
file name can be used to find the appropriate replication data file.

# Finding The Right Data File

It is desirable to be able to start processing changes starting with a
specific timestamp. This enables the end user to update databases
after a period of downtime. With minute updates, there are many
thousands of files. Since it takes about 10 milliseconds to download a
state.txt file from planet, scanning for files isn't really
practical. Once the proper data file is found, then it's easy to just
download the next data file in sequence.

Underpass has a table in it's database to store the timestamps, and
paths to the data files. That makes it possible to quickly find the
appropriate starting data file. 
