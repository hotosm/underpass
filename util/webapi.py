#!/usr/bin/python3
#
# Copyright (c) 2020, 2021, 2022 Humanitarian OpenStreetMap Team
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

'''
    Simple web API for querying the Underpass database

    Available endpoints:

    Get a summary of tag validation results (returns a CSV)
    /tagValidationSummary
    /tagValidationSummary?hashtag=hotosm-project

    Get a list of geometry validation results (returns a GeoJSON)
    /geoValidationList
    /geoValidationList?hashtag=hotosm-project

    Get a measure of all validation (returns a CSV)
    /validationMeasure
    /validationMeasure?hashtag=hotosm-project
'''

import psycopg2
import argparse
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import urlparse, parse_qs
import json

hostName = "localhost"
serverPort = 8080
db = None

class DB():
    conn = None
    connectionString = ""
    def connect(self):
        """ Connect to the Underpass database """
        try:
            self.conn = psycopg2.connect(self.connectionString)
        except (Exception, psycopg2.DatabaseError) as error:
            print(error)

    def close(self):
        if self.conn is not None:
            self.conn.close()

    def getTagValidationSummary(self, hashtag):
        """ Get latest tag validation results (optional: by hashtag) """
        q = 'WITH t2 AS ( \
            SELECT id \
            from changesets \
            where (added is not null or modified is not null)'

        if hashtag:
            q += 'and EXISTS ( SELECT * from unnest(hashtags) as h where h ~* \'^' + hashtag + '\' )'

        q += 'and updated_at > NOW() - INTERVAL \'240 HOURS\' \
        ), \
        t1 AS ( \
            SELECT change_id, source, \
            unnest(values) as unnest_values \
            from validation\
        )\
        SELECT \
            t1.unnest_values as tag, t1.source, count(t1.unnest_values)\
            FROM t1, t2 \
            where t1.change_id = t2.id \
            group by t1.unnest_values, t1.source \
            order by count desc \
            limit 20;'

        cur = self.conn.cursor()
        result = "tag    source    count\n"
        try:
            cur.execute(q)
        except:
            print("There was an error running the query.")
            cur.close()
            return result
        for row in cur:
            result += '    '.join((str(x)) for x in row) + '\n'
        cur.close()
        return result

    def getGeoValidationList(self, hashtag):
        """ Get latest geometry validation results (optional: by hashtag) """
        q = 'WITH t2 AS ( \
            SELECT id, closed_at \
            from changesets \
            where (added is not null or modified is not null)'

        if hashtag:
            q += 'and EXISTS ( SELECT * from unnest(hashtags) as h where h ~* \'^' + hashtag + '\' )'

        q += 'and updated_at > NOW() - INTERVAL \'240 HOURS\' \
        ), \
        t1 AS ( \
            SELECT osm_id, change_id, location, status \
            from validation \
            where \'badgeom\' = any(status) \
            or \'overlaping\' = any(status) \
            and source = \'building\' \
        )\
        SELECT row_to_json(fc) FROM ( \
            SELECT \'FeatureCollection\' AS type \
            ,array_to_json(array_agg(f)) AS features \
        FROM ( SELECT \'Feature\' AS type \
            , ST_AsGeoJSON(ST_Transform(t1.location, 4326),15,0)::json As geometry \
            , ( SELECT row_to_json(t) \
            FROM ( SELECT t1.osm_id, t1.change_id, t2.closed_at, t1.status \
                    ) AS t ) AS properties \
            FROM t1, t2 \
                where t1.change_id = t2.id \
                order by t2.closed_at desc \
                limit 50 \
            ) AS f ) AS fc'

        cur = self.conn.cursor()
        result = ""
        try:
            cur.execute(q)
        except:
            print("There was an error running the query.")
            cur.close()
            return result
        for row in cur:
            result += json.dumps(row[0])
        cur.close()
        return result

    def getValidationMeasure(self, hashtag):
        """ Get a measure of quality (optional: by hashtag) """
        q = '\
            with t0 as ( \
            select id from changesets c '

        if hashtag:
            q += 'where EXISTS ( SELECT * from unnest(hashtags) as h where h ~* \'^' + hashtag + '\' )'

        q += '), \
            t1 as ( \
                select key, sum(value) as total \
                from ( \
                    select key, value::numeric \
                    from changesets c1, t0, each(added) \
                    where c1.id = t0.id \
                ) s \
                group by 1 \
                order by total desc \
            ), t2 as ( \
            select source, unnest(status) as un_status, count(source) as invalid \
                from validation v, t0 \
                where v.change_id = t0.id \
                group by source, un_status \
            ) select key, un_status as status, invalid, total from t1 \
            INNER JOIN t2 \
            ON t2.source = t1.key \
            where un_status != \'complete\''

        cur = self.conn.cursor()
        result = "key    status    invalid    total\n"
        try:
            cur.execute(q)
        except:
            print("There was an error running the query.")
            cur.close()
            return result
        for row in cur:
            result += '    '.join((str(x)) for x in row) + '\n'
        cur.close()
        return result

class WebApi(BaseHTTPRequestHandler):
    def do_GET(self):
        contentType = "text"
        parsed_url = parse_qs(urlparse(self.path).query)
        if db.conn:

            if self.path.startswith("/tagValidationSummary"):
                contentType = "text/csv"
                if 'hashtag' in parsed_url:
                    response = db.getTagValidationSummary(parsed_url['hashtag'][0])
                else:
                    response = db.getTagValidationSummary(None)

            elif self.path.startswith("/geoValidationList"):
                contentType = "application/json"
                if 'hashtag' in parsed_url:
                    response = db.getGeoValidationList(parsed_url['hashtag'][0])
                else:
                    response = db.getGeoValidationList(None)

            elif self.path.startswith("/"):
                contentType = "text/csv"
                if 'hashtag' in parsed_url:
                    response = db.getValidationMeasure(parsed_url['hashtag'][0])
                else:
                    response = db.getValidationMeasure(None)

            else:
                response = "Not found."
        if response == "Not found.":
            self.send_response(404)
        else:
            self.send_response(200)
        self.send_header("Content-type", contentType)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(bytes(response, "utf-8"))

if __name__ == "__main__":
    args = argparse.ArgumentParser()
    args.add_argument("--connectionString", "-c", help="Connection string", type=str, \
        default="host=localhost port=5439 dbname=galaxy user=underpass password=underpass")
    args = args.parse_args()
    db = DB()
    db.connectionString = args.connectionString
    db.connect()
    webServer = HTTPServer((hostName, serverPort), WebApi)
    print("Server started http://%s:%s" % (hostName, serverPort))
    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass
    db.close()
    webServer.server_close()
    print("Server stopped.")