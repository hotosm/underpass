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

from .db import UnderpassDB

class Raw:
    def __init__(self):
        pass

    underpassDB = UnderpassDB()

    def getArea(
        self, 
        area = None,
        responseType = "json"
    ):
        query = "with t0 AS ( SELECT osm_id AS node_id FROM raw_node WHERE ST_Intersects(\"geometry\", ST_GeomFromText('POLYGON(({0}))', 4326)) LIMIT 3000 ), \
            t1 AS (SELECT osm_id AS way_id, unnest(refs) AS node_id FROM raw_poly WHERE tags -> 'building' = 'yes' AND refs && array(SELECT * FROM t0)::bigint[] ), \
            t2 AS (SELECT way_id, osm_id, geometry FROM t1 JOIN raw_node ON node_id = raw_node.osm_id), \
            t3 AS (SELECT way_id, ST_MakeLine(geometry) AS linestring FROM t2 GROUP BY way_id), \
            t4 AS (SELECT way_id, status, ST_MakePolygON(ST_AddPoint(t3.linestring, ST_StartPoint(t3.linestring))) AS polygON FROM t3 LEFT JOIN validatiON ON validatiON.osm_id = way_id), \
            t5 AS ( SELECT jsONb_build_object( 'type', 'Feature', 'id', t4.way_id, 'properties', to_jsONb(t4) - 'polygON' , 'geometry', ST_AsGeoJSON(polygON)::jsONb ) AS feature FROM t4 ) \
            SELECT jsONb_build_object( 'type', 'FeatureCollectiON', 'features', jsONb_agg(t5.feature) ) FROM t5 ; \
            ".format(area)
        return self.underpassDB.run(query, responseType)


