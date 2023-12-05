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
RESULTS_PER_PAGE_LIST = 10

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
        status = None,
        table = None):

        geoType = getGeoType(table)
        query = "with t_ways AS ( \
            SELECT '" + geoType + "' as type, " + table + ".osm_id as id, " + table + ".timestamp, geom as geometry, tags, status, hashtags, editor, created_at FROM " + table + " \
            LEFT JOIN validation ON validation.osm_id = " + table + ".osm_id \
            LEFT JOIN changesets c ON c.id = " + table + ".changeset \
            WHERE \
            {0} {1} {2} {3} {4} {5} \
        ), \
        t_features AS (  \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(t_ways) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM t_ways  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
            "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
            "AND created at >= {0} AND created_at <= {1}".format(dateFrom, dateTo) if dateFrom and dateTo else "",
            "AND status = '{0}'".format(status) if (status) else "",
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
        status = None,
        table = None,
    ):

        geoType = getGeoType(table)
        if table == "nodes":
            osmType = "node"
        else:
            osmType = "way"

        query = "\
            SELECT '" + osmType + "' as type, '" + geoType + "' as geotype, " + table + ".osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, " + table + ".timestamp, tags, status, created_at FROM " + table + " \
            LEFT JOIN validation ON validation.osm_id = " + table + ".osm_id \
            LEFT JOIN changesets c ON c.id = " + table + ".changeset \
            WHERE \
            {0} {1} {2} {3} {4} \
        ".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "1=1 ",
            "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
            "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
            "AND created_at >= '{0}' AND created_at <= '{1}'".format(dateFrom, dateTo) if (dateFrom and dateTo) else "",
            "AND status = '{0}'".format(status) if (status) else "",
        )
        return query

def queryToJSON(query, orderBy, page):
    return "with data AS (" + query + ") , t_features AS ( \
            SELECT to_jsonb(data) as feature from data {0} \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;" \
        .format(
            "WHERE " + orderBy + " IS NOT NULL ORDER BY " + orderBy + " DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + (" OFFSET {0}" \
            .format(page * RESULTS_PER_PAGE_LIST) if page else ""),
        )

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
        status = None,
        page = None
    ):
        return self.underpassDB.run(geoFeaturesQuery(
            area,
            tags,
            hashtag,
            dateFrom,
            dateTo,
            page,
            status,
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
        status = None,
        page = None
    ):
        return self.underpassDB.run(geoFeaturesQuery(
            area,
            tags,
            hashtag,
            dateFrom,
            dateTo,
            page,
            status,
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
        status = None,
        page = None
    ):
        return self.underpassDB.run(geoFeaturesQuery(
            area,
            tags,
            hashtag,
            dateFrom,
            dateTo,
            page,
            status,
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
        status = None,
        page = None
    ):

        polygons = self.getPolygons( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        status,
        page)

        lines = self.getLines( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        status,
        page)

        nodes = self.getNodes( 
        area,
        tags,
        hashtag,
        responseType,
        dateFrom,
        dateTo,
        status,
        page)

        result = {'type': 'FeatureCollection', 'features': []}

        if polygons and polygons['features']:
            result['features'] = result['features'] + polygons['features']

        if lines and lines['features']:
            result['features'] = result['features'] + lines['features']

        elif nodes and nodes['features']:
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
        status = None,
        page = None
    ):
        return self.underpassDB.run(listFeaturesQuery(
            area,
            tags,
            hashtag,
            page,
            dateFrom,
            dateTo,
            status,
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
        status = None,
        page = None
    ):
        return self.underpassDB.run(listFeaturesQuery(
            area,
            tags,
            hashtag,
            page,
            dateFrom,
            dateTo,
            status,
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
        status = None,
        page = None
    ):
        return self.underpassDB.run(listFeaturesQuery(
            area,
            tags,
            hashtag,
            page,
            dateFrom,
            dateTo,
            status,
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
        status = None,
        orderBy = None,
        page = None
    ):

        queryPolygons = listFeaturesQuery(
        area,
        tags,
        hashtag,
        page,
        dateFrom,
        dateTo,
        status,
        "ways_poly")

        queryLines = listFeaturesQuery(
        area,
        tags,
        hashtag,
        page,
        dateFrom,
        dateTo,
        status,
        "ways_line")

        queryNodes = listFeaturesQuery(
        area,
        tags,
        hashtag,
        page,
        dateFrom,
        dateTo,
        status,
        "nodes")

        query = queryToJSON(
            " UNION ".join([queryPolygons, queryLines, queryNodes]),
            orderBy or "id",
            page or 0,
        )

        return self.underpassDB.run(query, responseType, True)
