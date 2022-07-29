//
// Copyright (c) 2020, 2021, 2022 Humanitarian OpenStreetMap Team
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
#include <dejagnu.h>
#include "galaxy/changeset.hh"
#include "galaxy/osmchange.hh"
#include <boost/geometry.hpp>
#include "data/geoutil.hh"

TestState runtest;

using namespace logger;

class TestChangeset : public changeset::ChangeSetFile {};
class TestOsmChange : public osmchange::OsmChangeFile {};

int
countFeatures(TestOsmChange &osmchange) {
    int count = 0;
    for (auto cit = std::begin(osmchange.changes); cit != std::end(osmchange.changes); ++cit) {
        osmchange::OsmChange *testOsmChange = cit->get();
        count += testOsmChange->nodes.size();
        count += testOsmChange->ways.size();
    }
    return count;
}

int
countFeatures(TestChangeset &changeset) {
    return changeset.changes.size();
}

bool
getPriority(TestOsmChange &osmchange, bool debug = false) {
    bool result = true;
    for (auto cit = std::begin(osmchange.changes); cit != std::end(osmchange.changes); ++cit) {
        osmchange::OsmChange *testOsmChange = cit->get();
        for (auto nit = std::begin(testOsmChange->nodes); nit != std::end(testOsmChange->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
            if (!node->priority) {
                result = false;
                if (debug) {
                    std::cout << "node: false" << std::endl;
                }
            }
        }
        for (auto wit = std::begin(testOsmChange->ways); wit != std::end(testOsmChange->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            if (!way->priority) {
                result = false;
                if (debug) {
                    std::cout << "way: false" << std::endl;
                }
            }
        }
    }
    return result;
}

int
main(int argc, char *argv[])
{
    multipolygon_t polyWholeWorld;
    boost::geometry::read_wkt("MULTIPOLYGON(((-180 90,180 90, 180 -90, -180 -90,-180 90)))", polyWholeWorld);
    multipolygon_t polySmallArea;
    boost::geometry::read_wkt("MULTIPOLYGON (((20 35, 45 20, 30 5, 10 10, 10 30, 20 35)))", polySmallArea);
    multipolygon_t polyHalf;
    boost::geometry::read_wkt("MULTIPOLYGON(((91.08473230447439 25.195528629552243,91.08475247411987 25.192143075605387,91.08932089882008 25.192152201214213,91.08927047470638 25.195501253482632,91.08473230447439 25.195528629552243)))", polyHalf);
    multipolygon_t polyEmpty;

    // Small changeset in Bangladesh
    std::string changesetFile(DATADIR);
    changesetFile += "/testsuite/testdata/areafilter-test.osc";
    std::string osmchangeFile(DATADIR);
    osmchangeFile += "/testsuite/testdata/areafilter-test.osm";
    TestChangeset changeset;
    TestOsmChange osmchange;
    changeset::ChangeSet *testChangeset;

    // ChangeSet - Whole world
    changeset.readChanges(changesetFile);
    changeset.areaFilter(polyWholeWorld);
    testChangeset = changeset.changes.front().get();
    if (testChangeset && testChangeset->priority) {
        runtest.pass("ChangeSet areaFilter - true (whole world)");
    } else {
        runtest.fail("ChangeSet areaFilter - true (whole world)");
    }

    // ChangeSet - Small area in North Africa
    changeset.readChanges(changesetFile);
    changeset.areaFilter(polySmallArea);
    testChangeset = changeset.changes.front().get();
    if (testChangeset && testChangeset->priority) {
        runtest.fail("ChangeSet areaFilter - false (small area)");
    } else {
        runtest.pass("ChangeSet areaFilter - false (small area)");
    }

    // ChangeSet - Empty polygon
    changeset.readChanges(changesetFile);
    changeset.areaFilter(polyEmpty);
    testChangeset = changeset.changes.front().get();
    if (testChangeset && testChangeset->priority) {
        runtest.pass("ChangeSet areaFilter - true (empty)");
    } else {
        runtest.fail("ChangeSet areaFilter - true (empty)");
    }

    // ChangeSet - Half area
    changeset.readChanges(changesetFile);
    changeset.areaFilter(polyHalf);
    testChangeset = changeset.changes.front().get();
    if (testChangeset && testChangeset->priority) {
        runtest.pass("ChangeSet areaFilter - true (half area)");
    } else {
        runtest.fail("ChangeSet areaFilter - true (half area)");
    }

    // OsmChange - Small area in North Africa
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polySmallArea);
    if (countFeatures(osmchange) == 0) {
        runtest.pass("OsmChange areaFilter - false (small area)");
    } else {
        runtest.fail("OsmChange areaFilter - false (small area)");
    }

    // OsmChange - Whole world
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polyWholeWorld);
    if (getPriority(osmchange) && countFeatures(osmchange) == 48) {
        runtest.pass("OsmChange areaFilter - true (whole world)");
    } else {
        runtest.fail("OsmChange areaFilter - true (whole world)");
    }
    // Delete all changes
    osmchange.areaFilter(polySmallArea);

    // OsmChange - Empty polygon
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polyEmpty);
    if (getPriority(osmchange) && countFeatures(osmchange) == 48) {
        runtest.pass("OsmChange areaFilter - true (empty)");
    } else {
        runtest.fail("OsmChange areaFilter - true (empty)");
    }
    // Delete all changes
    osmchange.areaFilter(polySmallArea);

    // OsmChange - Half area
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polyHalf);
    if (getPriority(osmchange, true) && countFeatures(osmchange) == 28) {
        runtest.pass("OsmChange areaFilter - true (half area)");
    } else {
        runtest.fail("OsmChange areaFilter - true (half area)");
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
