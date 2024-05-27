//
// Copyright (c) 2020, 2021, 2022, 2023, 2024 Humanitarian OpenStreetMap Team
//
// This file is part of Underpass.
//
//     Underpass is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Underpass is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.
//

#include <iostream>
#include "raw/queryraw.hh"
#include "osm/osmobjects.hh"
#include <boost/date_time.hpp>

using namespace queryraw;
using namespace osmobjects;

logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();

int main() {

    dbglogfile.setVerbosity();

    auto pq = std::make_shared<Pq>();
    pq->connect("colorado");
    auto sqlprep = std::make_shared<SQLPrep>(pq);
    std::vector<osmobjects::OsmNode> nodes;

    OsmNode node1;
    node1.id = 11111;
    ptime now  = boost::posix_time::second_clock::universal_time();

    // node1.timestamp =
    node1.changeset = 22222;
    node1.timestamp = now;
    node1.addTag("place", "city");
    node1.addTag("name", "Nederland");
    node1.user = "testuser";
    boost::geometry::read_wkt("POINT (30 10)", node1.point);
    nodes.push_back(node1);

    OsmNode node2;
    node2.id = 3333;
    node2.changeset = 4444;
    node1.timestamp = now;
    node2.addTag("place", "city");
    node2.addTag("name", "Rollinsville");
    node2.user = "test user";
    boost::geometry::read_wkt("POINT (40 20)", node2.point);
    nodes.push_back(node2);

    if (sqlprep->insert_entries(nodes)) {
        std::cerr << "Inserted a node" << std::endl;
    } else {
        std::cerr << "Couldn't inserted a node" << std::endl;
    }

#if 0
    auto delnode = InsertObject(node);
    delnode.dump();
    delnode.buildQuery("nodes");
    OsmWay way;
    std::vector<int> refs = {1, 22, 333, 4444};
    way.id = 33333;
    way.changeset = 44444;
    way.addTag("building", "yes");

    auto delway = InsertObject(way);
    delway.dump();
    delway.buildQuery("way_poly");

    return 0;
#endif
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
