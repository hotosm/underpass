# Database Schemas

There are several database schemas used for OpenStreetMap data. The
primary two used by most people working with OSM files are created by
the two utility programs, *osm2pgsql* and *ogr2ogr*. Both work the
same, and contain the same data, it's just organized
differently. Underpass doesn't import into these database, as there
are actively maintained tools for this already. These schemas are
well-suited towards data extracts, and
[conflation](https://wiki.openstreetmap.org/wiki/Conflation) and
[validation](https://labs.mapbox.com/mapping/validating-osm/).

## osm2pgsql

[Osm2pgsql](https://osm2pgsql.org/) is the primary tool for importing
OSM files. The *osm2pgsql* program can import a data file into
postgres, but it doesn’t support updating the data. As it contains
geospatial data, it’s a compact and easily queried schema. It lacks
any knowledge of relations, which isn’t usually a problem unless you
want to do deeper analysis of the data.

&nbsp;

Table | Description |
--------|------------ |
planet_osm_line | All of the ways that aren't a polygon
planet_osm_point | All of the nodes
planet_osm_polygon | All of the polygons
planet_osm_roads | Unused currently
spatial_ref_sys | Geospatial data for postgis

All of these tables have the same columns in the database. Any tag
that doesn't fit one of these columns is stored in the *tags"
column. The tags column uses
[hstore](https://www.postgresqltutorial.com/postgresql-hstore/), which
can store additional keyword/value pairs. The columns are:
 osm_id, access, addr:housename, addr:housenumber, addr:interpolation,
 admin_level, aerialway, aeroway, amenity,  area, barrier, bicycle,
 brand, bridge, boundary, building, construction, covered, culvert,
 cutting, denomination, disused, embankment, foot, generator:source,
 harbour, highway, historic, horse, intermittent, junction, landuse,
 layer, leisure, lock, man_made, military, motorcar, name, natural,
 office, oneway, operator, place, population, power, power_source,
 public_transport, railway, ref, religion, route, service, shop
, sport, surface, toll, tourism, tower:type, tracktype, tunnel
, water, waterway, wetland, width, wood, z_order, way_area, tags, way

## ogr2ogr

[Ogr and GDAL](https://www.osgeo.org/projects/gdal/) are the basis for
many GIS projects, but has [minimal
OSM](https://gdal.org/drivers/vector/osm.html) support. It can read 
the OSM XML and PBF formats, and can import into a database. It uses a
different schema for each table.

&nbsp;

Table | Description |
--------|------------ |
lines | All of the single lines
multilinestrings | All of the multiple lines
multipolygons | All of the polygons
other_relations | More of the tags
points | All of the nodes
spatial_ref_sys | Geospatial data for postgis

[ogr2ogr](https://gdal.org/programs/ogr2ogr.html) uses a config file
(*osmconf.ini*) to determine which fields in the OSM file get imported
into which column. Any data not in one of these columns get put in the
*other_tags* column. Other_tags is an hstore column, so contains
multiple pairs of data. It is possible to modify the osmconf.ini file
to have data that was going into the other_tags column go into it's
own column.

The data columns of an ogr2ogr created database are:

Table | Columns |
--------|------------ |
multipolygons | ogc_fid, osm_id, osm_way_id, name, type, aeroway, amenity, admin_level, barrier, boundary, building, craft, geological, historic, land_area, landuse, leisure, man_made, military, natural, office, place, shop, sport, tourism, other_tags, wkb_geometry
multilinestrings | ogc_fid, osm_id, name, type, other_tags, wkb_geometry
lines | ogc_fid, osm_id, name, highway, waterway, aerialway, barrier, man_made, z_order, other_tags, wkb_geometry

# Example Queries

[SQL](https://en.wikipedia.org/wiki/SQL) is a powerful language for
manipulating database data.

	# Select all buildings
	SELECT name,wkb_geometry FROM multipolygons WHERE building='yes';

	# Count highways
	SELECT COUNT(wkb_geometry) FROM multilinestrings WHERE highway is NOT NULL;

	# Find bad highways
    # SELECT * FROM multilinestrings WHERE other_tags->'surface'='bad';

# Core Database Schemas

The primary one that operates the core servers uses **apidb**. There
is a simplified version called **pgsnapshot**, which is more suited
towards running ones own database. Both of these schemas are the only
ones that can be updated with replication files. A replication file is
the data of the actual change, and available with minutely, hourly, or
daily updates.

Osmosis is commonly used for manipulating and importing data into
these schemas. Although it has been unmaintained for several
years. It’s written in Java, and as it uses temporary files on disk,
an import can consume upwards of a terabyte.

Underpass can import a data file into the *pgsnapshot* schema without
*osmosis*. It can also use [replication files](changefile.md) to
update the database.

## pgsnapshot

The *pgsnapshot* schema is a more compact version of the main
database. It contains geospatial information, and can be updated using
replication files. It can also be easily queried to produce data
extracts in OSM format. By default, it is not well suited to any data
analysis that uses geospatial calculations since the nodes are stored
separately from the ways. Underpass extends this schema by by adding a
geometry column to the ways table.

&nbsp;

Keyword | Description
--------|------------
actions | The action to do for a change,ie.. Create, delete, or modify
locked | Locked when updating the database
nodes | All the nodes
relation_members | The memebers in a relation
relations | A list of replations
replication_changes | Counters on the changes in this replication file
schema_info | Version number of this schema
spatial_ref_sys | Geospatial data used by Postgis
sql_changes | Used when applying a replication file
state | The stat of the database update
users | A list of user IDs and usernames
way_nodes | A list of node references in a way
ways | A list of ways

## apidb

The *apidb* schema is the one that operates the core OpenStreetMap
servers, and is usually access through a REST API.


Keyword | Description |
--------|------------ |
acls | tbd
active_storage_attachments | tbd
active_storage_blobs | tbd
ar_internal_metadata | tbd
changeset_comments | tbd
changeset_tags | tbd
changesets | tbd
changesets_subscribers | tbd
client_applications | tbd
current_node_tags | tbd
current_nodes | tbd
current_relation_members | tbd
current_relation_tags | tbd
current_relations | tbd
current_way_nodes | tbd
current_way_tags | tbd
current_ways | tbd
delayed_jobs | tbd
diary_comments | tbd
diary_entries | tbd
diary_entry_subscriptions | tbd
friends | tbd
gps_points | tbd
gpx_file_tags | tbd
gpx_files | tbd
issue_comments | tbd
issues | tbd
languages | tbd
messages | tbd
node_tags | tbd
nodes | tbd
note_comments | tbd
notes | tbd
oauth_nonces | tbd
oauth_tokens | tbd
redactions | tbd
relation_members | tbd
relation_tags | tbd
relations | tbd
reports | tbd
schema_migrations | tbd
spatial_ref_sys | tbd
user_blocks | tbd
user_preferences | tbd
user_roles | tbd
user_tokens | tbd
users | tbd
way_nodes | tbd
way_tags | tbd
ways | tbd

