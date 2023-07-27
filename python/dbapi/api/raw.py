#!/usr/bin/python3
#
# Copyright (c) 2023 Humanitarian OpenStreetMap Team
#
# This file is part of Underpass.
#
#     Underpass is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Underpass is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.

from .db import UnderpassDB

class Raw:
    def __init__(self):
        pass

    underpassDB = UnderpassDB()

    def getPolygons(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json"
    ):
        print(key, value)
        query = "with t_ways AS ( \
            SELECT raw_poly.osm_id, geometry, tags, status FROM raw_poly \
            LEFT JOIN validation ON validation.osm_id = raw_poly.osm_id \
            WHERE \
            ST_Intersects(\"geometry\", \
            ST_GeomFromText('POLYGON(({0}))', 4326) \
            ) {1} {2} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', t_ways.osm_id, 'properties', to_jsonb(t_ways) - 'geometry' - 'osm_id' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) as result FROM t_features;".format(
            area,
            "and raw_poly.tags ? '{0}'".format(key) if key and not value else "",
            "and raw_poly.tags->'{0}' LIKE '%{1}%'".format(key, value) if key and value else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getNodes(
        self,
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json"
    ):
        query = "with t_nodes AS ( \
            SELECT raw_node.osm_id, geometry, tags, status FROM raw_node \
            LEFT JOIN validation ON validation.osm_id = raw_node.osm_id \
            WHERE \
            ST_Intersects(\"geometry\", \
            ST_GeomFromText('POLYGON(({0}))', 4326) \
            ) {1} {2} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', t_nodes.osm_id, 'properties', to_jsonb(t_nodes) - 'geometry' - 'osm_id' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_nodes  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) as result FROM t_features;".format(
            area,
            "and raw_node.tags ? '{0}'".format(key) if key and not value else "",
            "and raw_node.tags->'{0}' LIKE '%{1}%'".format(key, value) if key and value else "",
        )
        return self.underpassDB.run(query, responseType, True)

