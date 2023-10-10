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

CREATE TABLE IF NOT EXISTS public.changesets (
    id int8 NOT NULL,
    editor text,
    uid integer NOT NULL,
    created_at timestamptz,
    closed_at timestamptz,
    updated_at timestamptz,
    added public.hstore,
    modified public.hstore,
    deleted public.hstore,
    hashtags text[],
    source text,
    validated boolean,
    quality integer,
    bbox public.geometry(MultiPolygon,4326)
);
ALTER TABLE ONLY public.changesets
    ADD CONSTRAINT changesets_pkey PRIMARY KEY (id);

DROP TYPE IF EXISTS public.objtype;
CREATE TYPE public.objtype AS ENUM ('node', 'way', 'relation');
DROP TYPE IF EXISTS public.status;
CREATE TYPE public.status AS ENUM ('notags', 'complete', 'incomplete', 'badvalue', 'correct', 'badgeom', 'orphan', 'overlaping', 'duplicate');

CREATE TABLE IF NOT EXISTS public.validation (
    osm_id int8,
    uid int8,
    changeset int8,
    type public.objtype,
    status public.status,
    values text[],
    source text,
    version bigint,
    timestamp timestamp with time zone,
    location public.geometry(Geometry,4326)
);
ALTER TABLE ONLY public.validation
    ADD CONSTRAINT validation_pkey PRIMARY KEY (osm_id, status, source);

CREATE TABLE IF NOT EXISTS public.ways_poly (
    osm_id int8,
    changeset int8,
    geom public.geometry(Polygon,4326),
    tags JSONB,
    refs int8[],
    timestamp timestamp with time zone,
    version int,
    "user" text,
    uid int8
);

CREATE TABLE IF NOT EXISTS public.ways_line (
    osm_id int8,
    changeset int8,
    geom public.geometry(LineString,4326),
    tags JSONB,
    refs int8[],
    timestamp timestamp with time zone,
    version int,
    "user" text,
    uid int8
);

CREATE TABLE IF NOT EXISTS public.nodes (
    osm_id int8,
    changeset int8,
    geom public.geometry(Point,4326),
    tags JSONB,
    timestamp timestamp with time zone,
    version int,
    "user" text,
    uid int8
);

CREATE TABLE IF NOT EXISTS public.way_refs (
    way_id int8,
    node_id int8
);

CREATE UNIQUE INDEX nodes_id_idx ON public.nodes (osm_id);
CREATE UNIQUE INDEX ways_poly_id_idx ON public.ways_poly (osm_id);
CREATE UNIQUE INDEX ways_line_id_idx ON public.ways_line(osm_id);
CREATE INDEX way_refs_node_id_idx ON public.way_refs (node_id);
CREATE INDEX way_refs_way_id_idx ON public.way_refs (way_id);

CREATE INDEX nodes_version_idx ON public.nodes (version);
CREATE INDEX ways_poly_version_idx ON public.ways_poly (version);
CREATE INDEX ways_line_version_idx ON public.ways_line (version);

CREATE INDEX nodes_timestamp_idx ON public.nodes(timestamp DESC);
CREATE INDEX ways_poly_timestamp_idx ON public.ways_poly(timestamp DESC);
CREATE INDEX ways_line_timestamp_idx ON public.ways_line(timestamp DESC);
