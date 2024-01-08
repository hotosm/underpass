#!/usr/bin/python3
#
# Copyright (c) 2023, 2024 Humanitarian OpenStreetMap Team
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

def listAllFeaturesQuery(
        area,
        tags,
        orderBy,
        page,
        table,
    ):

        geoType = getGeoType(table)
        if table == "nodes":
            osmType = "node"
        else:
            osmType = "way"

        query = "\
            (SELECT '" + osmType + "' as type, '" + geoType + "' as geotype, " + table + ".osm_id as id, ST_X(ST_Centroid(geom)) as lat, ST_Y(ST_Centroid(geom)) as lon, " + table + ".timestamp, tags, changeset FROM " + table + " \
            WHERE {0} {1} {2})\
        ".format(
            "ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "1=1",
            "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
            "AND " + orderBy + " IS NOT NULL ORDER BY " + orderBy + " DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + (" OFFSET {0}" \
                .format(page * RESULTS_PER_PAGE_LIST) if page else ""),
        ).replace("WHERE 1=1 AND", "WHERE")
        return query

def queryToJSON(query, orderBy, page):
    return "with data AS (" + query + ") , t_features AS ( \
            SELECT to_jsonb(data) as feature from data {0} \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;" \
        .format(
            "WHERE " + orderBy + " IS NOT NULL ORDER BY " + orderBy + " DESC LIMIT " + str(RESULTS_PER_PAGE_LIST) + (" OFFSET {0}" \
            .format(page * RESULTS_PER_PAGE_LIST) if page else ""),
        )

def queryToJSONAllFeatures(query, hashtag, dateFrom, dateTo, status):
    return "with predata AS (" + query + ") , \
           data as ( \
                select predata.type, geotype, predata.id, predata.timestamp, tags, status, predata.changeset, created_at, lat, lon from predata \
                LEFT JOIN validation ON validation.osm_id = id \
                LEFT JOIN changesets c ON c.id = predata.changeset \
                WHERE {0} {1}{2} \
            ),\
            t_features AS ( \
            SELECT to_jsonb(data) as feature from data \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;" \
        .format(
            hashtagQueryFilter(hashtag, "predata") if hashtag else "1=1",
            "AND created_at >= '{0}' AND created_at <= '{1}'".format(dateFrom, dateTo) if (dateFrom and dateTo) else "",
            "AND status = '{0}'".format(status) if (status) else ""
        ).replace("WHERE 1=1 AND", "WHERE")

class Raw:
    def __init__(self,db):
        self.underpassDB = db

    def getList(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        orderBy,
        page,
        featureType
    ):
        if featureType == "line":
            return self.getLinesList(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                orderBy,
                page
            )
        elif featureType == "node":
            return self.getNodesList(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                orderBy,
                page
            )
        elif featureType == "polygon":
            return self.getPolygonsList(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                orderBy,
                page
            )
        else:
            return self.getAllList(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                orderBy,
                page
            )

    def getFeatures(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page,
        featureType
    ):
        if featureType == "line":
            return self.getLines(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                page
            )
        elif featureType == "node":
            return self.getNodes(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                page
            )
        elif featureType == "polygon":
            return self.getPolygons(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                page
            )
        else:
            return self.getAll(
                area,
                tags,
                hashtag,
                dateFrom,
                dateTo,
                status,
                page
            )

    def getPolygons(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page
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
        ))

    def getLines(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page
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
        ))


    def getNodes(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page
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
        ), True)

    def getAll(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page
    ):

        polygons = self.getPolygons( 
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page)

        lines = self.getLines( 
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page)

        nodes = self.getNodes( 
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        page)

        result = {'type': 'FeatureCollection', 'features': []}

        if polygons and "features" in polygons:
            result['features'] = result['features'] + polygons['features']

        if lines and "features" in lines:
            result['features'] = result['features'] + lines['features']

        elif nodes and "features" in nodes:
            result['features'] = result['features'] + nodes['features']
            
        return result

    def getPolygonsList(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        orderBy,
        page
    ):

        queryPolygons = listAllFeaturesQuery(
            area,
            tags,
            orderBy or "osm_id",
            page or 0,
            "ways_poly")

        query = queryToJSONAllFeatures(
            " UNION ".join([queryPolygons]),
            hashtag,
            dateFrom,
            dateTo,
            status
        )
        return self.underpassDB.run(query)

    def getLinesList(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        orderBy,
        page
    ):

        queryLines = listAllFeaturesQuery(
            area,
            tags,
            orderBy or "osm_id",
            page or 0,
            "ways_line")

        query = queryToJSONAllFeatures(
            " UNION ".join([queryLines]),
            hashtag,
            dateFrom,
            dateTo,
            status
        )
        return self.underpassDB.run(query)

    def getNodesList(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        orderBy,
        page
    ):

        queryNodes = listAllFeaturesQuery(
            area,
            tags,
            orderBy or "osm_id",
            page or 0,
            "nodes")

        query = queryToJSONAllFeatures(
            " UNION ".join([queryNodes]),
            hashtag,
            dateFrom,
            dateTo,
            status
        )
        return self.underpassDB.run(query)
        
    def getAllList(
        self,
        area,
        tags,
        hashtag,
        dateFrom,
        dateTo,
        status,
        orderBy,
        page
    ):

        queryPolygons = listAllFeaturesQuery(
        area,
        tags,
        orderBy or "osm_id",
        page or 0,
        "ways_poly")

        queryLines = listAllFeaturesQuery(
        area,
        tags,
        orderBy or "osm_id",
        page or 0,
        "ways_line")

        queryNodes = listAllFeaturesQuery(
        area,
        tags,
        orderBy or "osm_id",
        page or 0,
        "nodes")

        query = queryToJSONAllFeatures(
            " UNION ".join([queryPolygons, queryLines, queryNodes]),
            hashtag,
            dateFrom,
            dateTo,
            status
        )
        return self.underpassDB.run(query)
