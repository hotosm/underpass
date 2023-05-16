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
from .queryHelper import bbox as bboxQueryBuilder

class Raw:
    def __init__(self):
        pass

    underpassDB = UnderpassDB()

    def getArea(
        self, 
        area = None,
        responseType = "json"
    ):
        query = "with t0 as ( \
                    select osm_id as node_id from raw \
                    where osm_type = 'N' \
                    and ST_Intersects(\"geometry\", ST_GeomFromText('POLYGON(({0}))', 4326)) \
                    limit 2000 \
                ), t1 as ( \
                    select osm_id as way_id from raw where \
                \"type\" = 'polygon' \
                and refs && array(select * from t0)::bigint[] \
                ),  \
                t2 AS ( \
                select status, way_id, unnest(refs) AS node_id from raw left join validation on validation.osm_id = raw.osm_id, t1 where raw.osm_id = t1.way_id \
                and raw.type = 'polygon' \
                and tags -> 'building' = 'yes' \
                ), \
                t3 as ( \
                    select status, way_id, node_id, ROW_NUMBER () OVER (ORDER BY NULL) as rn from t2 \
                ), \
                t4 AS ( \
                select status, rn, way_id, node_id, geometry from t3 inner join raw on node_id = osm_id and type = 'point' order by rn desc \
                ), \
                t5 AS ( \
                SELECT  status, way_id, ST_MakePolygon(ST_MakeLine(geometry)) AS polygon FROM t4 group by way_id, status \
                ), \
                t6 as ( \
                    SELECT jsonb_build_object( \
                        'type',       'Feature', \
                        'id',         t5.way_id, \
                        'properties', to_jsonb(t5) - 'polygon' , \
                        'geometry',   ST_AsGeoJSON(polygon)::jsonb \
                ) AS feature from t5 \
                ) SELECT jsonb_build_object( \
                    'type',     'FeatureCollection', \
                    'features', jsonb_agg(t6.feature) \
                ) from t6;".format(area)
        return self.underpassDB.run(query, responseType)



    def getAreaByHashtag(
        self, 
        area = None,
        responseType = "json"
    ):
        query = "with t0 as ( \
                    select osm_id as node_id from raw join changesets on changesets.id = raw.change_id \
                    where EXISTS ( SELECT * from unnest(hashtags) as h where h ~* '^hotosm-project' ) \
                    and osm_type = 'N' \
                    and ST_Intersects(\"geometry\", ST_GeomFromText('POLYGON(({0}))', 4326)) \
                    limit 2000 \
                ), t1 as ( \
                    select osm_id as way_id from raw where \
                \"type\" = 'polygon' \
                and refs && array(select * from t0)::bigint[] \
                ),  \
                t2 AS ( \
                select status, way_id, unnest(refs) AS node_id from raw left join validation on validation.osm_id = raw.osm_id, t1 where raw.osm_id = t1.way_id \
                and raw.type = 'polygon' \
                and tags -> 'building' = 'yes' \
                ), \
                t3 as ( \
                    select status, way_id, node_id, ROW_NUMBER () OVER (ORDER BY NULL) as rn from t2 \
                ), \
                t4 AS ( \
                select status, rn, way_id, node_id, geometry from t3 inner join raw on node_id = osm_id and type = 'point' order by rn desc \
                ), \
                t5 AS ( \
                SELECT  status, way_id, ST_MakePolygon(ST_MakeLine(geometry)) AS polygon FROM t4 group by way_id, status \
                ), \
                t6 as ( \
                    SELECT jsonb_build_object( \
                        'type',       'Feature', \
                        'id',         t5.way_id, \
                        'properties', to_jsonb(t5) - 'polygon' , \
                        'geometry',   ST_AsGeoJSON(polygon)::jsonb \
                ) AS feature from t5 \
                ) SELECT jsonb_build_object( \
                    'type',     'FeatureCollection', \
                    'features', jsonb_agg(t6.feature) \
                ) from t6;".format(area)
        return self.underpassDB.run(query, responseType)

