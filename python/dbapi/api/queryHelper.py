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

RESULTS_PER_PAGE = 25

def hashtags(hashtagsList):
    return "EXISTS ( SELECT * from unnest(hashtags) as h where {condition} )".format(
        condition=' OR '.join(
            map(lambda x: "h ~* '^{hashtag}'".format(hashtag=x), hashtagsList)
        )
    )

def bbox(wktMultipolygon):
    return "ST_Intersects(bbox, ST_GeomFromText('{area}', 4326))".format(
        area=wktMultipolygon
    )
