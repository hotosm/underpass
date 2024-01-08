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
sys.path.append(os.path.realpath('../dbapi'))

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from models import * 
from api import raw, stats
from api.db import UnderpassDB
import config

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=config.ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"]
)

db = UnderpassDB(config.UNDERPASS_DB)
db.connect()
rawer = raw.Raw(db)
statser = stats.Stats(db)

@app.get("/")
def read_root():
    return {"Welcome": "This is the Underpass REST API."}

@app.post("/raw/polygons")
def getPolygons(request: RawRequest):
    results = rawer.getPolygons(
        area = request.area or None,
        tags = request.tags or "",
        hashtag = request.hashtag or "",
        dateFrom = request.dateFrom or "",
        dateTo = request.dateTo or "",
        status = request.status or "",
        page = request.page
    )
    return results

@app.post("/raw/nodes")
def getNodes(request: RawRequest):
    results = rawer.getNodes(
        area = request.area,
        tags = request.tags or "",
        hashtag = request.hashtag or "",
        dateFrom = request.dateFrom or "",
        dateTo = request.dateTo or "",
        status = request.status or "",
        page = request.page
    )
    return results

@app.post("/raw/lines")
def getLines(request: RawRequest):
    results = rawer.getLines(
        area = request.area,
        tags = request.tags or "",
        hashtag = request.hashtag or "",
        dateFrom = request.dateFrom or "",
        dateTo = request.dateTo or "",
        status = request.status or "",
        page = request.page
    )
    return results

@app.post("/raw/features")
def getRawFeatures(request: RawRequest):
    results = rawer.getFeatures(
        area = request.area or None,
        tags = request.tags or "",
        hashtag = request.hashtag or "",
        dateFrom = request.dateFrom or "",
        dateTo = request.dateTo or "",
        status = request.status or "",
        page = request.page,
        featureType = request.featureType or None
    )
    return results

@app.post("/raw/list")
def getRawList(request: RawRequest):
    results = rawer.getList(
        area = request.area or None,
        tags = request.tags or "",
        hashtag = request.hashtag or "",
        dateFrom = request.dateFrom or "",
        dateTo = request.dateTo or "",
        status = request.status or "",
        orderBy = request.orderBy or None,
        page = request.page,
        featureType = request.featureType or None
    )
    return results

@app.post("/stats/count")
def getStatsCount(request: StatsRequest):
    results = statser.getCount(
        area = request.area or None,
        tags = request.tags or "",
        hashtag = request.hashtag or "",
        dateFrom = request.dateFrom or "",
        dateTo = request.dateTo or "",
        status = request.status or "",
        featureType = request.featureType or None
    )
    return results