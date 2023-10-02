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

RESULTS_PER_PAGE = 500
RESULTS_PER_PAGE_LIST = 100

# TODO: improve this code
def tagsQueryFilter(tagsQuery, table):
    query = ""
    tags = tagsQuery.split(",")
    keyValue = tags[0].split("=")

    if len(keyValue) == 2:
        query += "{0}.tags->'{1}' ~* '^{2}'".format(table, keyValue[0], keyValue[1])
    else:
        query += "{0}.tags ? '{1}'".format(table, keyValue[0])

    for tag in tags[1:]:
        keyValue = tag.split("=")
        if len(keyValue) == 2:
            query += "OR {0}.tags->'{!}' ~* '^{2}'".format(table, keyValue[0], keyValue[1])
        else:
            query += "OR {0}.tags ? '{1}'".format(table, keyValue[0])
    return query

class Raw:
    def __init__(self, db):
        self.underpassDB = db

    def getPolygons(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        query = "with t_ways AS ( \
            SELECT 'Polygon' as type, ways_poly.osm_id as id, ways_poly.timestamp, geom as geometry, tags, status FROM ways_poly \
            LEFT JOIN validation ON validation.osm_id = ways_poly.osm_id \
            WHERE \
            {0} {1} {2} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_ways) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, "ways_poly") + ")" if tags else "",
            "ORDER BY ways_poly.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE),
        )
        return self.underpassDB.run(query, responseType, True)

    def getLines(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        query = "with t_ways AS ( \
            SELECT 'LineString' as type, ways_line.osm_id as id, ways_line.timestamp, geom as geometry, tags, status FROM ways_line \
            LEFT JOIN validation ON validation.osm_id = ways_line.osm_id \
            WHERE \
            {0} {1} {2} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_ways) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, "ways_line") + ")" if tags else "",
            "ORDER BY ways_line.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE),
        )
        return self.underpassDB.run(query, responseType, True)

    def getNodes(
        self,
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        query = "with t_nodes AS ( \
            SELECT 'Point' as type, nodes.osm_id as id, nodes.timestamp, geom as geometry, tags, status FROM nodes \
            LEFT JOIN validation ON validation.osm_id = nodes.osm_id \
            WHERE \
            {0} {1} {2} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_nodes) \
            - 'geometry' - 'osm_id' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_nodes  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, "nodes") + ")" if tags else "",
            "ORDER BY nodes.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE),
        )
        return self.underpassDB.run(query, responseType, True)

    def getAll(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):

        polygons = self.getPolygons( 
        area,
        tags,
        hashtag,
        responseType,
        page)

        lines = self.getLines( 
        area,
        tags,
        hashtag,
        responseType,
        page)

        nodes = self.getNodes( 
        area,
        tags,
        hashtag,
        responseType,
        page)

        result = {'type': 'FeatureCollection', 'features': []}

        if 'features' in polygons and polygons['features']:
            result['features'] = result['features'] + polygons['features']

        if 'features' in lines and lines['features']:
            result['features'] = result['features'] + lines['features']

        elif 'features' in nodes and nodes['features']:
            result['features'] = result['features'] + nodes['features']
            
        return result

    def getPolygonsList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):

        query = "with t_ways AS ( \
            SELECT 'way' as type, 'Polygon' as geotype, ways_poly.osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, ways_poly.timestamp, tags, status FROM ways_poly \
            LEFT JOIN validation ON validation.osm_id = ways_poly.osm_id \
            WHERE \
            {0} {1} {2} \
        ), t_features AS ( \
            SELECT to_jsonb(t_ways) as feature from t_ways \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, "ways_poly") + ")" if tags else "",
            "ORDER BY ways_poly.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + " OFFSET {0}".format(page * RESULTS_PER_PAGE_LIST) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getLinesList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        if page == 0:
            page = 1

        query = "with t_lines AS ( \
            SELECT 'way' as type, 'LineString' as geotype, ways_line.osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, ways_line.timestamp, tags, status FROM ways_line \
            LEFT JOIN validation ON validation.osm_id = ways_line.osm_id \
            WHERE \
            {0} {1} {2} \
        ), t_features AS ( \
            SELECT to_jsonb(t_lines) as feature from t_lines \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, "ways_line") + ")" if tags else "",
            "ORDER BY ways_line.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + " OFFSET {0}".format(page * RESULTS_PER_PAGE_LIST) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)

    def getNodesList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):
        if page == 0:
            page = 1

        query = "with t_nodes AS ( \
            SELECT 'node' as type, 'Point' as geotype, nodes.osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, nodes.timestamp, tags, status FROM nodes \
            LEFT JOIN validation ON validation.osm_id = nodes.osm_id \
            WHERE {0} {1} {2} \
        ), \
        t_features AS (  \
            SELECT to_jsonb(t_nodes) AS feature FROM t_nodes  \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, "nodes") + ")" if tags else "",
            "ORDER BY nodes.timestamp DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + " OFFSET {0}".format(page * RESULTS_PER_PAGE_LIST) if page else "",
        )
        return self.underpassDB.run(query, responseType, True)
        
    def getAllList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        page = None
    ):

        polygons = self.getPolygonsList( 
        area,
        tags,
        hashtag,
        responseType,
        page)

        lines = self.getLinesList( 
        area,
        tags,
        hashtag,
        responseType,
        page)

        nodes = self.getNodesList( 
        area,
        tags,
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