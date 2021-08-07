-- Create osmstats DB and tables, the root path from inside the container is /code/


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

DROP DATABASE IF EXISTS osmstats;

CREATE DATABASE osmstats WITH TEMPLATE = template0 ENCODING = 'UTF8' LC_COLLATE = 'en_US.UTF-8' LC_CTYPE = 'en_US.UTF-8';

\connect osmstats
\i /code/data/osmstats.sql
-- not used for now
-- \i /code/data/pgsnapshot.sql
-- will eventually disappear
-- \i /code/data/geoboundaries.sql

