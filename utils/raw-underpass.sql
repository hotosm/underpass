CREATE UNIQUE INDEX raw_osm_id_idx ON public.raw_node (osm_id);
CREATE UNIQUE INDEX raw_poly_osm_id_idx ON public.raw_poly (osm_id);
CREATE INDEX way_refs_node_id_idx ON public.way_refs (node_id);
CREATE INDEX way_refs_way_id_idx ON public.way_refs (way_id);

CREATE INDEX node_version_idx ON public.raw_node (version);
CREATE INDEX way_version_idx ON public.raw_poly (version);
