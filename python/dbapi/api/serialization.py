
#!/usr/bin/python3
#
# Copyright (c) 2024 Humanitarian OpenStreetMap Team
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

import json

def queryToJSON(query: str):
   jsonQuery = "with data AS \n ({query}) \n \
      SELECT to_jsonb(data) as result from data;" \
      .format(query=query)
   return jsonQuery

def deserializeTags(data):
    result = []
    if data:
      for row in data:
         row_dict = dict(row)
         if 'tags' in row:
            row_dict['tags'] = json.loads(row['tags'])
         result.append(row_dict)
    return result
