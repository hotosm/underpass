CREATE UNIQUE INDEX raw_osm_id_idx ON public.raw (osm_id, osm_type);
CREATE INDEX raw_timestamp_idx ON public.raw ("timestamp" DESC);

