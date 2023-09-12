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

RESULTS_PER_PAGE = 25

class Raw:
    def __init__(self, db):
        self.underpassDB = db

    def getPolygons(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        query = "with t_ways AS ( \
            SELECT 'Polygon' as type, ways_poly.osm_id as id, ways_poly.timestamp, geom as geometry, tags, status FROM ways_poly \
            LEFT JOIN validation ON validation.osm_id = ways_poly.osm_id \
            WHERE \
            {0} {1} {2} {3} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_ways) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "and ways_poly.tags ? '{0}'".format(key) if key and not value else "",
            "and ways_poly.tags->'{0}' ~* '^{1}'".format(key, value) if key and value else "",
            "ORDER BY ways_poly.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE) + " OFFSET {0}".format(page * RESULTS_PER_PAGE) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getLines(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        query = "with t_ways AS ( \
            SELECT 'LineString' as type, ways_line.osm_id as id, ways_line.timestamp, geom as geometry, tags, status FROM ways_line \
            LEFT JOIN validation ON validation.osm_id = ways_line.osm_id \
            WHERE \
            {0} {1} {2} {3} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_ways) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "and ways_line.tags ? '{0}'".format(key) if key and not value else "",
            "and ways_line.tags->'{0}' ~* '^{1}'".format(key, value) if key and value else "",
            "ORDER BY ways_line.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE) + " OFFSET {0}".format(page * RESULTS_PER_PAGE) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getNodes(
        self,
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        query = "with t_nodes AS ( \
            SELECT 'Point' as type, nodes.osm_id as id, geom as geometry, tags, status FROM nodes \
            LEFT JOIN validation ON validation.osm_id = nodes.osm_id \
            WHERE \
            {0} {1} {2} {3} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_nodes) \
            - 'geometry' - 'osm_id' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_nodes  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "and nodes.tags ? '{0}'".format(key) if key and not value else "",
            "and nodes.tags->'{0}' ~* '^{1}'".format(key, value) if key and value else "",
            "ORDER BY nodes.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE) + " OFFSET {0}".format(page * RESULTS_PER_PAGE) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getAll(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):

        polygons = self.getPolygons( 
        area,
        key,
        value,
        hashtag,
        responseType,
        page)

        lines = self.getLines( 
        area,
        key,
        value,
        hashtag,
        responseType,
        page)

        nodes = self.getNodes( 
        area,
        key,
        value,
        hashtag,
        responseType,
        page)

        result = {'type': 'FeatureCollection', 'features': []}

        if polygons['features']:
            result['features'] = result['features'] + polygons['features']

        if lines['features']:
            result['features'] = result['features'] + lines['features']

        elif nodes['features']:
            result['features'] = result['features'] + nodes['features']
            
        return result

    def getPolygonsList(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        if page == 0:
            page = 1

        query = "with t_ways AS ( \
            SELECT 'way' as type, ways_poly.osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, ways_poly.timestamp, tags, status FROM ways_poly \
            LEFT JOIN validation ON validation.osm_id = ways_poly.osm_id \
            WHERE \
            {0} {1} {2} {3} \
        ), t_features AS ( \
            SELECT to_jsonb(t_ways) as feature from t_ways \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "and ways_poly.tags ? '{0}'".format(key) if key and not value else "",
            "and ways_poly.tags->'{0}' ~* '^{1}'".format(key, value) if key and value else "",
            "ORDER BY ways_poly.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE) + " OFFSET {0}".format(page * RESULTS_PER_PAGE) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getLinesList(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        if page == 0:
            page = 1

        query = "with t_lines AS ( \
            SELECT 'way' as type, ways_line.osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, ways_line.timestamp, tags, status FROM ways_line \
            LEFT JOIN validation ON validation.osm_id = ways_line.osm_id \
            WHERE \
            {0} {1} {2} {3} \
        ), t_features AS ( \
            SELECT to_jsonb(t_lines) as feature from t_lines \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "and ways_line.tags ? '{0}'".format(key) if key and not value else "",
            "and ways_line.tags->'{0}' ~* '^{1}'".format(key, value) if key and value else "",
            "ORDER BY ways_line.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE) + " OFFSET {0}".format(page * RESULTS_PER_PAGE) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getNodesList(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        if page == 0:
            page = 1

        query = "with t_nodes AS ( \
            SELECT 'node' as type, nodes.osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, tags, status FROM nodes \
            LEFT JOIN validation ON validation.osm_id = nodes.osm_id \
            WHERE {0} {1} {2} {3} \
        ), \
        t_features AS (  \
            SELECT to_jsonb(t_nodes) AS feature FROM t_nodes  \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "and nodes.tags ? '{0}'".format(key) if key and not value else "",
            "and nodes.tags->'{0}' ~* '^{1}'".format(key, value) if key and value else "",
            "ORDER BY nodes.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE) + " OFFSET {0}".format(page * RESULTS_PER_PAGE) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)
        
    def getAllList(
        self, 
        area = None,
        key = None,
        value = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):

        polygons = self.getPolygonsList( 
        area,
        key,
        value,
        hashtag,
        responseType,
        page)

        lines = self.getLinesList( 
        area,
        key,
        value,
        hashtag,
        responseType,
        page)

        nodes = self.getNodesList( 
        area,
        key,
        value,
        hashtag,
        responseType,
        page)

        result = []

        if polygons:
            result = result + polygons

        if lines:
            result = result + lines

        if nodes:
            result = result + nodes
            
        return result