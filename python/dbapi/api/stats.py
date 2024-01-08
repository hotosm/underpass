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

class Stats:
    def __init__(self, db):
        self.underpassDB = db

    def getCount(
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
            query = "with t1 as ( \
                    select count(validation.osm_id) from validation \
                    left join " + table + " on validation.osm_id = " + table + ".osm_id \
                    where {0} {1} {2} {3} {4}".format(
                    "ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "1=1 ",
                    "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
                    "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
                    "AND created at >= {0} AND created_at <= {1}".format(dateFrom, dateTo) if dateFrom and dateTo else "",
                    "AND status = '" + status + "'")
            query += "), t2 as ( \
                    select count(" + table + ".osm_id) as total from " + table + " \
                    where {0} {1} {2} {3} \
                    ) select t1.count, t2.total from t1,t2".format(
                    "ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "1=1 ",
                    "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
                    "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
                    "AND created at >= {0} AND created_at <= {1}".format(dateFrom, dateTo) if dateFrom and dateTo else "")
        else:
            query = "select count(" + table + ".osm_id) from " + table \
                + " where \
                {0} {1} {2} {3} {4}".format(
                    "ST_Intersects(\"geom\", ST_GeomFromText('MULTIPOLYGON((({0})))', 4326) )".format(area) if area else "1=1 ",
                    "AND (" + tagsQueryFilter(tags, table) + ")" if tags else "",
                    "AND " + hashtagQueryFilter(hashtag, table) if hashtag else "",
                    "AND created at >= {0} AND created_at <= {1}".format(dateFrom, dateTo) if dateFrom and dateTo else "",
                    "AND status = '{0}'".format(status) if (status) else "",
                )
        print(query)
        return(self.underpassDB.run(query, True))

    