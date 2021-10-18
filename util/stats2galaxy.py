#!/usr/bin/python3

# Copyright (c) 2021 Humanitarian OpenStreetMap Team
#
# This file is part of Odkconvert.
#
#     stats2galaxy is free software: you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation, either version 3 of the License, or
#     (at your option) any later version.
#
#     Odkconvert is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with Odkconvert.  If not, see <https:#www.gnu.org/licenses/>.
#

import argparse
import psycopg2
import epdb
from codetiming import Timer


class Pydb(object):
    def __init__(self, db=None, host=None):
        """Postgres access functions"""
        connect = "dbname=\'"
        connect += db + "\'"
        if host is not None:
            connect += "host=\'" + host + "\'"
        self.dbshell = psycopg2.connect(connect)
        self.dbshell.autocommit = True
        self.dbcursor = self.dbshell.cursor()

    def query(self, query):
        self.dbcursor.execute(query)
        result = self.dbcursor.fetchall()
        return result


class Merge(object):
    def __init__(self, indb=None, outdb=None, host=None):
        """Load a small bounding box for each country using the geoboundaries in the
        osmstats database."""
        self.indb = Pydb(indb, host)
        self.outdb = Pydb(outdb, host)
        self.countries = dict()
        geoquery = "SELECT cid,St_AsText(ST_Envelope(ST_Buffer(ST_Centroid(boundary), 10, 4))) FROM geoboundaries;"
        self.outdb.dbcursor.execute(geoquery)
        for row in self.outdb.dbcursor.fetchall():
            self.countries[row[0]] = row[1]
        # self.hashtags = dict()
        # hashquery = "SELECT id,hashtag FROM raw_hashtags"
        # self.indb.dbcursor.execute(hashquery)
        # for row in self.indb.dbcursor.fetchall():
        #     self.hashtags[row[0]] = row[1]
        self.timer = Timer()

    def getBbox(self, cid):
        """Get the bounding box for a country"""
        if cid in self.countries:
            return self.countries[cid]
        else:
            return None

    def mergeUsers(self):
        """Merge the raw_users table from the leaderboard into osmstats"""
        inquery = "SELECT id,name FROM raw_users;"
        self.indb.dbcursor.execute(inquery)
        for entry in self.indb.dbcursor.fetchall():
            outquery = "INSERT INTO users(id,name) VALUES({id}, \'{name}\') ON CONFLICT(id) DO UPDATE SET id="
            # watch out for single quotes in user names
            value = entry[1].replace("'", "&apos;")
            outquery += str(entry[0]) + ", name=\'" + value + "\'"
            query = outquery.format(id=int(entry[0]), name=value)
            self.outdb.dbcursor.execute(query)

    def mergeHashtags(self):
        """Merge the raw_hashtags table from the leaderboard into osmstats"""
        self.timer.start()
        inquery = "SELECT changeset_id,hashtag FROM raw_changesets_hashtags INNER JOIN raw_hashtags ON (raw_changesets_hashtags.hashtag_id = id);"
        self.indb.dbcursor.execute(inquery)
        self.timer.stop()
        for entry in self.indb.dbcursor.fetchall():
            outquery = "INSERT INTO changesets(id,hashtags) VALUES({id}, ARRAY['{hashtags}']) ON CONFLICT(id) DO UPDATE SET id="
            # watch out for single quotes in user names
            fixed = entry[1].replace("'", "&apos;")
            outquery += str(entry[0]) + ", hashtags=ARRAY_APPEND(changesets.hashtags, '" + fixed + "')"
            self.outdb.dbcursor.execute(outquery.format(id=int(entry[0]), hashtags=fixed))

    def mergeStatistics(self):
        """Merge the raw_changesets table from the leaderboard into osmstats"""
        self.timer.start()
        query = "SELECT id, road_km_added, road_km_modified, waterway_km_added, waterway_km_modified, roads_added, roads_modified, waterways_added, waterways_modified, buildings_added, buildings_modified, pois_added, pois_modified, editor, user_id, created_at, closed_at, updated_at,country_id FROM raw_changesets INNER JOIN raw_changesets_countries ON (raw_changesets_countries.changeset_id = id);"
        self.indb.dbcursor.execute(query)
        self.timer.stop()
        result = self.indb.dbcursor.fetchone()
        while result is not None:
            stats = dict()
            added = dict()
            modified = dict()
            # non statistics fields
            stats['change_id'] = result[0]
            stats['editor'] = result[11]
            stats['user_id'] = result[14]
            stats['created_at'] = result[15]
            stats['closed_at'] = result[16]
            if stats['created_at'] is None and stats['closed_at'] is None:
                result = self.indb.dbcursor.fetchone()
                continue
            if stats['created_at'] is None:
                 stats['created_at'] = stats['closed_at']
            if stats['closed_at'] is None:
                stats['closed_at'] = stats['created_at']
            stats['updated_at'] = result[17]
            stats['country_id'] = result[18]
            if self.getBbox(result[18]) is None:
                print("Country ID %s is not in the geoboundaries table" % result[18])
                result = self.indb.dbcursor.fetchone()
                continue                
            stats['bbox'] = "ST_Multi(ST_GeomFromText('" 
            stats['bbox'] += self.getBbox(result[18]) + "')"
            # Added fields
            added['highway_km'] = result[1]
            added['waterway_km'] = result[3]
            added['highways'] = result[4]
            added['waterways'] = result[7]
            added['buildings'] = result[9]
            added['pois'] = result[11]
            # Modified fields
            modified['highway_km'] = result[2]
            modified['waterway_km'] = result[4]
            modified['highways'] = result[6]
            modified['waterways'] = result[8]
            modified['buildings'] = result[10]
            modified['pois'] = result[12]
            # Get the next row, since we're done with this one
            result = self.indb.dbcursor.fetchone()

            # Build the hstore for the added statistics 
            hadd = "HSTORE(ARRAY["
            for key, value in added.items():
                hadd += "ARRAY['" + key + "','" + str(value) + "'],"
            length = len(hadd)-1
            hadd = hadd[:length]
            hadd += "])"

            # Build the hstore for the added statistics 
            hmod = "HSTORE(ARRAY["
            for key, value in modified.items():
                hmod += "ARRAY['" + key + "','" + str(value) + "'],"
            length = len(hmod)-1
            hmod = hmod[:length]
            hmod += "])"

            query = "INSERT INTO changesets(id, editor, user_id, created_at, closed_at, updated_at, added, modified, bbox)"
            query += " VALUES({id}, '{editor}', {user_id}, '{created_at}', '{closed_at}', '{updated_at}', {add}, {mod}, {bbox})) ON CONFLICT(id) DO UPDATE SET editor='{editor}', user_id={user_id}, created_at='{created_at}', closed_at='{closed_at}', updated_at='{updated_at}', added={add}, modified={mod}, bbox={bbox});"
            outquery = query.format(id=stats['change_id'],
                                    editor=stats['editor'],
                                    user_id=stats['user_id'],
                                    created_at=stats['created_at'],
                                    closed_at=stats['closed_at'],
                                    updated_at=stats['updated_at'],
                                    bbox=stats['bbox'],
                                    add=hadd,
                                    mod=hmod)
            #print(outquery)
            self.outdb.dbcursor.execute(outquery)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='convert Leaderboard database to Galaxy database')
    parser.add_argument("--host", help='The database host (localhost)')
    parser.add_argument("--indb",  default="leaderboard", help='The input database (MM Leaderboard)')
    parser.add_argument("--outdb", default='osmstats', help='The output database (Galaxy)')
    args = parser.parse_args()

    user = Merge(args.indb, args.outdb, args.host)
    result = user.mergeUsers()
    result = user.mergeHashtags()
    result = user.mergeStatistics()

