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

# Build and run queries for getting statistics

from dataclasses import dataclass
from .filters import tagsQueryFilter, hashtagQueryFilter
from .sharedTypes import Table, GeoType
from .serialization import queryToJSON
import json

# Stats parameters DTO
@dataclass
class StatsParamsDTO:
    tags: list[str] = None
    hashtag: str = ""
    dateFrom: str = ""
    dateTo: str = ""
    area: str = None
    table: Table = Table.nodes

def featureCountQuery(params: StatsParamsDTO, asJson: bool = False):
    query = "select count(distinct {table}.osm_id) AS count FROM {table} \
        LEFT JOIN changesets c ON changeset = c.id \
        WHERE{area}{tags}{hashtag}{date}".format(
            table=params.table.value,
            area=" AND ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({area})))', 4326) ) \n"
                .format(area=params.area) if params.area else "",
            tags=" AND (" + tagsQueryFilter(params.tags, params.table.value) + ") \n" if params.tags else "",
            hashtag=" AND " + hashtagQueryFilter(params.hashtag, params.table.value) if params.hashtag else "",
            date=" AND created_at >= {dateFrom} AND created_at <= {dateTo}\n"
                .format(dateFrom=params.dateFrom, dateTo=params.dateTo) 
                if params.dateFrom and params.dateTo else "\n"
        ).replace("WHERE AND", "WHERE")
    if asJson:
        return queryToJSON(query)
    return query

class Stats:
    def __init__(self, db):
        self.db = db

    async def getNodesCount(
        self, 
        params: StatsParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.nodes
        result = await self.db.run(featureCountQuery(params), singleObject=True)
        if asJson:
            return json.dumps(dict(result))

    async def getLinesCount(
        self, 
        params: StatsParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.lines
        result = await self.db.run(featureCountQuery(params), singleObject=True)
        if asJson:
            return json.dumps(dict(result))

    async def getPolygonsCount(
        self, 
        params: StatsParamsDTO,
        asJson: bool = False
    ):
        params.table = Table.polygons
        result = await self.db.run(featureCountQuery(params), singleObject=True)
        if asJson:
            return json.dumps(dict(result))
        return result


    async def getCount(
        self, 
        params: StatsParamsDTO,
        asJson: bool = False
    ):

        params.table = Table.nodes
        queryNodes = featureCountQuery(params)

        params.table = Table.lines
        queryLines = featureCountQuery(params)

        params.table = Table.polygons
        queryPolygons = featureCountQuery(params)

        query = "SELECT ({queries}) AS count;".format(queries=" + ".join([
            "({queryPolygons})".format(queryPolygons=queryPolygons),
            "({queryLines})".format(queryLines=queryLines),
            "({queryNodes})".format(queryNodes=queryNodes)
        ]))

        result = await self.db.run(query, singleObject=True)
        if asJson:
            return json.dumps(dict(result))
        return result
