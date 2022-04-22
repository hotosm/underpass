--
-- Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
--
-- This file is part of Underpass.
--
--     Underpass is free software: you can redistribute it and/or modify
--     it under the terms of the GNU General Public License as published by
--     the Free Software Foundation, either version 3 of the License, or
--     (at your option) any later version.
--
--     Underpass is distributed in the hope that it will be useful,
--     but WITHOUT ANY WARRANTY; without even the implied warranty of
--     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--     GNU General Public License for more details.
--
--     You should have received a copy of the GNU General Public License
--     along with Underpass.  If not, see <https:--www.gnu.org/licenses/>.

create extension if not exists btree_gist;
create extension if not exists postgis;

-- tables with these columns will be automatically created with lua insertion 
CREATE TABLE IF NOT EXISTS  nodes (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(point, 4326) NULL,
	grid int4 NULL
);
-- Multi column index is used for the larger datasets which will have large gist index size (such as loading asia, america) to reduce amount of the index to look for spatial query , db with smaller datasets such as only with few grid level data it makes no sense , by default with lua script it will create gist index ordered with geohash method 
CREATE INDEX CONCURRENTLY  IF NOT EXISTS   nodes_grid_geom_idx ON public.nodes USING gist (grid, geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS   nodes_osm_id_idx ON public.nodes USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS   nodes_timestamp_idx ON public.nodes USING btree ("timestamp");



CREATE TABLE IF NOT EXISTS  ways_line (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(linestring, 4326) NULL,
	grid int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_line_grid_geom_idx ON public.ways_line USING gist (grid, geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_line_osm_id_idx ON public.ways_line USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_line_timestamp_idx ON public.ways_line USING btree ("timestamp");


CREATE TABLE IF NOT EXISTS  ways_poly (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(polygon, 4326) NULL,
	grid int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_poly_grid_geom_idx ON public.ways_poly USING gist (grid, geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_poly_osm_id_idx ON public.ways_poly USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_poly_timestamp_idx ON public.ways_poly USING btree ("timestamp");



CREATE TABLE IF NOT EXISTS  relations (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(geometry, 4326) NULL,
	grid int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  relations_geom_idx ON public.relations USING gist (geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  relations_osm_id_idx ON public.relations USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  relations_tags_idx ON public.relations USING gin (tags);

-- Clustering command , it is recommended for larger datasets , It will be easy for indexes to find right files faster , will also reduce coutry geom index size since all related rows will be clustered as block 

CLUSTER nodes USING nodes_grid_geom_idx;
CLUSTER ways_line USING ways_line_grid_geom_idx;
CLUSTER ways_poly USING ways_poly_grid_geom_idx;
