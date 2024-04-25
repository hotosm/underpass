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

def tagsQueryFilter(tagsQuery, table):
    query = ""
    tags = tagsQuery.split(",")
    keyValue = tags[0].split("=")

    if len(keyValue) == 2:
        query += "{table}.tags->>'{key}' ~* '^{value}'".format(
            table=table,
            key=keyValue[0],
            value=keyValue[1]
        )
    else:
        query += "{table}.tags->>'{key}' IS NOT NULL".format(
            table=table, 
            key=keyValue[0]
        )

    for tag in tags[1:]:
        keyValue = tag.split("=")
        if len(keyValue) == 2:
            query += "OR {table}.tags->>'{key}' ~* '^{value}'".format(
                table=table,
                key=keyValue[0],
                value=keyValue[1]
            )
        else:
            query += "OR {table}.tags->>'{key}' IS NOT NULL".format(
                table=table,
                key=keyValue[0]
            )
    return query

def hashtagQueryFilter(hashtag, table):
    return "'{hashtag}' = ANY (hashtags)".format(
        hashtag=hashtag
    )
