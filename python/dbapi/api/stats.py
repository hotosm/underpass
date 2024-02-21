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

from .filters import tagsQueryFilter, hashtagQueryFilter

class Stats:
    def __init__(self, db):
        self.underpassDB = db

    async def getCount(
        self, 
        area = None,
        tags = None,
        hashtag = None,
        dateFrom = None,
        dateTo = None,
        status = None,
        featureType = None,
    ):
        if featureType == "line":
            table = "ways_line"
        elif featureType == "node":
            table = "nodes"
        else:
            table = "ways_poly"

        if status:
            query = "with all_features as ( \
                select {0}.osm_id from {0} \
                left join changesets c on changeset = c.id \
                where {1} {2} {3} {4} {5} \
            ), \
            count_validated_features as ( \
                select count(distinct(all_features.osm_id)) as count from all_features \
                left join validation v on all_features.osm_id = v.osm_id \
                where v.status = '{6}' \
            ), count_features as (\
                select count(distinct(all_features.osm_id)) as total from all_features \
            ) \
            select count, total from  count_validated_features, count_features".format(
                table,
                "created_at >= '{0}'".format(dateFrom) if (dateFrom) else "1=1",
                "AND created_at <= '{0}'".format(dateTo) if (dateTo) else "",
                "AND ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "",
                "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
                "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
                status
            )
        else:
           query = "select count(distinct {0}.osm_id) as count from {0} \
            left join changesets c on changeset = c.id \
            where {1} {2} {3} {4} {5}".format(
                table,
                "created_at >= '{0}'".format(dateFrom) if (dateFrom) else "1=1",
                "AND created_at <= '{0}'".format(dateTo) if (dateTo) else "",
                "AND ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "",
                "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
                "AND " + hashtagQueryFilter(hashtag, table) if hashtag else ""
            )
        return(await self.underpassDB.run(query, True))

    