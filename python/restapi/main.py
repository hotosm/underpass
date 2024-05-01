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

#     This is a simple REST API that you can run easily to get data
#     from the Underpass database
#
#     1. Install requirements
#     pip install psycopg2 ; pip install fastapi ; pip install "uvicorn[standard]"
#
#     2. Run
#     uvicorn main:app --reload 
#
#     Optionally, for running the server into the Docker container
#     and access from the host:
#
#     uvicorn main:app --reload --host 0.0.0.0
#
#     3. Request
#     curl --location 'http://127.0.0.1:8000/report/dataQualityGeo' \
#       --header 'Content-Type: application/json' \
#       --data '{"fromDate": "2023-03-01T00:00:00"}'

import sys,os

from fastapi import FastAPI
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware
from models import StatsRequest, RawRequest, RawListRequest, RawValidationRequest, RawValidationListRequest
import raw, stats, rawval
import config

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=config.ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"]
)

class Index:
    @app.get("/")
    async def index():
        return {"message": "This is the Underpass REST API."}

class Config:
# Availability (which countries this API provides data)
    # Ex: ["nepal", "argentina"]
    @app.get("/availability")
    async def getAvailability():
        return {
            "countries": config.AVAILABILITY
        }

# Raw OSM Data

class Raw:
    @app.post("/raw/polygons")
    async def polygons(request: RawRequest):
        return await raw.polygons(request)

    @app.post("/raw/nodes")
    async def nodes(request: RawRequest):
        return await raw.nodes(request)

    @app.post("/raw/lines")
    async def lines(request: RawRequest):
        return await raw.lines(request)

    @app.post("/raw/features")
    async def features(request: RawRequest):
        return await raw.features(request)

    @app.post("/raw/list")
    async def list(request: RawListRequest):
        return await raw.list(request)

    @app.post("/raw/polygons/list")
    async def polygons(request: RawListRequest):
        return await raw.polygonsList(request)

    @app.post("/raw/nodes/list")
    async def nodes(request: RawListRequest):
        return await raw.nodesList(request)

    @app.post("/raw/lines/list")
    async def lines(request: RawListRequest):
        return await raw.linesList(request)


# Raw OSM Data and Validation

class RawValidation:
    @app.post("/raw-validation/polygons")
    async def polygons(request: RawValidationRequest):
        return await raw.polygons(request)

    @app.post("/raw-validation/nodes")
    async def nodes(request: RawValidationRequest):
        return await rawval.nodes(request)

    @app.post("/raw-validation/lines")
    async def lines(request: RawValidationRequest):
        return await rawval.lines(request)

    @app.post("/raw-validation/features")
    async def features(request: RawValidationRequest):
        return await rawval.features(request)

    @app.post("/raw-validation/list")
    async def list(request: RawValidationListRequest):
        return await rawval.list(request)

    @app.post("/raw-validation/polygons/list")
    async def polygons(request: RawValidationListRequest):
        return await rawval.polygonsList(request)

    @app.post("/raw-validation/nodes/list")
    async def nodes(request: RawValidationListRequest):
        return await rawval.nodesList(request)

    @app.post("/raw-validation/lines/list")
    async def lines(request: RawValidationListRequest):
        return await rawval.linesList(request)

# Statistics

class Stats:
    @app.post("/stats/nodes")
    async def nodes(request: StatsRequest):
        return await stats.nodes(request)

    @app.post("/stats/lines")
    async def lines(request: StatsRequest):
        return await stats.lines(request)

    @app.post("/stats/polygons")
    async def polygons(request: StatsRequest):
        return await stats.polygons(request)

    @app.post("/stats/features")
    async def features(request: StatsRequest):
        return await stats.features(request)
