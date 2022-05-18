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
from sys import argv, stdout
import progressbar
import json
import logging

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.StreamHandler(stdout)
    ]
)

def progressbarWidget(title):
    return [' [',
        progressbar.Timer(format = title + ' (%(elapsed)s'),'] ',
        progressbar.Bar('*')
    ]


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
        query = """SELECT added_buildings, modified_buildings, added_amenity, \
                modified_amenity, added_highway, modified_highway, added_highway_meters, \
                added_places, modified_places FROM all_changesets_stats WHERE changeset = %s""" \
                % changesetId
        self.dbcursor.execute(query)
        changeset = self.dbcursor.fetchone()
        if changeset:
            return {
                'added_buildings': changeset[0] or 0,
                'modified_buildings': changeset[1] or 0,
                'added_amenity': changeset[2] or 0,
				'modified_amenity': changeset[3] or 0,
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
        data = {}
        bar = progressbar.ProgressBar(max_value=len(dataFromFile), widgets=progressbarWidget("Processing Underpass stats")).start()
        data_count = 0
        for row in dataFromFile:
            changeset_id = row['changeset_id']
            if not changeset_id in data:
                data[changeset_id] = {
                    'underpass': {
                        'added_buildings': self.getValue(row, 'added','building'),
                        'modified_buildings': self.getValue(row, 'modified','building'),
                        'added_amenity': self.getValue(row, 'added','amenity'),
                        'modified_amenity': self.getValue(row, 'modified','amenity'),
                        'added_highway': self.getValue(row, 'added','highway'),
                        'modified_highway': self.getValue(row, 'modified','highway'),
                        'added_highway_km': self.getValue(row, 'added','highway_km'),
                        'added_places': self.getValue(row, 'added','places'),
                        'modified_places': self.getValue(row, 'modified','places'),
                    }
                }          
            else:
                data[changeset_id]['underpass']['added_buildings'] += self.getValue(row, 'added','building')
                data[changeset_id]['underpass']['modified_buildings'] += self.getValue(row, 'modified','building')
                data[changeset_id]['underpass']['added_amenity'] += self.getValue(row, 'added','amenity')
                data[changeset_id]['underpass']['modified_amenity'] += self.getValue(row, 'modified','amenity')
                data[changeset_id]['underpass']['added_highway'] += self.getValue(row, 'added','highway')
                data[changeset_id]['underpass']['modified_highway'] += self.getValue(row, 'modified','highway')
                data[changeset_id]['underpass']['added_highway_km'] += self.getValue(row, 'added','highway_km')
                data[changeset_id]['underpass']['added_places'] += self.getValue(row, 'added','places')
                data[changeset_id]['underpass']['modified_places'] += self.getValue(row, 'modified','places')
            data_count += 1
            bar.update(data_count)
        return data

    def getValue(self, row, column, label):
        for value in row[column]:
            if label in value:
                return value[label]
        return 0

try:
    insightsDB = InsightsConnection()
except:
    logging.error('Error connecting to Insights DB')
    quit()

underpassStats = UnderpassStats()
stats = underpassStats.readJSON(json_file)

count = 0
bar = progressbar.ProgressBar(max_value=len(stats), widgets=progressbarWidget("Getting Insights stats")).start()
for changeset_id in stats:
    stats[changeset_id]["changeset"] = changeset_id
    stats[changeset_id]["insights"] = insightsDB.getChangesetsStats(changeset_id)
    bar.update(count)
    count += 1
 
print(json.dumps(stats))

