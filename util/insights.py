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
import json

def usage():
    out = """
    Usage:

    python insights.py <db_connection_string> <stats_json_file>
    """
    print(out)
    quit()

if len(argv) <= 1:
    usage()

database = argv[1]
json_file = argv[2]

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
                added_highway, modified_highway, added_highway_meters, \
                added_places, modified_places FROM all_changesets_stats WHERE changeset = %s""" \
                % changesetId
        self.dbcursor.execute(query)
        changeset = self.dbcursor.fetchone()
        if changeset:
            return {
                'changeset': changeset[0] or 0,
                'added_buildings': changeset[1] or 0,
                'modified_buildings': changeset[2] or 0,
                'added_amenity': changeset[3] or 0,
                'added_highway': changeset[4] or 0,
                'modified_highway': changeset[5] or 0,
                'added_highway_km': round(changeset[6] or 0) ,
                'added_places': changeset[7] or 0,
                'modified_places': changeset[8] or 0
            }
        else:
            return None


class UnderpassStats:
    def __init__(self):
        pass

    def readJSON(self, json_file):
        f = open(json_file)
        dataFromFile = json.load(f)
        data = []
        for row in dataFromFile:
            existingItemIndex = next((i for i, item in enumerate(data) if item["changeset"] == row['changeset_id']), None)
            if existingItemIndex == None:
                data.append({
                    'changeset': row['changeset_id'],
                    'added_buildings': self.getValue(row, 'added','building'),
                    'modified_buildings': self.getValue(row, 'modified','building'),
                    'added_amenity': self.getValue(row, 'added','amenity'),
                    'added_highway': self.getValue(row, 'added','highway'),
                    'modified_highway': self.getValue(row, 'modified','highway'),
                    'added_highway_km': self.getValue(row, 'added','highway_km'),
                    'added_places': self.getValue(row, 'added','places'),
                    'modified_places': self.getValue(row, 'modified','places'),
                })
            else:
                data[existingItemIndex]['added_buildings'] += self.getValue(row, 'added','building')
                data[existingItemIndex]['modified_buildings'] += self.getValue(row, 'modified','building')
                data[existingItemIndex]['added_amenity'] += self.getValue(row, 'added','amenity')
                data[existingItemIndex]['added_highway'] += self.getValue(row, 'added','highway')
                data[existingItemIndex]['modified_highway'] += self.getValue(row, 'modified','highway')
                data[existingItemIndex]['added_highway_km'] += self.getValue(row, 'added','highway_km')
                data[existingItemIndex]['added_places'] += self.getValue(row, 'added','places')
                data[existingItemIndex]['modified_places'] += self.getValue(row, 'modified','places')
        
        return data

    def getValue(self, row, column, label):
        for value in row[column]:
            if label in value:
                return value[label]
        return 0

bar = PixelSpinner('Reading Underpass stats ... ')
underpassStats = UnderpassStats()
stats = underpassStats.readJSON(json_file)

bar = PixelSpinner('Connecting ... ')
insightsDB = InsightsConnection()
bar = PixelSpinner('Getting data ... ')

insightsCount = 0
badStatsCount = 0
notFound = []
for stat in stats:
    insights = insightsDB.getChangesetsStats(stat['changeset'])
    if insights is not None:
        insightsCount += 1
        if insights != stat:
            badStatsCount += 1
            bad_stat = {'changeset': stat['changeset']}
            for value in insights:
                if insights[value] != stat[value]:
                    bad_stat['stats_' + str(value)] = stat[value]
                    bad_stat['insights__' + str(value)] = insights[value]
            print(bad_stat)
    else:
        notFound.append(stat['changeset'])

print("\n---------")
print(len(notFound), "changesets not found in Insights DB")
print(badStatsCount, "/", insightsCount, "are not equal (", badStatsCount * 100 / insightsCount, "%)")