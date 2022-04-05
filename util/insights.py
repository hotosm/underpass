#!/usr/bin/python3
#
# Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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

import psycopg2
from urllib.parse import urlparse
from sys import argv
from progress.spinner import PixelSpinner

def usage():
    out = """
    Usage:

    python insights.py <db_connection_string> <changeset_id>
    """
    print(out)
    quit()

if len(argv) <= 1:
    usage()

database = argv[1]
changesetId = argv[2]

class InsightsConnection:
    def __init__(self):
        conn = urlparse(database)
        
        self.dbshell = psycopg2.connect(
            database = conn.hostname,
            user = conn.username,
            password = conn.password,
            port = conn.port,
            host = conn.scheme
        )
        self.dbshell.autocommit = True
        self.dbcursor = self.dbshell.cursor()

    def getChangesetsStats(self, changesetId):
        query = """SELECT changeset, added_buildings, modified_buildings, added_amenity, \
                added_highway, modified_highway, added_highway_meters, modified_highway_meters, \
                added_places, modified_places FROM all_changesets_stats WHERE changeset = %s""" \
                % changesetId
        self.dbcursor.execute(query)
        changeset = self.dbcursor.fetchone()
        return {
            'changeset': changeset[0],
            'added_buildings': changeset[1],
            'modified_buildings': changeset[2],
            'added_amenity': changeset[3],
            'added_highway': changeset[4],
            'modified_highway': changeset[5],
            'added_highway_meters': changeset[6],
            'modified_highway_meters': changeset[7],
            'added_places': changeset[8],
            'modified_places': changeset[9]
        }

bar = PixelSpinner('Connecting ... ')
insightsDB = InsightsConnection()
bar = PixelSpinner('Getting data ... ')
stats = insightsDB.getChangesetsStats(changesetId)
print(stats)

