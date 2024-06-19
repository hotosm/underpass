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

import sys,os
sys.path.append(os.path.realpath('../dbapi'))

from models import RawRequest, RawListRequest
from api import raw as RawApi
from api.db import DB
import config
import json

db = DB(config.UNDERPASS_OSM_DB)
raw = RawApi.Raw(db)

async def polygons(request: RawRequest):
    return json.loads(await raw.getPolygons(
        RawApi.RawFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
        ), asJson=True)
    )

async def nodes(request: RawRequest):
    return json.loads(await raw.getNodes(
        RawApi.RawFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
        ), asJson=True)
    )

async def lines(request: RawRequest):
    return json.loads(await raw.getLines(
        RawApi.RawFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
        ), asJson=True)
    )

async def features(request: RawRequest):
    return json.loads(await raw.getFeatures(
        RawApi.RawFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
       ), asJson=True)
    )

async def polygonsList(request: RawListRequest):
    return await raw.getPolygonsList(
        RawApi.ListFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page
        )
    )

async def nodesList(request: RawListRequest):
    return await raw.getNodesList(
        RawApi.ListFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page
        )
    )

async def linesList(request: RawListRequest):
    return await raw.getLinesList(
        RawApi.ListFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page
        )
    )

async def list(request: RawListRequest):
    return await raw.getList(
        RawApi.ListFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page
        )
    )

