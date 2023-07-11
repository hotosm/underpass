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

CREATE TABLE public.changesets (
    id int8 NOT NULL,
    editor text,
    user_id integer NOT NULL,
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

CREATE TYPE public.objtype AS ENUM ('node', 'way', 'relation');
CREATE TYPE public.status AS ENUM ('notags', 'complete', 'incomplete', 'badvalue', 'correct', 'badgeom', 'orphan', 'overlaping', 'duplicate');

CREATE TABLE public.validation (
    osm_id int8,
    user_id int8,
    change_id int8,
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

CREATE TABLE public.raw_poly (
    osm_id int8,
    change_id int8,
    geometry public.geometry(Polygon,4326),
    tags public.hstore,
    refs int8[],
    timestamp timestamp with time zone,
    version int
);

CREATE TABLE public.raw_node (
    osm_id int8,
    change_id int8,
    geometry public.geometry(Point,4326),
    tags public.hstore,
    timestamp timestamp with time zone,
    version int
);

CREATE TABLE way_refs (
    way_id int8,
    node_id int8
)

CREATE UNIQUE INDEX raw_osm_id_idx ON public.raw_node (osm_id);
CREATE UNIQUE INDEX raw_poly_osm_id_idx ON public.raw_poly (osm_id);
