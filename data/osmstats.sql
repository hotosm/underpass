--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

CREATE EXTENSION IF NOT EXISTS hstore WITH SCHEMA public;
COMMENT ON EXTENSION hstore IS 'data type for storing sets of (key, value) pairs';
CREATE EXTENSION IF NOT EXISTS postgis WITH SCHEMA public;
COMMENT ON EXTENSION postgis IS 'PostGIS geometry, geography, and raster spatial types and functions';

SET default_tablespace = '';

SET default_table_access_method = heap;

CREATE TABLE public.changesets (
    id bigint NOT NULL,
    editor text,
    user_id integer NOT NULL,
    created_at timestamp with time zone,
    closed_at timestamp with time zone,
    updated_at timestamp with time zone,
    added public.hstore,
    modified public.hstore,
    deleted public.hstore,
    hashtags text[],
    source text,
    changes public.hstore,
    geom public.geometry(Geometry,4326)
);

--
-- Name: geoboundaries; Type: TABLE; Schema: public;
--

CREATE TABLE public.geoboundaries (
    cid integer NOT NULL,
    name character varying NOT NULL,
    admin_level integer,
    other_tags public.hstore,
    boundary public.geometry
);

--
-- Name: raw_ground; Type: TABLE; Schema: public;
--

CREATE TABLE public.ground_data (
    starttime timestamp with time zone NOT NULL,
    endtime timestamp with time zone NOT NULL,
    username text NOT NULL,
    changeset_id integer,
    location public.geometry
);


CREATE TABLE public.raw_users (
    id integer NOT NULL,
    name text,
    tm_registration timestamp with time zone,
    osm_registration timestamp with time zone,
    tasks_mapped integer,
    tasks_validated integer,
    tasks_invalidated integer,
    projects_mapped integer[],
    gender text,
    home geometry(Point,4326)
);

CREATE MATERIALIZED VIEW public.user_stats AS
 SELECT raw_changesets.user_id,
    raw_users.name,
    count(raw_changesets.id) AS changesets,
    sum(raw_changesets.road_km_added) AS road_km_added,
    sum(raw_changesets.road_km_modified) AS road_km_modified,
    sum(raw_changesets.waterway_km_added) AS waterway_km_added,
    sum(raw_changesets.waterway_km_modified) AS waterway_km_modified,
    sum(raw_changesets.roads_added) AS roads_added,
    sum(raw_changesets.roads_modified) AS roads_modified,
    sum(raw_changesets.waterways_added) AS waterways_added,
    sum(raw_changesets.waterways_modified) AS waterways_modified,
    sum(raw_changesets.buildings_added) AS buildings_added,
    sum(raw_changesets.buildings_modified) AS buildings_modified,
    sum(raw_changesets.pois_added) AS pois_added,
    sum(raw_changesets.pois_modified) AS pois_modified,
    sum(
        CASE
            WHEN ("position"(lower(raw_changesets.editor), 'josm'::text) > 0) THEN 1
            ELSE 0
        END) AS josm_edits,
    ( SELECT count(raw_countries_users.country_id) AS count
           FROM public.raw_countries_users
          WHERE (raw_countries_users.user_id = raw_changesets.user_id)) AS countries,
    ( SELECT count(raw_hashtags_users.hashtag_id) AS count
           FROM public.raw_hashtags_users
          WHERE (raw_hashtags_users.user_id = raw_changesets.user_id)) AS hashtags,
    max(COALESCE(raw_changesets.closed_at, raw_changesets.created_at)) AS updated_at
   FROM (public.raw_changesets
     JOIN public.raw_users ON ((raw_changesets.user_id = raw_users.id)))
  WHERE (raw_changesets.user_id IS NOT NULL)
  GROUP BY raw_changesets.user_id, raw_users.name
  WITH NO DATA;

CREATE VIEW public.users AS
 SELECT u.id,
    u.name,
    us.changesets,
    NULL::public.geometry AS geo_extent,
    us.buildings_added AS total_building_count_add,
    us.buildings_modified AS total_building_count_mod,
    us.waterways_added AS total_waterway_count_add,
    us.pois_added AS total_poi_count_add,
    us.road_km_added AS total_road_km_add,
    us.road_km_modified AS total_road_km_mod,
    us.waterway_km_added AS total_waterway_km_add,
    us.josm_edits AS total_josm_edit_count,
    0 AS total_gps_trace_count_add,
    0 AS total_gps_trace_updated_from_osm,
    us.roads_added AS total_road_count_add,
    us.roads_modified AS total_road_count_mod,
    0 AS total_tm_done_count,
    0 AS total_tm_val_count,
    0 AS total_tm_inval_count
   FROM (public.raw_users u
     JOIN public.user_stats us ON ((us.user_id = u.id)));

ALTER TABLE ONLY public.raw_changesets
    ADD CONSTRAINT raw_changesets_pkey PRIMARY KEY (id);

ALTER TABLE ONLY public.raw_users
    ADD CONSTRAINT raw_users_pkey PRIMARY KEY (id);

ALTER TABLE ONLY public.geoboundaries
    ADD CONSTRAINT geoboundaries_pkey PRIMARY KEY (cid);
