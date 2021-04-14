# Statistics Database

The new statistics database schema is very different from the current
one used for [OSM Stats](osmstats.md). The current database schema is
more oriented towards supporting the display functions, and is not
extensable beyond the few features it collects data about.

The new schema is designed to be more flexible to track more types of
features, as well as to be more flexible on the types of queries it
can suppport. By adding spatial data, it can filter data extracts by
polygon. The current implementaion is limited to using country
boundaries.

## geoboundaries table

This a static table, the data is maintained outside of this
project. It can be produced by extracting administrative boundaries
from OpenStreetMap data to updfate if changes are made to the
boundaries. This table is not used by backend, it's primarily to
assist the frontend to create a list of existing polygons that can be
used by the frontend to filter data. Rather than the existing
raw_countries table in the osmstats schema which was English only,
this one support internationalized names, and both the official 2 and
3 letter ISO abbreviations.

Currently this table contains the data for most countries, and US
states. Other boundaries could also be added, and using admin_level to
differentiate them from the existing ones. For example, this could
also be populated by Tasking Manager project boundaries.

Keyword | Description
--------|------------
cid | An internal ID for this administrative boundary
name | The name of the boundary
admin_level | The administrative level of this boundary
tags | Other metadata related to this boundary
boundary | The multipolygon of this boundary

## changesets table

This is the primary table used to contain the data for each
changeset. All of the data is stored in a table for better query
performance on large data sets. It also limits needing SQL [sub
queries](https://www.postgresql.org/docs/13/functions-subquery.html) or a
[JOIN](https://www.postgresql.org/docs/13/tutorial-join.html) between
tables, reducing complexity.

An [hstore](https://www.postgresql.org/docs/13/hstore.html) is used to
store the statistics instead of having a separate table for each
feature to allow for more flexibility. An hstore is a key & value
pair, and multiple data items can be stored in a single column,
indexed by the key. This allows for more features to be added by the
backend and the frontend, without having modify the database schema.

An example query to count the total number of buildings added by the
user **4321** for a Tasking Manager project **1234** would be this:

> SELECT SUM(CAST(added::hstore->'building' AS DOUBLE precision)) FROM
changesets WHERE 'hotosm-project1234' = ANY(hashtags) AND user_id=4321;

&nbsp;

Keyword | Description
--------|------------
id | The ID of this changeset
editor | The editor used for this changeset
user_id | The OSM User ID of the mapper
created_at | The timestamp 
closed_at | The timestamp 
updated_at | The timestamp 
added | An array of the added map features
modified | An array of the modified map features
deleted | An array of the deleted map features
hashtags | An array of the hashtags used for this changeset
source | The imagery source used for this changeset
validated | Where this changeset has been validated in the tasking manager
bbox | The bounding box of this changeset

## users table

Tnis table contains data on mappers, and is used to track indivigual
activity. Often badges are given based on this data.

Keyword | Description
--------|------------
id | The OSM ID of this mapper
name | The OSM user name of the mapper
tm_registration | The timestamp of when the mapper registered with the tasking manager
osm_registration | The timestamp of when the mapper registered with openstreetmap
gender | The mappers gender, when available
home | The mappers home location, when available

## ground data table

This table collects statistics on ground mapping campaigns, which
contains different metadata that gets lost when the data is edited for
uploading to OpenStreetMap. This table is currently a placeholder for
future developement at this time, so unused for now,

Keyword | Description
--------|------------
starttime | The timestamp of when the user opened the app and started data collection
endtime | The timestamp for when the data collection was completed
username | The username of the mapper, not the same as an OSM username
changeset_id | Changeset ID applied when the data is uploaded to OSM
location | The coordinates of this POI
