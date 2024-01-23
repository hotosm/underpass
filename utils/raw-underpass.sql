CREATE UNIQUE INDEX nodes_id_idx ON public.nodes (osm_id DESC);
CREATE UNIQUE INDEX ways_poly_id_idx ON public.ways_poly (osm_id DESC);
CREATE UNIQUE INDEX ways_line_id_idx ON public.ways_line(osm_id DESC);
CREATE INDEX way_refs_node_id_idx ON public.way_refs (node_id);
CREATE INDEX way_refs_way_id_idx ON public.way_refs (way_id);

CREATE INDEX nodes_version_idx ON public.nodes (version);
CREATE INDEX ways_poly_version_idx ON public.ways_poly (version);
CREATE INDEX ways_line_version_idx ON public.ways_line (version);

CREATE INDEX nodes_timestamp_idx ON public.nodes(timestamp DESC);
CREATE INDEX ways_poly_timestamp_idx ON public.ways_poly(timestamp DESC);
CREATE INDEX ways_line_timestamp_idx ON public.ways_line(timestamp DESC);

CREATE INDEX idx_changesets_hashtags ON public.changesets USING gin(hashtags);
CREATE INDEX idx_osm_id_status ON public.validation (osm_id)

