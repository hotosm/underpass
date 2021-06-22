# Statistics Calculations

Underpass process two input streams for data, but only one is used for
the initial statistics calculations. [ChangeSet](changefile.md) are
used to populate some of the columns in the database, but aren't used
during statistrics calculations by the backend. Only the
[OsmChange](changefile.md) file is used, as it contains the the data
that is changed.

The OsmChange data file contains 3 catagories of data, what was
created, modified, or deleted. Currently only changed and modified
data are used for statistics calculations.

All of the changed data is parsed into a [data
structure](https://hotosm.github.io/underpass/classosmchange_1_1OsmChange.html)
that can be passed between classes. This data structure contains all
the data as well as the associated action, create or modify. The
parsed data is then passed to the
[OsmChangeFile::collectStat()](https://hotosm.github.io/underpass/classosmchange_1_1OsmChangeFile.html#a4a6035b16ec815be6e0289b65bcbaaad)
method to do the calculations. While currently part of the core code,
in the future this will be a plugin, allowing others to create
different statistics calculations without modifying the code.

## What Is Collected

The original statistics counted buildings, waterways, and POIs. The
new statistics break this down into two catagories, assumulates
statistics for things like *buildings*, as well as the more detailed
representation, like what type of building it is. The list of values
for buildings is configurable. Currently it's a simple list, but in
the future will be populated by a config file in
[YAML](https://yaml.org/) format.

OpenStreetMap features support a *keyword* and *value* pair. The
keywords are [loosely defined](https://taginfo.openstreetmap.org/),
and over time some have changed and been improved. Often new mappers
get confused, so may use keywords and values in an inconsistent
manner. Also over time the definitions of some tags has changed, or
been extended.

For example, let's look at schools. There is a variety of ways school
buildings are tagged. Sometimes it's *building=school*, with school as
the value. Sometimes school is the keyword, and the value is the type
of school. In this case, the type of school is accumulated, as well as
a generic *school* count. Also if *building=school* isn't used, then
every school also increments the count of buildings.

Each catagory of data has the total accumulated value, as well as the
more detailed breakdown. For example, it's possible to extract
statistics for only hospitals, which is a subset of all the
buildings. As keywords and values can be in different feature
categories, some are checked for in multiple ways. Some keywords and
values may be spelled differently than the default, so variations are
also looked for to be complete.

### Building Types

Most buildings added by remote tracing of satellite imagery lack any
metadata tags beyond *building=yes*. When local mappers import more
detailed data, or update the existing metadata, those values get
added. This is the current set of building values being accumulated.

- hospital
- school
- healthcare
- clinic
- health center
- health centre
- latrine
- latrines
- toilet
- toilets

### Amenity Types

Most amenities are added by local mappers or through a data
import. Not all amenities are buildings, but for our use case that's
all that is analyzed. These are the common values for the *amenity*
keyword.

- hospital
- school
- clinic
- kindergarten
- drinking_water
- health_facility
- health_center
- healthcare
	
### Places Types

- village
- hamlet
- neigborhood
- city
- town

### Highway Types

- highway
- tertiary
- secondary
- unclassified
- track
- residential
- path
- bridge
- waterway

### School Types

- primary
- secondary
- kindergarten
	
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

The source is the satellite imagery used for remote mapping.

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
validated | Where this changeset has been validated in the Tasking Manager
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
