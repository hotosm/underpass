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

import os

UNDERPASS_DB=os.getenv("UNDERPASS_API_DB") or "postgresql://localhost/underpass"
UNDERPASS_OSM_DB=os.getenv("UNDERPASS_API_OSM_DB") or "postgresql://localhost/osm"
ORIGINS = os.getenv("UNDERPASS_API_ORIGINS").split(",") if os.getenv("UNDERPASS_API_ORIGINS") else [
    "http://localhost",
    "http://localhost:5000",
    "http://localhost:3000",
    "http://localhost:8080",
    "http://127.0.0.1",
    "http://127.0.0.1:5000"
]
AVAILABILITY=os.getenv("UNDERPASS_API_AVAILABILITY").split(",") if os.getenv("UNDERPASS_API_AVAILABILITY") else []