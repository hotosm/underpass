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

import psycopg2

class UnderpassDB():
    conn = None

    # Default Underpass local DB configuration
    # This might be replaced by an .ini config file
    
    def __init__(self, connectionString = None):
        self.connectionString = connectionString or "postgresql://underpass:underpass@postgis/underpass"
        self.connect()

    def connect(self):
        """ Connect to the database """
        try:
            self.conn = psycopg2.connect(self.connectionString)
        except (Exception, psycopg2.DatabaseError) as error:
            print(error)

    def close(self):
        if self.conn is not None:
            self.conn.close()

    def run(self, query, responseType = 'json'):
        if self.conn:
            cur = self.conn.cursor()
            try:
                cur.execute(query)
            except:
                print("*******" + "\n" + query + "\n")
                cur.close()
                return {"Error": "There was an error running the query."}

            results = None

            if responseType == 'csv':
                results = ""
                for row in cur:
                    results += '    '.join((str(x)) for x in row) + '\n'
                cur.close()
                csvHeaders = '    '.join([desc[0] for desc in cur.description])  + '\n'
                return csvHeaders + results
            else:
                results = []
                colnames = [desc[0] for desc in cur.description]
                for row in cur:
                    item = {}
                    for index, column in enumerate(colnames):
                        item[column] = row[index]
                    results.append(item)
            cur.close()
            return results
