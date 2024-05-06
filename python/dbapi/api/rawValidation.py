
#!/usr/bin/python3
#
# Copyright (c) 2024 Humanitarian OpenStreetMap Team
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

# Build and run queries for getting validation results
# from the Underpass validation table in combination with raw OSM data
#
# This file requires to have both OSM Raw Data and Underpass tables
# into the same database.

from dataclasses import dataclass
from enum import Enum
from .sharedTypes import Table, GeoType
from .filters import tagsQueryFilter, hashtagQueryFilter
from .serialization import queryToJSON
from .config import RESULTS_PER_PAGE, RESULTS_PER_PAGE_LIST, DEBUG
from .raw import RawFeaturesParamsDTO, ListFeaturesParamsDTO, rawQueryToJSON, listQueryToJSON
from .serialization import deserializeTags
import json


# Validation errorrs
class ValidationError(Enum):
    notags = "notags"
    complete = "complete"
    incomplete = "incomplete"
    badvalue = "badvalue"
    correct = "correct"
    badgeom = "badgeom"
    orphan = "orphan"
    overlapping = "overlapping"
    duplicate = "duplicate"
    valid = "valid"

# OSM types
class OsmType(Enum):
    nodes = "node"
    lines = "way"
    polygons = "way"

# Validation Count parameters DTO
@dataclass
class ValidationCountParamsDTO:
    status: ValidationError
    tags: list[str] = None
    hashtag: str = ""
    dateFrom: str = ""
    dateTo: str = ""
    area: str = None
    table: Table = Table.nodes

@dataclass
class RawValidationFeaturesParamsDTO(RawFeaturesParamsDTO):
    status: ValidationError = None

@dataclass
class ListValidationFeaturesParamsDTO(ListFeaturesParamsDTO):
    status: ValidationError = None

# Build queries for counting validation data
def countQuery(
    params: ValidationCountParamsDTO,
    asJson: bool = False):

    query = "with all_features as ( \
            select {table}.osm_id from {table} \
            left join changesets c on changeset = c.id \
            WHERE{dateFrom}{dateTo}{area}{tags}{hashtag} \
        ), \
        count_validated_features as ( \
            select count(distinct(all_features.osm_id)) as count from all_features \
            left join validation v on all_features.osm_id = v.osm_id \
            where v.status = '{status}' \
        ), count_features as (\
            select count(distinct(all_features.osm_id)) as total from all_features \
        ) \
        select count, total from  count_validated_features, count_features".format(
            table=params.table.value,
            dateFrom=" AND created_at >= '{dateFrom}'".format(dateFrom=params.dateFrom) if (params.dateFrom) else "",
            dateTo=" AND created_at <= '{dateTo}'".format(dateTo=params.dateTo) if (params.dateTo) else "",
            area=" AND ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({area})))', 4326) )".format(area=params.area) if params.area else "",
            tags=" AND (" + tagsQueryFilter(params.tags, params.table.value) + ") \n" if params.tags else "",
            hashtag=" AND " + hashtagQueryFilter(params.hashtag, params.table.value) if params.hashtag else "",
            status=params.status.value
        ).replace("WHERE AND", "WHERE")

    if asJson:
        return queryToJSON(query)
    return query

# Build queries for getting geometry features
def geoFeaturesQuery(params: RawValidationFeaturesParamsDTO, asJson: bool = False):
    geoType:GeoType = GeoType[params.table.name]
    query = "SELECT '{type}' as type, \
            {table}.osm_id as id, \n \
            {table}.timestamp, \n \
            ST_AsText(geom) as geometry, \n \
            tags, \n \
            status, \n \
            hashtags, \n \
            editor, \n \
            created_at \n \
            FROM {table} \n \
            LEFT JOIN changesets c ON c.id = {table}.changeset \n \
            LEFT JOIN validation ON validation.osm_id = {table}.osm_id \
            WHERE{area}{tags}{hashtag}{date}{status} {limit}; \n \
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
            status=" AND status = '{status}'".format(status=params.status.value) if (params.status) else "",
            limit=" LIMIT {limit}".format(limit=RESULTS_PER_PAGE),
        ).replace("WHERE AND", "WHERE")
    if asJson:
        return rawQueryToJSON(query, params)

    return query


# Build queries for getting list of features
def listFeaturesQuery(
        params: ListValidationFeaturesParamsDTO,
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
            c.created_at, \n \
            status \n \
            FROM {table} \n \
            LEFT JOIN changesets c ON c.id = {table}.changeset \n \
            LEFT JOIN validation v ON v.osm_id = {table}.osm_id \n \
            WHERE{fromDate}{toDate}{hashtag}{area}{tags}{status}{order} \
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
        status=" AND status = '{status}'".format(status=params.status.value) if (params.status) else "",
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

# This class build and run queries for Validation Data
class RawValidation:
    def __init__(self,db):
        self.db = db

    # Get count of validation errors
    async def getNodesCount(
        self, 
        params: ValidationCountParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.nodes
        query = countQuery(params,asJson=asJson)
        return(await self.db.run(query, asJson=asJson, singleObject=True))

    async def getLinesCount(
        self, 
        params: ValidationCountParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.lines
        query = countQuery(params,asJson=asJson)
        return(await self.db.run(query, asJson=asJson, singleObject=True))

    async def getPolygonsCount(
        self, 
        params: ValidationCountParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.polygons
        query = countQuery(params,asJson=asJson)
        return(await self.db.run(query, asJson=asJson, singleObject=True))

    async def getCount(
        self, 
        params: ValidationCountParamsDTO,
        asJson: bool = False
    ):

        params.table = Table.nodes
        queryNodes = countQuery(params)
        nodesCount = await self.db.run(queryNodes, singleObject=True)

        params.table = Table.lines
        queryLines = countQuery(params)
        linesCount = await self.db.run(queryLines, singleObject=True)

        params.table = Table.polygons
        queryPolygons = countQuery(params)
        polygonsCount = await self.db.run(queryPolygons, singleObject=True)

        result = {
            "total": nodesCount['total'] + linesCount['total'] + + polygonsCount['total'],
            "count":  nodesCount['count'] + linesCount['count'] + + polygonsCount['count']
        }

        return(result)

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
        params: RawValidationFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.polygons
        result = await self.db.run(geoFeaturesQuery(params, asJson), asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)


    # Get line features
    async def getLines(
        self,
        params: RawValidationFeaturesParamsDTO,
       asJson: bool = False
    ):
        params.table = Table.lines
        result = await self.db.run(geoFeaturesQuery(params, asJson), asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)

    # Get node features
    async def getNodes(
        self,
        params: RawValidationFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.nodes
        result = await self.db.run(geoFeaturesQuery(params, asJson), asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)

    # Get all (polygon, line, node) features
    async def getAll(
        self,
        params: RawValidationFeaturesParamsDTO,
        asJson: bool = False
    ):
        if asJson:

            polygons = json.loads(await self.getPolygons(params, asJson))
            lines = json.loads(await self.getLines(params, asJson))
            nodes = json.loads(await self.getNodes(params, asJson))

            jsonResult = {'type': 'FeatureCollection', 'features': []}

            if polygons and "features" in polygons and polygons['features']:
                jsonResult['features'] = jsonResult['features'] + polygons['features']

            if lines and "features" in lines and lines['features']:
                jsonResult['features'] = jsonResult['features'] + lines['features']

            elif nodes and "features" in nodes and nodes['features']:
                jsonResult['features'] = jsonResult['features'] + nodes['features']
        
            # elif relations and "features" in relations and relations['features']:
            #     result['features'] = result['features'] + relations['features']

            result = json.dumps(jsonResult)

        else:
            polygons = await self.getPolygons(params, asJson)
            lines = await self.getLines(params, asJson)
            nodes = await self.getNodes(params, asJson)
            result = polygons + lines + nodes
            
        return result
    
    # Get a list of line features
    async def getLinesList(
        self,
        params: ListValidationFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = "lines"
        result = await self.db.run(listFeaturesQuery(params, asJson), asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)

    # Get a list of node features
    async def getNodesList(
        self,
        params: ListValidationFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = "nodes"
        result = await self.db.run(listFeaturesQuery(params, asJson), asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)

    # Get a list of polygon features
    async def getPolygonsList(
        self,
        params: ListValidationFeaturesParamsDTO,
        asJson: bool = False
    ):
        params.table = "polygons"
        result = await self.db.run(listFeaturesQuery(params, asJson), asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)

    # Get a list of all features
    async def getList(
        self,
        params: ListValidationFeaturesParamsDTO,
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

        result = await self.db.run(query, asJson=asJson)
        if asJson:
            return result
        return deserializeTags(result)
