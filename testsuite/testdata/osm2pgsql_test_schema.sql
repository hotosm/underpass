--
-- PostgreSQL database dump
--

-- Dumped from database version 12.8 (Ubuntu 12.8-0ubuntu0.20.04.1)
-- Dumped by pg_dump version 12.8 (Ubuntu 12.8-0ubuntu0.20.04.1)

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

--
-- Name: osm2pgsql_pgsql; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA osm2pgsql_pgsql;


--
-- Name: hstore; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS hstore WITH SCHEMA public;


--
-- Name: EXTENSION hstore; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON EXTENSION hstore IS 'data type for storing sets of (key, value) pairs';


--
-- Name: postgis; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS postgis WITH SCHEMA public;


--
-- Name: EXTENSION postgis; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON EXTENSION postgis IS 'PostGIS geometry, geography, and raster spatial types and functions';


SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: planet_osm_line; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_line (
    osm_id bigint,
    access text,
    "addr:housename" text,
    "addr:housenumber" text,
    "addr:interpolation" text,
    admin_level text,
    aerialway text,
    aeroway text,
    amenity text,
    area text,
    barrier text,
    bicycle text,
    brand text,
    bridge text,
    boundary text,
    building text,
    construction text,
    covered text,
    culvert text,
    cutting text,
    denomination text,
    disused text,
    embankment text,
    foot text,
    "generator:source" text,
    harbour text,
    highway text,
    historic text,
    horse text,
    intermittent text,
    junction text,
    landuse text,
    layer text,
    leisure text,
    lock text,
    man_made text,
    military text,
    motorcar text,
    name text,
    "natural" text,
    office text,
    oneway text,
    operator text,
    place text,
    population text,
    power text,
    power_source text,
    public_transport text,
    railway text,
    ref text,
    religion text,
    route text,
    service text,
    shop text,
    sport text,
    surface text,
    toll text,
    tourism text,
    "tower:type" text,
    tracktype text,
    tunnel text,
    water text,
    waterway text,
    wetland text,
    width text,
    wood text,
    z_order integer,
    way_area real,
    tags public.hstore,
    way public.geometry(LineString,4326)
);


--
-- Name: planet_osm_nodes; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_nodes (
    id bigint NOT NULL,
    lat integer NOT NULL,
    lon integer NOT NULL
);


--
-- Name: planet_osm_point; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_point (
    osm_id bigint,
    access text,
    "addr:housename" text,
    "addr:housenumber" text,
    "addr:interpolation" text,
    admin_level text,
    aerialway text,
    aeroway text,
    amenity text,
    area text,
    barrier text,
    bicycle text,
    brand text,
    bridge text,
    boundary text,
    building text,
    capital text,
    construction text,
    covered text,
    culvert text,
    cutting text,
    denomination text,
    disused text,
    ele text,
    embankment text,
    foot text,
    "generator:source" text,
    harbour text,
    highway text,
    historic text,
    horse text,
    intermittent text,
    junction text,
    landuse text,
    layer text,
    leisure text,
    lock text,
    man_made text,
    military text,
    motorcar text,
    name text,
    "natural" text,
    office text,
    oneway text,
    operator text,
    place text,
    population text,
    power text,
    power_source text,
    public_transport text,
    railway text,
    ref text,
    religion text,
    route text,
    service text,
    shop text,
    sport text,
    surface text,
    toll text,
    tourism text,
    "tower:type" text,
    tunnel text,
    water text,
    waterway text,
    wetland text,
    width text,
    wood text,
    z_order integer,
    tags public.hstore,
    way public.geometry(Point,4326)
);


--
-- Name: planet_osm_polygon; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_polygon (
    osm_id bigint,
    access text,
    "addr:housename" text,
    "addr:housenumber" text,
    "addr:interpolation" text,
    admin_level text,
    aerialway text,
    aeroway text,
    amenity text,
    area text,
    barrier text,
    bicycle text,
    brand text,
    bridge text,
    boundary text,
    building text,
    construction text,
    covered text,
    culvert text,
    cutting text,
    denomination text,
    disused text,
    embankment text,
    foot text,
    "generator:source" text,
    harbour text,
    highway text,
    historic text,
    horse text,
    intermittent text,
    junction text,
    landuse text,
    layer text,
    leisure text,
    lock text,
    man_made text,
    military text,
    motorcar text,
    name text,
    "natural" text,
    office text,
    oneway text,
    operator text,
    place text,
    population text,
    power text,
    power_source text,
    public_transport text,
    railway text,
    ref text,
    religion text,
    route text,
    service text,
    shop text,
    sport text,
    surface text,
    toll text,
    tourism text,
    "tower:type" text,
    tracktype text,
    tunnel text,
    water text,
    waterway text,
    wetland text,
    width text,
    wood text,
    z_order integer,
    way_area real,
    tags public.hstore,
    way public.geometry(Geometry,4326)
);


--
-- Name: planet_osm_rels; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_rels (
    id bigint NOT NULL,
    way_off smallint,
    rel_off smallint,
    parts bigint[],
    members text[],
    tags text[]
);


--
-- Name: planet_osm_roads; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_roads (
    osm_id bigint,
    access text,
    "addr:housename" text,
    "addr:housenumber" text,
    "addr:interpolation" text,
    admin_level text,
    aerialway text,
    aeroway text,
    amenity text,
    area text,
    barrier text,
    bicycle text,
    brand text,
    bridge text,
    boundary text,
    building text,
    construction text,
    covered text,
    culvert text,
    cutting text,
    denomination text,
    disused text,
    embankment text,
    foot text,
    "generator:source" text,
    harbour text,
    highway text,
    historic text,
    horse text,
    intermittent text,
    junction text,
    landuse text,
    layer text,
    leisure text,
    lock text,
    man_made text,
    military text,
    motorcar text,
    name text,
    "natural" text,
    office text,
    oneway text,
    operator text,
    place text,
    population text,
    power text,
    power_source text,
    public_transport text,
    railway text,
    ref text,
    religion text,
    route text,
    service text,
    shop text,
    sport text,
    surface text,
    toll text,
    tourism text,
    "tower:type" text,
    tracktype text,
    tunnel text,
    water text,
    waterway text,
    wetland text,
    width text,
    wood text,
    z_order integer,
    way_area real,
    tags public.hstore,
    way public.geometry(LineString,4326)
);


--
-- Name: planet_osm_ways; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.planet_osm_ways (
    id bigint NOT NULL,
    nodes bigint[] NOT NULL,
    tags text[]
);


--
-- Name: planet_osm_nodes planet_osm_nodes_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.planet_osm_nodes
    ADD CONSTRAINT planet_osm_nodes_pkey PRIMARY KEY (id);


--
-- Name: planet_osm_rels planet_osm_rels_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.planet_osm_rels
    ADD CONSTRAINT planet_osm_rels_pkey PRIMARY KEY (id);


--
-- Name: planet_osm_ways planet_osm_ways_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public.planet_osm_ways
    ADD CONSTRAINT planet_osm_ways_pkey PRIMARY KEY (id);


--
-- Name: planet_osm_line_osm_id_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_line_osm_id_idx ON public.planet_osm_line USING btree (osm_id);


--
-- Name: planet_osm_line_way_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_line_way_idx ON public.planet_osm_line USING gist (way);


--
-- Name: planet_osm_point_osm_id_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_point_osm_id_idx ON public.planet_osm_point USING btree (osm_id);


--
-- Name: planet_osm_point_way_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_point_way_idx ON public.planet_osm_point USING gist (way);


--
-- Name: planet_osm_polygon_osm_id_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_polygon_osm_id_idx ON public.planet_osm_polygon USING btree (osm_id);


--
-- Name: planet_osm_polygon_way_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_polygon_way_idx ON public.planet_osm_polygon USING gist (way);


--
-- Name: planet_osm_rels_parts_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_rels_parts_idx ON public.planet_osm_rels USING gin (parts) WITH (fastupdate=off);


--
-- Name: planet_osm_roads_osm_id_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_roads_osm_id_idx ON public.planet_osm_roads USING btree (osm_id);


--
-- Name: planet_osm_roads_way_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_roads_way_idx ON public.planet_osm_roads USING gist (way);


--
-- Name: planet_osm_ways_nodes_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX planet_osm_ways_nodes_idx ON public.planet_osm_ways USING gin (nodes) WITH (fastupdate=off);


--
-- PostgreSQL database dump complete
--

