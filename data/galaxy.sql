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
    osm_id bigint,
    user_id bigint,
    change_id bigint,
    type public.objtype,
    angle double precision,
    status public.status[],
    values text[],
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

ALTER TABLE ONLY public.geoboundaries
    ADD CONSTRAINT geoboundaries_pkey PRIMARY KEY (cid);

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

CREATE TYPE public.gendertype AS ENUM ('male', 'female', 'nonbinary', 'other', 'private');
CREATE TYPE public.agetype AS ENUM ('child', 'teen', 'adult', 'mature');
CREATE TYPE public.destype AS ENUM ('volunteer', 'intern', 'civil', 'gis', 'director');
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
    gender public.gendertype,
    agerange public.agetype,
    orgid integer,
    trainings integer,
    marginalized varchar,
    country integer,
    youthmapper bool,
    designation varchar,
    access int4,
    home public.geometry(Point,4326)
);

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);

--
-- possible table for training data
--
CREATE TYPE public.eventtype AS ENUM ('virtual', 'inperson');
CREATE TYPE public.eventtopic AS ENUM ('remote', 'field', 'other');
CREATE TABLE public.training (
       tid integer PRIMARY KEY,
       name text,
       location varchar,
       eventtype public.eventtype,
       topictype public.eventtopic,
       topics text[],
       hours integer,
       date date
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
CREATE TABLE public.organizations (
       name text PRIMARY KEY,
       orgid integer,
       unit public.units,
       trainee public.segments
);
