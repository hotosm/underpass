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

from dataclasses import dataclass
from .filters import tagsQueryFilter, hashtagQueryFilter
from enum import Enum
from .config import RESULTS_PER_PAGE, RESULTS_PER_PAGE_LIST, DEBUG

# This file build and run queries for getting geometry features
# (Points, LinesStrings, Polygons) from the Raw OSM Data DB

# Order by
class OrderBy(Enum):
    createdAt = "created_at"
    id = "id"
    timestamp = "timestamp"

# DB table names
class Table(Enum):
    nodes = "nodes"
    lines = "ways_line"
    polygons = "ways_poly"
    relations = "relations"

# Geometry types
class GeoType(Enum):
    polygons = "Polygon"
    lines = "LineString"
    nodes = "Node"

# OSM types
class OsmType(Enum):
    nodes = "node"
    lines = "way"
    polygons = "way"

# Raw Features Query DTO
@dataclass
class RawFeaturesParamsDTO:
    area: str = None
    tags: list[str] = None
    hashtag: str = ""
    dateFrom: str = ""
    dateTo: str = ""
    table: Table = Table.nodes

# List Features Query DTO
@dataclass
class ListFeaturesParamsDTO(RawFeaturesParamsDTO):
    orderBy: OrderBy = OrderBy.id
    page: int = 0

# Build queries for getting geometry features
def geoFeaturesQuery(params: RawFeaturesParamsDTO, asJson: bool = False):
    geoType:GeoType = GeoType[params.table.name]
    query = "SELECT '{type}' as type, \
            osm_id as id, \n \
            timestamp, \n \
            ST_AsText(geom) as geometry, \n \
            tags, \n \
            hashtags, \n \
            editor, \n \
            created_at \n \
            FROM {table} \n \
            LEFT JOIN changesets c ON c.id = {table}.changeset \n \
            WHERE{area}{tags}{hashtag}{date} {limit}; \n \
        ".format(
            type=geoType.value,
            table=params.table.value,
            area=" AND ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({area})))', 4326) ) \n"
                .format(area=params.area) if params.area else "",
            tags=" AND (" + tagsQueryFilter(params.tags, params.table.value) + ") \n" if params.tags else "",
            hashtag=" AND " + hashtagQueryFilter(params.hashtag, params.table.value) if params.hashtag else "",
            date=" AND created_at >= {dateFrom} AND created_at <= {dateTo}\n"
                .format(dateFrom=params.dateFrom, dateTo=params.dateTo) 
                if params.dateFrom and params.dateTo else "\n",
            limit=" LIMIT {limit}".format(limit=RESULTS_PER_PAGE)
        ).replace("WHERE AND", "WHERE")

    if asJson:
        return rawQueryToJSON(query, params)

    return query


# Build queries for getting list of features
def listFeaturesQuery(
        params: ListFeaturesParamsDTO,
        asJson: bool = False
    ):

        geoType:GeoType = GeoType[params.table]
        osmType:OsmType = OsmType[params.table]
        table:Table = Table[params.table]

        query = "( \
            SELECT '{type}' as type, \n \
                '{geotype}' as geotype, \n \
                {table}.osm_id as id, \n \
                ST_X(ST_Centroid(geom)) as lat, \n \
                ST_Y(ST_Centroid(geom)) as lon, \n \
                {table}.timestamp, \n \
                tags, \n \
                {table}.changeset, \n \
                c.created_at \n \
                FROM {table} \n \
                LEFT JOIN changesets c ON c.id = {table}.changeset \n \
                WHERE{fromDate}{toDate}{hashtag}{area}{tags}{order} \
            )\
        ".format(
            type=osmType.value,
            geotype=geoType.value,
            table=table.value,
            fromDate=" AND created_at >= '{dateFrom}'".format(dateFrom=params.dateFrom) if (params.dateFrom) else "",
            toDate=" AND created_at <= '{dateTo}'".format(dateTo=params.dateTo) if (params.dateTo) else "",
            hashtag=" AND " + hashtagQueryFilter(params.hashtag, table.value) if params.hashtag else "",
            area=" AND ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({area})))', 4326) )"
                .format(
                    area=params.area
                ) if params.area else "",
            tags=" AND (" + tagsQueryFilter(params.tags, table.value) + ")" if params.tags else "",
            order=" AND {order} IS NOT NULL ORDER BY {order} DESC LIMIT {limit} OFFSET {offset}"
                .format(
                    order=params.orderBy.value,
                    limit=RESULTS_PER_PAGE_LIST,
                    offset=params.page * RESULTS_PER_PAGE_LIST
                ) if params.page else ""
            ).replace("WHERE AND", "WHERE")
        if asJson:
            return listQueryToJSON(query, params)
        return query

# Build queries for returning a list of features as a JSON response
def listQueryToJSON(query: str, params: ListFeaturesParamsDTO):
    jsonQuery = "with predata AS \n ({query}) , \n \
        data as ( \n \
                select predata.type, \n \
                geotype, predata.id, \n \
                predata.timestamp, \n \
                tags, \n \
                predata.changeset, \n \
                predata.created_at as created_at, \n \
                lat, \n \
                lon \n \
                from predata \n \
                WHERE{date}{orderBy} \n \
            ),\n \
            t_features AS ( \n \
            SELECT to_jsonb(data) as feature from data \n \
        ) SELECT jsonb_agg(t_features.feature) as result FROM t_features;" \
        .format(
            query=query,
            date="created_at >= '{dateFrom}' AND created_at <= '{dateTo}'"
                .format(
                    dateFrom=params.dateFrom if (params.dateFrom) else "",
                    dateTo=" AND created_at <= '{dateTo}'".format(dateTo=params.dateTo) if (params.dateTo) else ""
                ) if params.dateFrom and params.dateTo else "",
            orderBy=" AND {orderBy} IS NOT NULL ORDER BY {orderBy} DESC"
            .format(
                orderBy=".".join(["predata",params.orderBy.value])
            ) if params.orderBy else "ORDER BY id DESC",
        ).replace("WHERE AND", "WHERE")
    if DEBUG:
        print(jsonQuery)
    return jsonQuery

# Build queries for returning a raw features as a JSON (GeoJSON) response
def rawQueryToJSON(query: str, params: RawFeaturesParamsDTO):
    jsonQuery = "with predata AS \n ({query}) , \n \
        t_features AS ( \
            SELECT jsonb_build_object( 'type', 'Feature', 'id', id, 'properties', to_jsonb(predata) \
            - 'geometry' , 'geometry', ST_AsGeoJSON(geometry)::jsonb ) AS feature FROM predata  \
        ) SELECT jsonb_build_object( 'type', 'FeatureCollection', 'features', jsonb_agg(t_features.feature) ) \
        as result FROM t_features;" \
        .format(
            query=query.replace(";","")
        )
    if DEBUG:
        print(jsonQuery)
    return jsonQuery

# This class build and run queries for OSM Raw Data
class Raw:
    def __init__(self,db):
        self.db = db

    # Get list of features
    def getList(
        self,
        params: ListFeaturesParamsDTO,
        featureType: GeoType = None,
        asJson: bool = False
    ):
        if featureType == GeoType.lines:
            return self.getLinesList(params, asJson=asJson)
        elif featureType == GeoType.nodes:
            return self.getNodesList(params, asJson=asJson)
        elif featureType == GeoType.polygons:
            return self.getPolygonsList(params, asJson=asJson)
        else:
            return self.getAllList(params, asJson=asJson)

    # Get geometry features (lines, nodes, polygons or all)
    async def getFeatures(
        self,
        params: RawFeaturesParamsDTO,
        featureType: GeoType = None,
        asJson: bool = False
    ):
        if featureType == "line":
            return self.getLines(params, asJson)
        elif featureType == "node":
            return self.getNodes(params, asJson)
        elif featureType == "polygon":
            return self.getPolygons(params, asJson)
        else:
            return await self.getAll(params, asJson)

    # Get polygon features
    async def getPolygons(
        self,
        params: RawFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.polygons
        return await self.db.run(geoFeaturesQuery(params, asJson), asJson=asJson)

    # Get line features
    async def getLines(
        self,
        params: RawFeaturesParamsDTO,
       asJson: bool = False
    ):
        params.table = Table.lines
        return await self.db.run(geoFeaturesQuery(params, asJson), asJson=asJson)

    # Get node features
    async def getNodes(
        self,
        params: RawFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.nodes
        return await self.db.run(geoFeaturesQuery(params, asJson), asJson=asJson)

    # Get all (polygon, line, node) features
    async def getAll(
        self,
        params: RawFeaturesParamsDTO,
        asJson: bool = False
    ):

        polygons = await self.getPolygons(params, asJson)

        print(polygons)
        
        lines = await self.getLines(params, asJson)
        nodes = await self.getNodes(params, asJson)

        if asJson:
            result = {'type': 'FeatureCollection', 'features': []}

            if polygons and "features" in polygons and polygons['features']:
                result['features'] = result['features'] + polygons['features']

            if lines and "features" in lines and lines['features']:
                result['features'] = result['features'] + lines['features']

            elif nodes and "features" in nodes and nodes['features']:
                result['features'] = result['features'] + nodes['features']
        
            # elif relations and "features" in relations and relations['features']:
            #     result['features'] = result['features'] + relations['features']

        else:
            result = [polygons, lines, nodes]
            
        return result

    # Get a list of line features
    async def getLinesList(
        self,
        params: ListFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = "lines"
        return await self.db.run(listFeaturesQuery(params, asJson), asJson=asJson)

    # Get a list of node features
    async def getNodesList(
        self,
        params: ListFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = "nodes"
        return await self.db.run(listFeaturesQuery(params, asJson), asJson=asJson)

    # Get a list of polygon features
    async def getPolygonsList(
        self,
        params: ListFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = "polygons"
        return await self.db.run(listFeaturesQuery(params, asJson), asJson=asJson)
        
    # Get a list of all features
    async def getAllList(
        self,
        params: ListFeaturesParamsDTO,
        asJson: bool = False
    ):

        params.table = "polygons"
        queryPolygons = listFeaturesQuery(params, asJson=False)
        params.table = "lines"
        queryLines = listFeaturesQuery(params, asJson=False)
        params.table = "nodes"
        queryNodes = listFeaturesQuery(params, asJson=False)

        # Combine queries for each geometry in a single query
        if asJson:
            query = listQueryToJSON(
                " UNION ".join([queryPolygons, queryLines, queryNodes]),
                params
            )
        else:
            query = " UNION ".join([queryPolygons, queryLines, queryNodes])

        return await self.db.run(query, asJson=asJson)
