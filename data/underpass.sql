CREATE EXTENSION IF NOT EXISTS hstore WITH SCHEMA public;
CREATE EXTENSION IF NOT EXISTS postgis WITH SCHEMA public;

CREATE TYPE interval AS ENUM('minute', 'hour', 'day', 'changeset');

CREATE TABLE public.states (
    timestamp timestamp without time zone DEFAULT now() NOT NULL,
    path text NOT NULL,
    sequence integer NOT NULL,
    frequency text NOT NULL
);
ALTER TABLE states ADD PRIMARY KEY (path, sequence);

CREATE TABLE public.creators (
    user_id bigint NOT NULL,
    change_id bigint NOT NULL,
    editor text NOT NULL,
    hashtags text[] NOT NULL
);

CREATE TABLE public.way_nodes (
    way_id bigint NOT NULL,
    node_id bigint NOT NULL,
    sequence_id integer NOT NULL
);

CREATE TABLE public.ways (
    id bigint NOT NULL,
    version integer NOT NULL,
    user_id integer NOT NULL,
    tstamp timestamp without time zone NOT NULL,
    changeset_id bigint NOT NULL,
    tags public.hstore,
    nodes bigint[],
    bbox public.geometry(Geometry,4326),
    linestring public.geometry(Geometry,4326)
);

CREATE TABLE public.nodes (
    id bigint NOT NULL,
    version integer NOT NULL,
    user_id integer NOT NULL,
    tstamp timestamp without time zone NOT NULL,
    changeset_id bigint NOT NULL,
    tags public.hstore,
    geom public.geometry(Point,4326)
);
