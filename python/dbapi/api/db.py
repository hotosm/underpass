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

class UnderpassDB():
    # Default Underpass local DB configuration
    # This might be replaced by an .ini config file
    
    def __init__(self, connectionString = None):
        self.connectionString = connectionString or "postgresql://underpass:underpass@postgis/underpass"
        self.pool = None

    async def __enter__(self):
        await self.connect()

    async def connect(self):
        """ Connect to the database """
        print("Connecting to DB ...")
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

    async def run(self, query, singleObject = False):
        if not self.pool:
            await self.connect()
        if self.pool:
            try:
                conn = await self.pool.acquire()
                result = await conn.fetch(query)
                if singleObject:
                    return result[0]
                return json.loads((result[0]['result']))
            except Exception as e: 
                # print("\n******* \n" + query + "\n******* \n")
                print(e)
                if singleObject:
                    return {}
                else:
                    return []
            finally:
                await self.pool.release(conn)
        return None
