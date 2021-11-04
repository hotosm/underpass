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
    validated boolean,
    quality integer,
    bbox public.geometry(MultiPolygon,4326)
);
ALTER TABLE ONLY public.changesets
    ADD CONSTRAINT changesets_pkey PRIMARY KEY (id);

CREATE TYPE public.objtype AS ENUM ('node', 'way', 'relation');
CREATE TYPE public.status AS ENUM ('notags', 'complete', 'incomplete', 'badvalue', 'correct', 'badgeom');

CREATE TABLE public.validation (
    osm_id bigint,
    user_id bigint,
    change_id bigint,
    type public.objtype,
    angle double precision,
    status public.status[],
    timestamp timestamp with time zone,
    location public.geometry(Geometry,4326)
);
ALTER TABLE ONLY public.validation
    ADD CONSTRAINT validation_pkey PRIMARY KEY (osm_id);

--
-- Name: geoboundaries; Type: TABLE; Schema: public;
--

CREATE TABLE public.geoboundaries (
    cid integer NOT NULL,
    name character varying NOT NULL,
    admin_level integer,
    tags public.hstore,
    priority boolean,
    boundary public.geometry(MultiPolygon,4326)
);

--
-- Name: ground_data; Type: TABLE; Schema: public;
--

CREATE TABLE public.ground_data (
    starttime timestamp with time zone NOT NULL,
    endtime timestamp with time zone NOT NULL,
    username text NOT NULL,
    changeset_id integer,
    location public.geometry
);

CREATE TABLE public.users (
    id bigint NOT NULL,
    username varchar,
    name varchar,
    date_registered timestamp,
    last_validation_date timestamp,
    osm_registration timestamp,
    tasks_mapped integer,
    tasks_validated integer,
    tasks_invalidated integer,
    projects_mapped integer[],
    mapping_level integer,
    gender int4,
    age real,
    access int4,
    home public.geometry(Point,4326)
);

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);

--
-- possible table for training data
--
CREATE TABLE public.training (
    name text,
    local boolean,
    organization oid,
    topics text[],
    hours integer,
    timestamp timestamp with time zone
);
CREATE TYPE public.units AS ENUM (
       'country',
       'region',
       'microgrant',
       'organization',
       'osm',
       'boundary',
       'campaign',
       'hot'
);
CREATE TYPE public.segments AS ENUM (
       'new_existing',
       'youth_mappers',
       'ngo',
       'government'
);
CREATE TABLE public.organization (
       name text,
       oid int,
       unit public.units,
       trainee public.segments
);

