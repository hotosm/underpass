
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

# This file build and run queries for getting validation results
# from the Underpass validation table in combination with raw OSM data

from .raw import listFeaturesQuery
from dataclasses import dataclass
from enum import Enum
from .filters import tagsQueryFilter, hashtagQueryFilter

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

# DB table names
class Table(Enum):
    nodes = "nodes"
    lines = "ways_line"
    polygons = "ways_poly"
    relations = "relations"

# Validation Count Query DTO
@dataclass
class ValidationCountParamsDTO:
    status: ValidationError
    tags: list[str] = None
    hashtag: str = ""
    dateFrom: str = ""
    dateTo: str = ""
    area: str = None
    table: Table = Table.nodes

def countQueryToJSON(query: str):
     jsonQuery = "with data AS \n ({query}) \n \
        SELECT to_jsonb(data) as result from data;" \
        .format(query=query)
     return jsonQuery

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
        return countQueryToJSON(query)
    return query

# This class build and run queries for Validation Data
class RawValidation:
    def __init__(self,db):
        self.db = db

    # Get count of validation errors
    async def getCount(
        self,
        params: ValidationCountParamsDTO,
        asJson: bool = False,
    ):
        return await self.db.run(countQuery(params, asJson), asJson=asJson)
