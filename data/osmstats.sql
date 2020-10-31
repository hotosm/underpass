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

CREATE TABLE public.augmented_diff_status (
    id integer,
    updated_at timestamp with time zone
);

CREATE TABLE public.badge_updater_status (
    last_run timestamp with time zone
);

CREATE TABLE public.badges (
    id integer NOT NULL,
    category integer,
    name text,
    level integer
);

CREATE SEQUENCE public.badges_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.badges_id_seq OWNED BY public.badges.id;

CREATE TABLE public.badges_users (
    user_id integer NOT NULL,
    badge_id integer NOT NULL,
    updated_at timestamp with time zone DEFAULT now()
);

CREATE TABLE public.raw_changesets (
    id bigint NOT NULL,
    road_km_added double precision,
    road_km_modified double precision,
    waterway_km_added double precision,
    waterway_km_modified double precision,
    roads_added integer,
    roads_modified integer,
    waterways_added integer,
    waterways_modified integer,
    buildings_added integer,
    buildings_modified integer,
    pois_added integer,
    pois_modified integer,
    editor text,
    user_id integer,
    created_at timestamp with time zone,
    closed_at timestamp with time zone,
    verified boolean DEFAULT false,
    augmented_diffs integer[],
    updated_at timestamp with time zone
);

CREATE VIEW public.changesets AS
 SELECT raw_changesets.id,
    raw_changesets.roads_added AS road_count_add,
    raw_changesets.roads_modified AS road_count_mod,
    raw_changesets.buildings_added AS building_count_add,
    raw_changesets.buildings_modified AS building_count_mod,
    raw_changesets.waterways_added AS waterway_count_add,
    raw_changesets.pois_added AS poi_count_add,
    0 AS gpstrace_count_add,
    raw_changesets.road_km_added AS road_km_add,
    raw_changesets.road_km_modified AS road_km_mod,
    raw_changesets.waterway_km_added AS waterway_km_add,
    0 AS gpstrace_km_add,
    raw_changesets.editor,
    raw_changesets.user_id,
    raw_changesets.created_at
   FROM public.raw_changesets;

CREATE TABLE public.raw_changesets_countries (
    changeset_id integer NOT NULL,
    country_id integer NOT NULL
);

CREATE VIEW public.changesets_countries AS
 SELECT raw_changesets_countries.changeset_id,
    raw_changesets_countries.country_id
   FROM public.raw_changesets_countries;

CREATE TABLE public.raw_changesets_hashtags (
    changeset_id integer NOT NULL,
    hashtag_id integer NOT NULL
);

CREATE VIEW public.changesets_hashtags AS
 SELECT raw_changesets_hashtags.changeset_id,
    raw_changesets_hashtags.hashtag_id
   FROM public.raw_changesets_hashtags;

CREATE TABLE public.changesets_status (
    id integer,
    updated_at timestamp with time zone
);

CREATE TABLE public.raw_countries (
    id integer NOT NULL,
    name text,
    code text NOT NULL
);

CREATE VIEW public.countries AS
 SELECT raw_countries.id,
    raw_countries.name,
    raw_countries.code
   FROM public.raw_countries;

CREATE TABLE public.raw_hashtags (
    id integer NOT NULL,
    hashtag text NOT NULL
);

CREATE MATERIALIZED VIEW public.hashtag_stats AS
 SELECT h.hashtag,
    count(c.id) AS changesets,
    count(DISTINCT c.user_id) AS users,
    sum(c.road_km_added) AS road_km_added,
    sum(c.road_km_modified) AS road_km_modified,
    sum(c.waterway_km_added) AS waterway_km_added,
    sum(c.waterway_km_modified) AS waterway_km_modified,
    sum(c.roads_added) AS roads_added,
    sum(c.roads_modified) AS roads_modified,
    sum(c.waterways_added) AS waterways_added,
    sum(c.waterways_modified) AS waterways_modified,
    sum(c.buildings_added) AS buildings_added,
    sum(c.buildings_modified) AS buildings_modified,
    sum(c.pois_added) AS pois_added,
    sum(c.pois_modified) AS pois_modified,
    sum(
        CASE
            WHEN ("position"(lower(c.editor), 'josm'::text) > 0) THEN 1
            ELSE 0
        END) AS josm_edits,
    max(COALESCE(c.closed_at, c.created_at)) AS updated_at
   FROM ((public.raw_changesets_hashtags ch
     JOIN public.raw_changesets c ON ((c.id = ch.changeset_id)))
     JOIN public.raw_hashtags h ON ((h.id = ch.hashtag_id)))
  GROUP BY h.hashtag
  WITH NO DATA;

CREATE VIEW public.hashtags AS
 SELECT raw_hashtags.id,
    raw_hashtags.hashtag
   FROM public.raw_hashtags;

CREATE SEQUENCE public.raw_countries_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.raw_countries_id_seq OWNED BY public.raw_countries.id;

CREATE MATERIALIZED VIEW public.raw_countries_users AS
 SELECT raw_changesets_countries.country_id,
    raw_changesets.user_id,
    count(raw_changesets.id) AS changesets
   FROM (public.raw_changesets_countries
     JOIN public.raw_changesets ON ((raw_changesets.id = raw_changesets_countries.changeset_id)))
  GROUP BY raw_changesets_countries.country_id, raw_changesets.user_id
  WITH NO DATA;

CREATE SEQUENCE public.raw_hashtags_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;

ALTER SEQUENCE public.raw_hashtags_id_seq OWNED BY public.raw_hashtags.id;

CREATE MATERIALIZED VIEW public.raw_hashtags_users AS
 SELECT _.hashtag_id,
    _.user_id,
    _.changesets,
    _.edits,
    _.buildings,
    _.roads,
    _.road_km,
    _.updated_at,
    rank() OVER (ORDER BY _.edits DESC) AS edits_rank,
    rank() OVER (ORDER BY _.buildings DESC) AS buildings_rank,
    rank() OVER (ORDER BY _.road_km DESC) AS road_km_rank,
    rank() OVER (ORDER BY _.updated_at DESC) AS updated_at_rank
   FROM ( SELECT raw_changesets_hashtags.hashtag_id,
            raw_changesets.user_id,
            count(raw_changesets.id) AS changesets,
            sum((((((((raw_changesets.buildings_added + raw_changesets.buildings_modified) + raw_changesets.roads_added) + raw_changesets.roads_modified) + raw_changesets.waterways_added) + raw_changesets.waterways_modified) + raw_changesets.pois_added) + raw_changesets.pois_modified)) AS edits,
            sum((raw_changesets.buildings_added + raw_changesets.buildings_modified)) AS buildings,
            sum((raw_changesets.roads_added + raw_changesets.roads_modified)) AS roads,
            sum((raw_changesets.road_km_added + raw_changesets.road_km_modified)) AS road_km,
            max(COALESCE(raw_changesets.closed_at, raw_changesets.created_at)) AS updated_at
           FROM (public.raw_changesets
             JOIN public.raw_changesets_hashtags ON ((raw_changesets_hashtags.changeset_id = raw_changesets.id)))
          GROUP BY raw_changesets_hashtags.hashtag_id, raw_changesets.user_id) _
  WITH NO DATA;

CREATE TABLE public.raw_users (
    id integer NOT NULL,
    name text
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

ALTER TABLE ONLY public.badges ALTER COLUMN id SET DEFAULT nextval('public.badges_id_seq'::regclass);

ALTER TABLE ONLY public.raw_countries ALTER COLUMN id SET DEFAULT nextval('public.raw_countries_id_seq'::regclass);

ALTER TABLE ONLY public.raw_hashtags ALTER COLUMN id SET DEFAULT nextval('public.raw_hashtags_id_seq'::regclass);

ALTER TABLE ONLY public.badges
    ADD CONSTRAINT badges_pkey PRIMARY KEY (id);

ALTER TABLE ONLY public.badges_users
    ADD CONSTRAINT badges_users_pkey PRIMARY KEY (user_id, badge_id);

ALTER TABLE ONLY public.raw_changesets_countries
    ADD CONSTRAINT raw_changesets_countries_pkey PRIMARY KEY (changeset_id, country_id);

ALTER TABLE ONLY public.raw_changesets_hashtags
    ADD CONSTRAINT raw_changesets_hashtags_pkey PRIMARY KEY (changeset_id, hashtag_id);

ALTER TABLE ONLY public.raw_changesets
    ADD CONSTRAINT raw_changesets_pkey PRIMARY KEY (id);

ALTER TABLE ONLY public.raw_countries
    ADD CONSTRAINT raw_countries_code_key UNIQUE (code);

ALTER TABLE ONLY public.raw_countries
    ADD CONSTRAINT raw_countries_pkey PRIMARY KEY (id);

ALTER TABLE ONLY public.raw_hashtags
    ADD CONSTRAINT raw_hashtags_hashtag_key UNIQUE (hashtag);

ALTER TABLE ONLY public.raw_hashtags
    ADD CONSTRAINT raw_hashtags_pkey PRIMARY KEY (id);

ALTER TABLE ONLY public.raw_users
    ADD CONSTRAINT raw_users_pkey PRIMARY KEY (id);

CREATE UNIQUE INDEX hashtag_stats_hashtag ON public.hashtag_stats USING btree (hashtag);

CREATE INDEX raw_changesets_user_id ON public.raw_changesets USING btree (user_id);

CREATE INDEX raw_countries_users_country_id ON public.raw_countries_users USING btree (country_id);

CREATE UNIQUE INDEX raw_countries_users_country_id_user_id ON public.raw_countries_users USING btree (country_id, user_id);

CREATE INDEX raw_countries_users_user_id ON public.raw_countries_users USING btree (user_id);

CREATE UNIQUE INDEX raw_hashtags_hashtag_idx ON public.raw_hashtags USING btree (hashtag);

CREATE UNIQUE INDEX raw_hashtags_hashtag_idx1 ON public.raw_hashtags USING btree (hashtag);

CREATE INDEX raw_hashtags_users_hashtag_id ON public.raw_hashtags_users USING btree (hashtag_id);

CREATE UNIQUE INDEX raw_hashtags_users_hashtag_id_user_id ON public.raw_hashtags_users USING btree (hashtag_id, user_id);

CREATE INDEX raw_hashtags_users_user_id ON public.raw_hashtags_users USING btree (user_id);

CREATE INDEX user_stats_updated_at ON public.user_stats USING btree (updated_at);


CREATE UNIQUE INDEX user_stats_user_id ON public.user_stats USING btree (user_id);

