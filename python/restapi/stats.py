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

from models import StatsRequest
from api import stats as StatsApi
from api.db import DB
import config

db = DB(config.UNDERPASS_DB)
stats = StatsApi.Stats(db)

def nodes(request: StatsRequest):
    return stats.getNodesCount(
        StatsApi.StatsParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo
        )
    )

async def lines(request: StatsRequest):
    return await stats.getLinesCount(
        StatsApi.StatsParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo
        )
    )

async def lines(request: StatsRequest):
    return await stats.getLinesCount(
        StatsApi.StatsParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo
        )
    )

async def polygons(request: StatsRequest):
    return await stats.getPolygonsCount(
        StatsApi.StatsParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo
        )
    )

async def features(request: StatsRequest):
    return await stats.getCount(
        StatsApi.StatsParamsDTO(
            area = request.area,
            tags = request.tags,
            hashtag = request.hashtag,
            dateFrom = request.dateFrom,
            dateTo = request.dateTo
        )
    )
