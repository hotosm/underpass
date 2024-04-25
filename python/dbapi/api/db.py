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

import asyncpg
import json
from .config import DEBUG

class DB():
    # Default DB configuration
    
    def __init__(self, connectionString = None):
        self.connectionString = connectionString or "postgresql://underpass:underpass@localhost:5432/underpass"
        self.pool = None
        # Extract the name of the database
        self.name = self.connectionString[self.connectionString.rfind('/') + 1:]

    async def __enter__(self):
        await self.connect()

    async def connect(self):
        """ Connect to the database """
        print("Connecting to DB ... " + self.connectionString if DEBUG else "")
        if not self.pool:
            try:
                self.pool = await asyncpg.create_pool(
                    min_size=1,
                    max_size=10,
                    command_timeout=60,
                    dsn=self.connectionString,
                )
            except Exception as e:
                print("Can't connect!")
                print(e)

    def close(self):
        if self.pool is not None:
            self.pool.close()

    async def run(self, query, singleObject = False, asJson=False):
        if DEBUG:
            print("Running query ...")
        if not self.pool:
            await self.connect()
        if self.pool:
            try:
                conn = await self.pool.acquire()
                result = await conn.fetch(query)
                if asJson:
                    if singleObject:
                        return json.dumps(result[0])
                    return result[0]['result']

                else:
                    return result
            except Exception as e: 
                print("\n******* \n" + query + "\n******* \n")
                print(e)
                return None
            finally:
                await self.pool.release(conn)
        return None
