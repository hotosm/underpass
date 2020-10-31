--
-- PostgreSQL database dump
--

-- Dumped from database version 12.4
-- Dumped by pg_dump version 12.4

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
-- Name: hstore; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS hstore WITH SCHEMA public;


--
-- Name: EXTENSION hstore; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION hstore IS 'data type for storing sets of (key, value) pairs';


--
-- Name: postgis; Type: EXTENSION; Schema: -; Owner: -
--

CREATE EXTENSION IF NOT EXISTS postgis WITH SCHEMA public;


--
-- Name: EXTENSION postgis; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION postgis IS 'PostGIS geometry, geography, and raster spatial types and functions';


--
-- Name: first_agg(anyelement, anyelement); Type: FUNCTION; Schema: public; Owner: rob
--

CREATE FUNCTION public.first_agg(anyelement, anyelement) RETURNS anyelement
    LANGUAGE sql STABLE
    AS $_$
        SELECT CASE WHEN $1 IS NULL THEN $2 ELSE $1 END;
$_$;


ALTER FUNCTION public.first_agg(anyelement, anyelement) OWNER TO rob;

--
-- Name: lock_database(text, text, text, boolean); Type: FUNCTION; Schema: public; Owner: rob
--

CREATE FUNCTION public.lock_database(new_process text, new_source text, new_location text, request_write_lock boolean) RETURNS integer
    LANGUAGE plpgsql
    AS $$
  DECLARE locked_id INT;
  DECLARE current_id INT;
  DECLARE current_process TEXT;
  DECLARE current_source TEXT;
  DECLARE current_location TEXT;
  DECLARE current_write_lock BOOLEAN;
BEGIN

   SELECT id, process, source, location, write_lock
    INTO current_id, current_process, current_source, current_location, current_write_lock
    FROM locked ORDER BY write_lock DESC NULLS LAST LIMIT 1;
  IF (current_process IS NULL OR CHAR_LENGTH(current_process) = 0 OR (request_write_lock = FALSE AND current_write_lock = FALSE)) THEN
    INSERT INTO locked (process, source, location, write_lock) VALUES (new_process, new_source, new_location, request_write_lock) RETURNING id INTO locked_id;
    RETURN locked_id;
  ELSE
    RAISE EXCEPTION 'Database is locked by another id {%}, process {%}, source {%}, location {%}', current_id, current_process, current_source, current_location;
  END IF;
END;
$$;


ALTER FUNCTION public.lock_database(new_process text, new_source text, new_location text, request_write_lock boolean) OWNER TO rob;

--
-- Name: osmosisupdate(); Type: FUNCTION; Schema: public; Owner: rob
--

CREATE FUNCTION public.osmosisupdate() RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
END;
$$;


ALTER FUNCTION public.osmosisupdate() OWNER TO rob;

--
-- Name: unlock_database(integer); Type: FUNCTION; Schema: public; Owner: rob
--

CREATE FUNCTION public.unlock_database(locked_id integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
  DECLARE response BOOLEAN;
  DECLARE exist_count INT;
BEGIN
  IF (locked_id = -1) THEN
    DELETE FROM locked;
    RETURN true;
  ELSE
    SELECT COUNT(*) INTO exist_count FROM locked WHERE id = locked_id;
    IF (exist_count = 1) THEN
      DELETE FROM locked WHERE id = locked_id;
      RETURN true;
    ELSE
      RETURN false;
    END IF;
  END IF;
END;
$$;


ALTER FUNCTION public.unlock_database(locked_id integer) OWNER TO rob;

--
-- Name: unnest_bbox_way_nodes(); Type: FUNCTION; Schema: public; Owner: rob
--

CREATE FUNCTION public.unnest_bbox_way_nodes() RETURNS void
    LANGUAGE plpgsql
    AS $$
DECLARE
	previousId ways.id%TYPE;
	currentId ways.id%TYPE;
	result bigint[];
	wayNodeRow way_nodes%ROWTYPE;
	wayNodes ways.nodes%TYPE;
BEGIN
	FOR wayNodes IN SELECT bw.nodes FROM bbox_ways bw LOOP
		FOR i IN 1 .. array_upper(wayNodes, 1) LOOP
			INSERT INTO bbox_way_nodes (id) VALUES (wayNodes[i]);
		END LOOP;
	END LOOP;
END;
$$;


ALTER FUNCTION public.unnest_bbox_way_nodes() OWNER TO rob;

--
-- Name: first(anyelement); Type: AGGREGATE; Schema: public; Owner: rob
--

CREATE AGGREGATE public.first(anyelement) (
    SFUNC = public.first_agg,
    STYPE = anyelement
);


ALTER AGGREGATE public.first(anyelement) OWNER TO rob;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: actions; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.actions (
    data_type character(1) NOT NULL,
    action character(1) NOT NULL,
    id bigint NOT NULL
);


ALTER TABLE public.actions OWNER TO rob;

--
-- Name: locked; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.locked (
    id integer NOT NULL,
    started timestamp without time zone DEFAULT now() NOT NULL,
    process text NOT NULL,
    source text NOT NULL,
    location text NOT NULL,
    write_lock boolean DEFAULT false NOT NULL
);


ALTER TABLE public.locked OWNER TO rob;

--
-- Name: locked_id_seq; Type: SEQUENCE; Schema: public; Owner: rob
--

CREATE SEQUENCE public.locked_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.locked_id_seq OWNER TO rob;

--
-- Name: locked_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: rob
--

ALTER SEQUENCE public.locked_id_seq OWNED BY public.locked.id;


--
-- Name: nodes; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.nodes (
    id bigint NOT NULL,
    version integer NOT NULL,
    user_id integer NOT NULL,
    tstamp timestamp without time zone NOT NULL,
    changeset_id bigint NOT NULL,
    tags public.hstore,
    geom public.geometry(Point,4326)
);


ALTER TABLE public.nodes OWNER TO rob;

--
-- Name: relation_members; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.relation_members (
    relation_id bigint NOT NULL,
    member_id bigint NOT NULL,
    member_type character(1) NOT NULL,
    member_role text NOT NULL,
    sequence_id integer NOT NULL
);
ALTER TABLE ONLY public.relation_members ALTER COLUMN relation_id SET (n_distinct=-0.09);
ALTER TABLE ONLY public.relation_members ALTER COLUMN member_id SET (n_distinct=-0.62);
ALTER TABLE ONLY public.relation_members ALTER COLUMN member_role SET (n_distinct=6500);
ALTER TABLE ONLY public.relation_members ALTER COLUMN sequence_id SET (n_distinct=10000);


ALTER TABLE public.relation_members OWNER TO rob;

--
-- Name: relations; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.relations (
    id bigint NOT NULL,
    version integer NOT NULL,
    user_id integer NOT NULL,
    tstamp timestamp without time zone NOT NULL,
    changeset_id bigint NOT NULL,
    tags public.hstore
);


ALTER TABLE public.relations OWNER TO rob;

--
-- Name: replication_changes; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.replication_changes (
    id integer NOT NULL,
    tstamp timestamp without time zone DEFAULT now() NOT NULL,
    nodes_modified integer DEFAULT 0 NOT NULL,
    nodes_added integer DEFAULT 0 NOT NULL,
    nodes_deleted integer DEFAULT 0 NOT NULL,
    ways_modified integer DEFAULT 0 NOT NULL,
    ways_added integer DEFAULT 0 NOT NULL,
    ways_deleted integer DEFAULT 0 NOT NULL,
    relations_modified integer DEFAULT 0 NOT NULL,
    relations_added integer DEFAULT 0 NOT NULL,
    relations_deleted integer DEFAULT 0 NOT NULL,
    changesets_applied bigint[] NOT NULL,
    earliest_timestamp timestamp without time zone NOT NULL,
    latest_timestamp timestamp without time zone NOT NULL
);


ALTER TABLE public.replication_changes OWNER TO rob;

--
-- Name: replication_changes_id_seq; Type: SEQUENCE; Schema: public; Owner: rob
--

CREATE SEQUENCE public.replication_changes_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.replication_changes_id_seq OWNER TO rob;

--
-- Name: replication_changes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: rob
--

ALTER SEQUENCE public.replication_changes_id_seq OWNED BY public.replication_changes.id;


--
-- Name: schema_info; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.schema_info (
    version integer NOT NULL
);


ALTER TABLE public.schema_info OWNER TO rob;

--
-- Name: sql_changes; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.sql_changes (
    id integer NOT NULL,
    tstamp timestamp without time zone DEFAULT now() NOT NULL,
    entity_id bigint NOT NULL,
    type text NOT NULL,
    changeset_id bigint NOT NULL,
    change_time timestamp without time zone NOT NULL,
    action integer NOT NULL,
    query text NOT NULL,
    arguments text
);


ALTER TABLE public.sql_changes OWNER TO rob;

--
-- Name: sql_changes_id_seq; Type: SEQUENCE; Schema: public; Owner: rob
--

CREATE SEQUENCE public.sql_changes_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.sql_changes_id_seq OWNER TO rob;

--
-- Name: sql_changes_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: rob
--

ALTER SEQUENCE public.sql_changes_id_seq OWNED BY public.sql_changes.id;


--
-- Name: state; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.state (
    id integer NOT NULL,
    tstamp timestamp without time zone DEFAULT now() NOT NULL,
    sequence_number bigint NOT NULL,
    state_timestamp timestamp without time zone NOT NULL,
    disabled boolean DEFAULT false NOT NULL
);


ALTER TABLE public.state OWNER TO rob;

--
-- Name: state_id_seq; Type: SEQUENCE; Schema: public; Owner: rob
--

CREATE SEQUENCE public.state_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.state_id_seq OWNER TO rob;

--
-- Name: state_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: rob
--

ALTER SEQUENCE public.state_id_seq OWNED BY public.state.id;


--
-- Name: users; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.users (
    id integer NOT NULL,
    name text NOT NULL
);


ALTER TABLE public.users OWNER TO rob;

--
-- Name: way_nodes; Type: TABLE; Schema: public; Owner: rob
--

CREATE TABLE public.way_nodes (
    way_id bigint NOT NULL,
    node_id bigint NOT NULL,
    sequence_id integer NOT NULL
);
ALTER TABLE ONLY public.way_nodes ALTER COLUMN way_id SET (n_distinct=-0.08);
ALTER TABLE ONLY public.way_nodes ALTER COLUMN node_id SET (n_distinct=-0.83);
ALTER TABLE ONLY public.way_nodes ALTER COLUMN sequence_id SET (n_distinct=2000);


ALTER TABLE public.way_nodes OWNER TO rob;

--
-- Name: ways; Type: TABLE; Schema: public; Owner: rob
--

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


ALTER TABLE public.ways OWNER TO rob;

--
-- Name: locked id; Type: DEFAULT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.locked ALTER COLUMN id SET DEFAULT nextval('public.locked_id_seq'::regclass);


--
-- Name: replication_changes id; Type: DEFAULT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.replication_changes ALTER COLUMN id SET DEFAULT nextval('public.replication_changes_id_seq'::regclass);


--
-- Name: sql_changes id; Type: DEFAULT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.sql_changes ALTER COLUMN id SET DEFAULT nextval('public.sql_changes_id_seq'::regclass);


--
-- Name: state id; Type: DEFAULT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.state ALTER COLUMN id SET DEFAULT nextval('public.state_id_seq'::regclass);


--
-- Name: actions pk_actions; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.actions
    ADD CONSTRAINT pk_actions PRIMARY KEY (data_type, id);


--
-- Name: nodes pk_nodes; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.nodes
    ADD CONSTRAINT pk_nodes PRIMARY KEY (id);


--
-- Name: relation_members pk_relation_members; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.relation_members
    ADD CONSTRAINT pk_relation_members PRIMARY KEY (relation_id, sequence_id);


--
-- Name: relations pk_relations; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.relations
    ADD CONSTRAINT pk_relations PRIMARY KEY (id);


--
-- Name: schema_info pk_schema_info; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.schema_info
    ADD CONSTRAINT pk_schema_info PRIMARY KEY (version);


--
-- Name: users pk_users; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT pk_users PRIMARY KEY (id);


--
-- Name: way_nodes pk_way_nodes; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.way_nodes
    ADD CONSTRAINT pk_way_nodes PRIMARY KEY (way_id, sequence_id);


--
-- Name: ways pk_ways; Type: CONSTRAINT; Schema: public; Owner: rob
--

ALTER TABLE ONLY public.ways
    ADD CONSTRAINT pk_ways PRIMARY KEY (id);


--
-- Name: idx_nodes_geom; Type: INDEX; Schema: public; Owner: rob
--

CREATE INDEX idx_nodes_geom ON public.nodes USING gist (geom);


--
-- Name: idx_relation_members_member_id_and_type; Type: INDEX; Schema: public; Owner: rob
--

CREATE INDEX idx_relation_members_member_id_and_type ON public.relation_members USING btree (member_id, member_type);


--
-- Name: idx_way_nodes_node_id; Type: INDEX; Schema: public; Owner: rob
--

CREATE INDEX idx_way_nodes_node_id ON public.way_nodes USING btree (node_id);


--
-- Name: idx_ways_bbox; Type: INDEX; Schema: public; Owner: rob
--

CREATE INDEX idx_ways_bbox ON public.ways USING gist (bbox);


--
-- Name: idx_ways_linestring; Type: INDEX; Schema: public; Owner: rob
--

CREATE INDEX idx_ways_linestring ON public.ways USING gist (linestring);


--
-- PostgreSQL database dump complete
--

