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

from api import raw
from api.db import DB

async def main():

    db = DB()
    await db.connect()
    rawer = raw.Raw(db)

    # Get List of OSM features for Nodes
    # print(
    #     await rawer.getNodesList(raw.ListFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "building=yes",
    #     ))
    # )

    # # Get List of OSM features for Nodes (as JSON)
    # print(
    #     await rawer.getNodesList(raw.ListFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "building=yes",
    #     ), asJson = True)
    # )

    # Get List of OSM features for Lines
    # print(
    #     await rawer.getLinesList(raw.ListFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "highway",
    #     ))
    # )

    # Get List of OSM features for Polygons
    # print(
    #     await rawer.getPolygonsList(raw.ListFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "building=yes",
    #     ))
    # )

    # Get List of OSM features for all geometries
    # print(
    #     await rawer.getList(raw.ListFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "building=yes",
    #     ))
    # )

    # Get Raw OSM features for Nodes
    # print(
    #     await rawer.getNodes(raw.RawFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "man_made=yes"
    #     ))
    # )

    # Get Raw OSM features for Lines
    # print(
    #     await rawer.getLines(raw.RawFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "highway"
    #     ))
    # )

    # Get Raw OSM features for Nodes
    # print(
    #     await rawer.getNodes(raw.RawFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "man_made=yes"
    #     ))
    # )

    # Get Raw OSM features for Polygons
    # print(
    #     await rawer.getPolygons(raw.RawFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "man_made=yes"
    #     ), asJson=True)
    # )

    # Get Raw OSM features for Nodes (as JSON)
    print(
        await rawer.getNodes(raw.RawFeaturesParamsDTO(
            area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
            tags = "man_made=yes"
        ), asJson=True)
    )

    # Get Raw OSM features for all geometries (as JSON)
    # print(
    #     await rawer.getFeatures(raw.RawFeaturesParamsDTO(
    #         area = "-180 90,180 90, 180 -90, -180 -90,-180 90",
    #         tags = "man_made=yes"
    #     ), asJson=True)
    # )

asyncio.run(main())


