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

#     This is an example on how you can use the Underpass API
#     to get data from the Underpass database
#
#     Install psycopg2 (pip install psycopg2) and run (python report.py)

import sys,os
sys.path.append(os.path.realpath('..'))

from api import report

reporter = report.Report()

results = reporter.getDataQualityTag(
    fromDate = "2022-12-28T00:00:00", 
    hashtags = [],
    responseType = "csv"
)

print(results)
