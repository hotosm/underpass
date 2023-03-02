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

#     This is a simple REST API that you can run easily to get data
#     from the Underpass database
#
#     1. Install requirements
#     pip install psycopg2 ; pip install uvicorn ; pip install "uvicorn[standard]"
#
#     2. Run
#     uvicorn main:app --reload
#
#     3. Request
#     curl --location 'http://127.0.0.1:8000/report/dataQualityGeo' \
#       --header 'Content-Type: application/json' \
#       --data '{"fromDate": "2023-03-01T00:00:00"}'

from fastapi import FastAPI
from pydantic import BaseModel
import dbapi as DBApi

class DataQualityRequest(BaseModel):
    fromDate: str = None
    toDate: str = None
    hashtags: list[str] = None
    area: str = None

app = FastAPI()
reporter = DBApi.Report()

@app.get("/")
def read_root():
    return {"Welcome": "This is the Underpass REST API."}

@app.post("/report/dataQualityGeo")
def dataQualityGeo(request: DataQualityRequest):
    reporter = DBApi.Report()
    results = reporter.getDataQualityGeo(
        fromDate = request.fromDate,
        toDate = request.toDate,
        hashtags = request.hashtags,
        area = request.area
    )
    return results

@app.post("/report/dataQualityTag")
def dataQualityGeo(request: DataQualityRequest):
    reporter = DBApi.Report()
    results = reporter.getDataQualityTag(
        fromDate = request.fromDate,
        toDate = request.toDate,
        hashtags = request.hashtags,
        area = request.area
    )
    return results
