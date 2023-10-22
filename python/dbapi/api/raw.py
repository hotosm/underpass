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

def tagsQueryFilter(tagsQuery, table):
    query = ""
    tags = tagsQuery.split(",")
    keyValue = tags[0].split("=")

    if len(keyValue) == 2:
        query += "{0}.tags->>'{1}' ~* '^{2}'".format(table, keyValue[0], keyValue[1])
    else:
        query += "{0}.tags->>'{1}' IS NOT NULL".format(table, keyValue[0])

    for tag in tags[1:]:
        keyValue = tag.split("=")
        if len(keyValue) == 2:
            query += "OR {0}.tags->>'{1}' ~* '^{2}'".format(table, keyValue[0], keyValue[1])
        else:
            query += "OR {0}.tags->>'{1}' IS NOT NULL".format(table, keyValue[0])
    return query

def hashtagQueryFilter(hashtag, table):
    return table + ".changeset IN (SELECT c.id FROM changesets c where jsonb_path_exists(to_jsonb(hashtags), '$[*] ? (@ like_regex \"^{0}\")') GROUP BY C.id)".format(hashtag)

def getGeoType(table):
    if table == "ways_poly":
        return "Polygon"
    elif table == "ways_line":
        return "LineString"
    return "Node"

def geoFeaturesQuery(
        area = None,
        tags = None,
        hashtag = None,
        dateFrom = None,
        dateTo = None,
        page = 0,
        table = None):

        geoType = getGeoType(table)
        query = "with t_ways AS ( \
            SELECT '" + geoType + "' as type, " + table + ".osm_id as id, " + table + ".timestamp, geom as geometry, tags, status, hashtags, editor, created_at FROM " + table + " \
            LEFT JOIN validation ON validation.osm_id = " + table + ".osm_id \
            LEFT JOIN changesets c ON c.id = " + table + ".changeset \
            WHERE \
            {0} {1} {2} {3} {4} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_ways) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
            "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
            "AND created at >= {0} AND created_at <= {1}".format(dateFrom, dateTo) if dateFrom and dateTo else "",
            "LIMIT " + str(RESULTS_PER_PAGE),
        )
        return query

def listFeaturesQuery(
        area = None,
        tags = None,
        hashtag = None,
        page = 0,
        dateFrom = None,
        dateTo = None,
        table = None,
        orderBy = "created_at"
    ):

        geoType = getGeoType(table)
        if table == "nodes":
            osmType = "node"
        else:
            osmType = "way"

        query = "with t_ways AS ( \
            SELECT '" + osmType + "' as type, '" + geoType + "' as geotype, " + table + ".osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, " + table + ".timestamp, tags, status, created_at FROM " + table + " \
            LEFT JOIN validation ON validation.osm_id = " + table + ".osm_id \
            LEFT JOIN changesets c ON c.id = " + table + ".changeset \
            WHERE \
            {0} {1} {2} {3} {4} \
        ), t_features AS ( \
            SELECT to_jsonb(t_ways) as feature from t_ways \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('POLYGON(({0}))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
            "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
            "AND created_at >= '{0}' AND created_at <= '{1}'".format(dateFrom, dateTo) if (dateFrom and dateTo) else "",
            "ORDER BY " + orderBy + " DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + (" OFFSET {0}".format(page * RESULTS_PER_PAGE_LIST) if page else ""),
        )
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
        dateFrom = None,
        dateTo = None,
        page = None
    ):
        return self.underpassDB.run(geoFeaturesQuery(
            area,
            tags,
            hashtag,
            dateFrom,
            dateTo,
            page,
            "ways_poly"
        ), responseType, True)

    def getLines(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        dateFrom = None,
        dateTo = None,
        page = None
    ):
        return self.underpassDB.run(geoFeaturesQuery(
            area,
            tags,
            hashtag,
            dateFrom,
            dateTo,
            page,
            "ways_line"
        ), responseType, True)

    def getNodes(
        self,
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        dateFrom = None,
        dateTo = None,
        page = None
    ):
        return self.underpassDB.run(geoFeaturesQuery(
            area,
            tags,
            hashtag,
            dateFrom,
            dateTo,
            page,
            "nodes"
        ), responseType, True)

    def getAll(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        dateFrom = None,
        dateTo = None,
        page = None
    ):

        polygons = self.getPolygons( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        page)

        lines = self.getLines( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        page)

        nodes = self.getNodes( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
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
        dateFrom = None,
        dateTo = None,
        page = None
    ):
        return self.underpassDB.run(listFeaturesQuery(
            area,
            tags,
            hashtag,
            page,
            dateFrom,
            dateTo,
            "ways_poly"
        ), responseType, True)

    def getLinesList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        dateFrom = None,
        dateTo = None,
        page = None
    ):
        return self.underpassDB.run(listFeaturesQuery(
            area,
            tags,
            hashtag,
            page,
            dateFrom,
            dateTo,
            "ways_line"
        ), responseType, True)


    def getNodesList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        dateFrom = None,
        dateTo = None,
        page = None
    ):
        return self.underpassDB.run(listFeaturesQuery(
            area,
            tags,
            hashtag,
            page,
            dateFrom,
            dateTo,
            "nodes"
        ), responseType, True)

        
    def getAllList(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        responseType = "json",
        dateFrom = None,
        dateTo = None,
        page = None
    ):

        polygons = self.getPolygonsList( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        page)

        lines = self.getLinesList( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        page)

        nodes = self.getNodesList( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        page)

        result = []

        if polygons:
            result = result + polygons

        if lines:
            result = result + lines

        if nodes:
            result = result + nodes
            
        return result