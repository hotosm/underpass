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

from models import RawValidationRequest, RawValidationListRequest, RawValidationStatsRequest
from api import rawValidation as RawValidationApi
from api.db import DB
import config
import json

db = DB(config.UNDERPASS_DB)
rawval = RawValidationApi.RawValidation(db)

async def polygons(request: RawValidationRequest):
    return json.loads(await  rawval.getPolygons(
        RawValidationApi.RawValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        ), asJson=True)
    )

async def nodes(request: RawValidationRequest):
    return json.loads(await rawval.getNodes(
        RawValidationApi.RawValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        ), asJson=True)
    )

async def lines(request: RawValidationRequest):
    return json.loads(await rawval.getLines(
        RawValidationApi.RawValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
       ), asJson=True)
    )

async def features(request: RawValidationRequest):
    return json.loads(await rawval.getFeatures(
        RawValidationApi.RawValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
         ), asJson=True)
    )

async def polygonsList(request: RawValidationListRequest):
    return await rawval.getPolygonsList(
        RawValidationApi.ListValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def nodesList(request: RawValidationListRequest):
    return await rawval.getNodesList(
        RawValidationApi.ListValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def linesList(request: RawValidationListRequest):
    return await rawval.getLinesList(
        RawValidationApi.ListValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def list(request: RawValidationListRequest):
    return await rawval.getList(
        RawValidationApi.ListValidationFeaturesParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            orderBy = request.orderBy,
            page = request.page,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def count(request: RawValidationStatsRequest):
    return await rawval.getCount(
        RawValidationApi.ValidationCountParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def nodesCount(request: RawValidationStatsRequest):
    return await rawval.getNodesCount(
        RawValidationApi.ValidationCountParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def linesCount(request: RawValidationStatsRequest):
    return await rawval.getLinesCount(
        RawValidationApi.ValidationCountParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )

async def polygonsCount(request: RawValidationStatsRequest):
    return await rawval.getPolygonsCount(
        RawValidationApi.ValidationCountParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo,
            status = RawValidationApi.ValidationError(request.status) if request.status else None
        )
    )
