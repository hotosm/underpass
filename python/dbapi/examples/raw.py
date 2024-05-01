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
    print(
        await rawer.getNodes(raw.ListFeaturesParamsDTO(
            area = "-64.28176188601022 -31.34986467833471,-64.10910770217268 -31.3479682248434,-64.10577675328835 -31.47636641835701,-64.28120672786282 -31.47873373712735,-64.28176188601022 -31.34986467833471",
            tags = "amenity=hospital"
        ), asJson=True)
    )

asyncio.run(main())


