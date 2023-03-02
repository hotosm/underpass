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
from .queryHelper import hashtags as hashtagsQueryBuilder
from .queryHelper import bbox as bboxQueryBuilder
from .queryHelper import RESULTS_PER_PAGE

class Report:
    def __init__(self):
        pass

    underpassDB = UnderpassDB()
    responseType = "json"

    def getDataQualityGeo(
        self, 
        fromDate = None,
        toDate = None,
        hashtags  = None, 
        area = None,
        page = 0
    ):
        query = "select \
            'https://osm.org/' || type || '/' || osm_id as link, \
            'https://osm.org/api/0.6/' || type || '/' || osm_id as download \
            from changesets \
            INNER JOIN validation \
            ON validation.change_id = changesets.id \
            where 'badgeom' = any (validation.status) \
            {0} {1} {2} {3} \
            order by osm_id \
            limit {4} offset {5}".format(
                "and closed_at >= '{0}'".format(fromDate) if fromDate else "",
                "and closed_at <= '{0}'".format(toDate) if toDate else "",
                "and " + hashtagsQueryBuilder(hashtags) if hashtags else "",
                "and " + bboxQueryBuilder(area) if area else "",
                RESULTS_PER_PAGE,
                RESULTS_PER_PAGE * page
            )
        return self.underpassDB.run(query, self.responseType)

    def getDataQualityTag(
            self,
            fromDate = None,
            toDate = None,
            hashtags  = None, 
            area = None,
            page = 0
        ):
        query = "WITH t2 AS ( \
            SELECT id \
                from changesets \
                where (added is not null or modified is not null) \
                {0} {1} {2} {3} \
            ), \
            t1 AS ( \
                SELECT change_id, source, osm_id, type, \
                unnest(values) as unnest_values \
                from validation, t2 \
                where change_id = t2.id \
            ) \
            select \
                'https://osm.org/' || t1.type || '/' || t1.osm_id as link, \
                t1.unnest_values as tag, t1.source \
                FROM t1, t2 \
                where t1.change_id = t2.id \
                limit {4} offset {5}".format(
                    "and closed_at >= '{0}'".format(fromDate) if fromDate else "",
                    "and closed_at <= '{0}'".format(toDate) if toDate else "",
                    "and " + hashtagsQueryBuilder(hashtags) if hashtags else "",
                    "and " + bboxQueryBuilder(area) if area else "",
                    RESULTS_PER_PAGE,
                    RESULTS_PER_PAGE * page
                )
        return self.underpassDB.run(query, self.responseType)