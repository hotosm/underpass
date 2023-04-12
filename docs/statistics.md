# Statistics Calculations

Underpass process two input streams for data, but only one is used for
the initial statistics calculations. [ChangeSet](changefile.md) are
used to populate some of the columns in the database, but aren't used
during statistics calculations by the backend. Only the
[OsmChange](changefile.md) file is used, as it contains the the data
that is changed.

The OsmChange data file contains 3 categories of data, what was
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

## Priority boundary

Currently, changes to be proccessed are filtered by [ChangeSetFile::areaFilter()](https://hotosm.github.io/underpass/classchangeset_1_1ChangeSetFile.html)
and [OsmChangeFile::areaFilter()](https://hotosm.github.io/underpass/classosmchange_1_1OsmChangeFile.html), using a boundary polygon.
In some cases is not possible to say if a OsmChange is inside
the priority boundary. To disable the filtering in the replication
process, you can add the argument `--osmnoboundary` for OsmChanges
or `--oscnoboundary` for Changesets.

## What Is Collected

The original statistics counted buildings, waterways, and POIs. The
new statistics break this down into two categories, accumulates
statistics for things like *buildings*, as well as the more detailed
representation, like what type of building it is. The list of values
is configurable, as it uses a [YAML](https://github.com/hotosm/underpass/blob/master/config/stats/statistics.yaml) file.


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

Each category of data has the total accumulated value, as well as the
more detailed breakdown. For example, it's possible to extract
statistics for only hospitals, which is a subset of all the
buildings. As keywords and values can be in different feature
categories, some are checked for in multiple ways. Some keywords and
values may be spelled differently than the default, so variations are
also looked for to be complete.

To find all the common tags, several continents worth of data was
analyzed to find the most common patterns for the features we want to
collect statistics for. Mixed with inconsistent tagging schemes is
random capitalization, misspellings, and international spellings. The
attempt is made to catch all reasonable variations.
[TagInfo](https://taginfo.openstreetmap.org/) was used to find totals
of some variations, with weird tagging that was not very common gets
ignored to avoid performance impacts, and data bloat.

### Building Types

Most buildings added by remote tracing of satellite imagery lack any
metadata tags beyond *building=yes*. When local mappers import more
detailed data, or update the existing metadata, those values get
added. This is a common set of building values.

- yes
- house
- residential
- commercial
- retail
- commercial;residential
- apartments
- kitchen
- roof
- construction
- school
- clinic
- hospital
- office
- public
- church
- mosque
- temple
- service
- warehouse
- industrial
- kiosk
- abandoned
- cabin
- bungalow
- hotel
- farm
- hut
- train_station
- house_boat
- barn
- historic
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

Places contain multiple features, and are mostly used to determine
local metadata improvements. 

- village
- hamlet
- neighborhood
- city
- town

### Highway Types

Highways traced from satellite imagery often lack metadata beyond the
functional type. For example, a highway that connects two villages is
easy to determine. An accumulated value for the total of highways, and
the total length in kilometers is store as an aggregate. More detailed
statistics are also kept allowing more detail when needed.

- trunk
- tertiary
- secondary
- unclassified
- track
- residential
- path
- service
- bridge

### School Types

When *school* is a keyword, there are several values for the type of
school. An aggregate total of schools can be calculated, as well as detail
for the type of school.

- primary
- secondary
- kindergarten

## Calculation Data flow

The data is processed by
[Underpass](https://github.com/hotosm/underpass). Underpass downloads
the changeset and the OsmChange files from the OpenStreetMap planet
server every minute. Downloaded files are also cached on disk, so it's
also possible to process data without a network connection. Once the
data is parsed from the respective data formats, it gets passed to the
[OsmChangeFile::collectStat()](https://hotosm.github.io/underpass/classosmchange_1_1OsmChangeFile.html#a4a6035b16ec815be6e0289b65bcbaaad)
method. That method loops through the data structure containing the
changes to the map data. Within that method, it calls
[ChangeSetFile::scanTags()](https://hotosm.github.io/underpass/classosmchange_1_1OsmChangeFile.html),
    which does all the real work. The *scanTags()* method uses [StatsConfigSearch::search()](https://hotosm.github.io/underpass/classstatsconfig_1_1StatsConfigSearch.html) to
    search the lists of keywords and values configured at the stats configuration file.
ScanTags() returns an array of statistics for the desired features.
That array is then converted by collectStats() into the [statistics data
structure](https://hotosm.github.io/underpass/classosmchange_1_1ChangeStats.html),
and control returns to the processing thread.

The processing thread then passes the statistics data to
[osmstats::applyChange()](https://hotosm.github.io/underpass/classosmstats_1_1QueryOSMStats.html#aa0aeffb3bb77e4891553ca1883f11a10),
to insert them into the database.

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
changesets WHERE 'hotosm-project-1234' = ANY(hashtags) AND user_id=4321;

The source is the satellite imagery used for remote mapping.

&nbsp;

Keyword | Description
--------|------------
id | The ID of this changeset
editor | The editor used for this changeset
user_id | The OSM User ID of the mapper
created_at | The timestamp when this changes was uploaded
closed_at | The timestamp when this uploaded change completed processing
updated_at | The timestamp when this last had data updated
added | An hstore array of the added map features
modified | An hstore array of the modified map features
deleted | An hstore array of the deleted map features
hashtags | An array of the hashtags used for this changeset
source | The imagery source used for this changeset
bbox | The bounding box of this changeset

