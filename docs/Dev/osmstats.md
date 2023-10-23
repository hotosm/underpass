# OSM Stats

The Red Cross maintains statistics of changes, so it’s possible to
track the mapping progress of individual users, teams, and mapping
campaigns. Underpass writes directly to the postgres database used for
OSM Stats. Underpass has the ability to recreate this database and
populate it with data from the raw data files. Underpass uses
[replication change files](changefile.md) to update these tables. The
database schema contains a number of tables which are documented here:

&nbsp;
## OSM Stats Tables

The OSM schema contains a number of tables, which are designed to
support the web front end. the **raw_** tables are mostly static, and
are simply a matching of an ID to a name for display. There are other
smaller tables for the same purpose, usedc to group queries together
for the front-end.

Keyword | Description
--------|------------
Augmented_diff_status | Contains the changeset ID of the augmented diff, and the timestamp of the last update
badges | Contains the the badge ID, the category, the badge name, and the experience level
badges_users |  Contains the user ID, the badge category, the badge name, and the user experience level
changesets_status | Contains the changeset ID and the timestamp for the update
raw_changesets | Contains all the data of the changeset
raw_changesets_countries | Contains the changeset ID and an index into the raw_changesets table
raw_changesets_hashtags | Contains the changeset ID and the an index into the raw_hashtag table
raw_countries | Contains an index number, and the country or state name plus a display abbreviation
raw_hashtags | Contains an index number and hashtag used
raw_users | Contains only the user ID and name
spatial_ref_sys | Geospatial data used by Postgis
badge_updater_status

&nbsp;
## raw_changesets Table

The main table is raw_changesets, which contains extracted data from
the two files is then processed to create the various statistics. The
counters use the existing data, and only add to this value based on
what is in the change file, as this requires much less processing
time. This is the primary table Underpass updates the data in.

Keyword | Description
--------|------------
id | This is the ID of the changeset, and comes from the changeset file
road_km_added | This value is updated by counting the length of roads added or deleted by the user in the change file
road_km_modified | This value is updated by counting the length of existing roads modified by the user in the change file 
waterway_km_added | This value is updated by counting the length of waterways added or deleted by the user in the change file
waterway_km_modified | This value is updated by counting the length of existing waterways modified by the user in the change file
roads_added | This value is updated by counting the roads added or deleted by the user in the change file
roads_modified | This value is updated by counting the existing roads modified by the user in the change file
waterways_added | This value is updated by counting the roads added or deleted by the user in the change file
waterways_modified | This value is updated by counting the existing waterways modified by the user in the change file
buildings_added | This value is updated by counting the buildings added by the user in the change file
buildings_modified | This value is updated by counting the existing buildings modified by the user in the change file
pois_added | This value is updated by counting the POIs added by the user in the change file
pois_modified | This value is updated by counting the existing POIs modified by the user in the change file
editor | The editor used, and comes from the changeset
uid | The user ID, comes from the changeset
created_at | The timestamp this changeset was created, comes from the changeset
closed_at | The timestamp this changeset was closed, comes from the changeset
verified | Whether this data has been validated
updated_at | The timestamp this change was applied to the database
augmented_diffs 

&nbsp;
## raw_users Table

Keyword | Description
--------|------------
id | OSM user ID
name | OSM username

&nbsp;
## raw_hashtags Table

Keyword | Description
--------|------------
id | hashtag ID, internal use only
hashtag | OSM username

&nbsp;
## raw_countries Table

Keyword | Description
--------|------------
id | country ID, internal use only
name | Country full name
code | The 3 letter ISO abbreviation

&nbsp;
## raw_changesets_countries Table

Keyword | Description
--------|------------
changeset_id | The changeset ID
country_id | The country ID

&nbsp;
## raw_changesets_hashtags Table

Keyword | Description
--------|------------
changeset_id | The changeset ID
hashtag_id | The hashtag ID

&nbsp;
## changesets_status Table

Keyword | Description
--------|------------
id | The changeset ID
updated_at | Timestamp of the update

&nbsp;
## badge Table

Keyword | Description
--------|------------
id | The badge ID
category | The badge catagory
name | The badge name
level | The badge level

&nbsp;
## badges_users Table

Keyword | Description
--------|------------
uid | The OSM user ID
badge_id | The badge ID
updated_at | The timestamp of the user receiving this badge

&nbsp;
## augmented_diff_status Table

Keyword | Description
--------|------------
id | The change ID
updated_at | The timestamp when this change was applied
