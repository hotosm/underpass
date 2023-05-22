CREATE UNIQUE INDEX raw_osm_id_idx ON public.raw_node (osm_id);
CREATE UNIQUE INDEX raw_poly_osm_id_idx ON public.raw_poly (osm_id);
CREATE INDEX raw_node_timestamp_idx ON public.raw_node ("timestamp" DESC);
CREATE INDEX raw_poly_timestamp_idx ON public.raw_poly ("timestamp" DESC);
CREATE INDEX idx_poly_ref on public.raw_poly USING GIN ("refs");
