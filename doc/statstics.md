# Statistics Collection

A large focus of Underpass's design is to support statistics
collection from the OSM [change data](changefile.md). Currently this has
been focused on producing statistics for the [Missing
Maps](https://www.missingmaps.org/)
[Leaderboard](http://www.missingmaps.org/leaderboards/#/missingmaps) 
and the [Red Cross](https://www.redcross.org/)
[Osm Stats](https://www.missingmaps.org/osmstats/) websites.

These two sites only track organized mapathons that have
[hashtags](http://www.missingmaps.org/partners/assets/docs/Guide%20for%20using%20hashtags%20and%20the%20leaderboard%20(MSFT).pdf). The
statsistics these sites are interested in is mapping progress for the
campaign. This is primarily concerned with tracking how many
buildings, highways, POIs, and waterways were either added or
modified. The totals are listed spreadsheet style.

## OSM Stats

The Red Cross maintains statistics of changes, so it's possible to 
track the mapping progress of individual users and
campaigns. Currently this data is acquired using 
[Overpass](https://osm.gs.mil/features/overpass) to generate 
an [augmented
diff](https://wiki.openstreetmap.org/wiki/Overpass_API/Augmented_Diffs),
which is then parsed by javascript code in the web frontend. The
Underpass writes directly to the *osmstats* database, and avoids the
overhead of both Overpass and parsing the diff file. It contains a
number of tables, some of which are documented here: 

Table | Description |
--------|------------ |
raw_changesets | Contains all the data of the changeset
raw_changesets_countries | Contains the changeset ID and an index into the raw_changesets table
raw_changesets_hashtags | Contains the changeset ID and the an index into the raw_hashtag table
raw_countries | Contains an index number, and the country or state name plus a display abbreviation
raw_hashtags | Contains an index number and hashtag used
raw_users | Contains only the user ID and name

The main table is *raw_changesets*, which contains calculated
statistics from the change data. This table gets populated with data
from both change files. Because Underpass is multi-threaded, different
columns are written at different times. Once complete, the
*updated_at* field is set with the current timestamp.

Table | Description |
--------|------------ |
id| This is the ID of the changeset, and comes from the changeset file
road_km_added| This value is updated by counting the length of roads added or deleted by the user in the change file
road_km_modified| This value is updated by counting the length of
existing roads modified by the user in the change file
waterway_km_added| This value is updated by counting the length of waterways added or deleted by the user in the change file
waterway_km_modified| This value is updated by counting the length of existing waterways modified by the user in the change file
roads_added| This value is updated by counting the roads added or deleted by the user in the change file
roads_modified| This value is updated by counting the existing roads modified by the user in the change file
waterways_added| This value is updated by counting the roads added or deleted by the user in the change file
waterways_modified| This value is updated by counting the existing waterways modified by the user in the change file
buildings_added| This value is updated by counting the buildings added by the user in the change file
buildings_modified| This value is updated by counting the existing buildings modified by the user in the change file
pois_added| This value is updated by counting the POIs added by the user in the change file
pois_modified| This value is updated by counting the existing POIs modified by the user in the change file
editor| The editor used, and comes from the changeset
user_id| The user ID, comes from the changeset
created_at| The timestamp this changeset was created, comes from the changeset
closed_at| The timestamp this changeset was closed, comes from the changeset
verified| Whether this data has been validated
updated_at| The timestamp this change was applied to the database


## Data Available For Statistics

This is a list of the available raw data sources for generating
statistics. These can be filtered by a timestamp or range, or limited
to a specific area using a polygon.


## ChangeSet File

This file from the OSM is available with minute, hourly, or daily
updates, called replication files. It documents the change itself, and
not the data of the change. This is mostly used for statistics
collection as it contains the hashtags and/or comments made when the
change was uploaded. 

Keyword | Description |
--------|------------ |
Created_at | When this changeset was created
Closed_at | When this changeset is completed, normally a few minutes
User ID | User ID in OSM of the mapper
Username | Username in OSM of the mapper
Change location | Bounding Box coordinates of this change
Various OSM tags | Existing metadata tags used for OSM


## OsmChange File

This file documents the data that got changed, and handles new data,
modified data, and deleted data. This includes the OSM tags that
changed. This file from the OSM is available with minute, hourly, or
daily updates.

Keyword | Description |
--------|------------ |
Version of the change | Incremented on each upload
Timestamp of the change | When this change was made
Location of the change | GPS coordinates of this change
Change tags | Any changed tags for an OSM element

### What is in an OSM Element ?

An OSM element is the actual map data . Because of the large variety of metadata tags, many statistics can be generated by analyzing the tags.

Keyword | Description |
--------|------------ |
The Object ID | The ID of the OSM object
The User ID | The OSM user ID who edited this object
The Username | The OSM username who edited this object
The type | Node, way, or relation
The location | GPS coordinates for a node
Nodes | List of node IDs for a way
Members | List of member objects IDs in a relation
Version of the change | Incremented on each upload
Timestamp | When this change was made
Various OSM tags | Existing metadata tags used for OSM

### OSM Tags

Currently use building, waterway, and highway tags
	
