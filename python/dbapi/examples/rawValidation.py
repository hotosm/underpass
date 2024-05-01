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

import sys,os
import asyncio
sys.path.append(os.path.realpath('..'))

from api import rawValidation
from api.db import DB

async def main():

    db = DB()
    await db.connect()
    rawval = rawValidation.RawValidation(db)

    # # Get validation count
    # print(
    #     await rawval.getCount(rawValidation.ValidationCountParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "building=yes",
    #         status = rawValidation.ValidationError.badgeom,
    #         table = rawValidation.Table.polygons,
    #     ), asJson = True)
    # )

    # Get Raw Validation OSM features for all geometries (as JSON)
    # print(
    #     await rawval.getFeatures(rawValidation.RawValidationFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "building=yes",
    #         status = rawValidation.ValidationError.badgeom
    #     ), asJson=True)
    # )

    # Get List of Raw Validation OSM features for Nodes
    print(
        await rawval.getPolygonsList(rawValidation.ListValidationFeaturesParamsDTO(
            area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
            tags = "building=yes",
            status = rawValidation.ValidationError.badgeom
        ))
    )

asyncio.run(main())


