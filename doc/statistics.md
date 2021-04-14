# Statistics Database

The new statistics database schema is very different from the current
one used for OSM Stats.

## geoboundaries table

Keyword | Description
--------|------------
cid | An internal ID for this administrative boundary
name | The name of the boundary
admin_level | The administrative level of this boundary
tags | Other metadata related to this boundary
boundary | The multipolygon of this boundary

## changesets table

Keyword | Description
--------|------------
id | 
editor | 
user_id | 
created_at | 
closed_at | 
updated_at | 
added | 
modified | 
deleted | 
hashtags |
source | 
validated | 
bbox | 

## users table

Keyword | Description
--------|------------
id | The OSM ID of this mapper
name | The OSM user name of the mapper
tm_registration | The timestamp of when the mapper registered with the tasking manager
osm_registration | The timestamp of when the mapper registered with openstreetmap
gender | The mappers gender, when available
home | The mappers home location, when available

## ground data table

Keyword | Description
--------|------------
starttime | The timestamp of when the user opened the app and started data collection
endtime | The timestamp for when the data collection was completed
username | The username of the mapper, not the same as an OSM username
changeset_id | Changeset ID applied when the data is uploaded to OSM
location | The coordinates of this POI
