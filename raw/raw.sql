create extension if not exists btree_gist;
create extension if not exists postgis;

CREATE TABLE IF NOT EXISTS  nodes (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(point, 4326) NULL,
	country int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS   nodes_country_geom_idx ON public.nodes USING gist (country, geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS   nodes_osm_id_idx ON public.nodes USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS   nodes_timestamp_idx ON public.nodes USING btree ("timestamp");

-- CLUSTER nodes USING nodes_country_geom_idx;

CREATE TABLE IF NOT EXISTS  ways_line (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(linestring, 4326) NULL,
	country int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_line_country_geom_idx ON public.ways_line USING gist (country, geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_line_osm_id_idx ON public.ways_line USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_line_timestamp_idx ON public.ways_line USING btree ("timestamp");

-- CLUSTER ways_line USING ways_line_country_geom_idx;

CREATE TABLE IF NOT EXISTS  ways_poly (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(polygon, 4326) NULL,
	country int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_poly_country_geom_idx ON public.ways_poly USING gist (country, geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_poly_osm_id_idx ON public.ways_poly USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  ways_poly_timestamp_idx ON public.ways_poly USING btree ("timestamp");

-- CLUSTER ways_poly USING ways_poly_country_geom_idx;


CREATE TABLE IF NOT EXISTS  relations (
	osm_id int8 NOT NULL,
	uid int4 NULL,
	"user" text NULL,
	"version" int4 NULL,
	changeset int4 NULL,
	"timestamp" timestamp NULL,
	tags jsonb NULL,
	geom geometry(geometry, 4326) NULL,
	country int4 NULL
);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  relations_geom_idx ON public.relations USING gist (geom);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  relations_osm_id_idx ON public.relations USING btree (osm_id);
CREATE INDEX CONCURRENTLY  IF NOT EXISTS  relations_tags_idx ON public.relations USING gin (tags);