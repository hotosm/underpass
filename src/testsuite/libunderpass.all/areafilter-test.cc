//
// Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
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
#include "osm/changeset.hh"
#include "osm/osmchange.hh"
#include <boost/geometry.hpp>
#include "utils/geoutil.hh"

TestState runtest;

using namespace logger;

class TestChangeset : public changesets::ChangeSetFile {};
class TestOsmChange : public osmchange::OsmChangeFile {};

int
countFeatures(TestOsmChange &osmchange) {
    int nodeCount = 0;
    int wayCount = 0;
    int relCount = 0;
    for (auto cit = std::begin(osmchange.changes); cit != std::end(osmchange.changes); ++cit) {
        osmchange::OsmChange *testOsmChange = cit->get();
        for (auto nit = std::begin(testOsmChange->nodes); nit != std::end(testOsmChange->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
            if (node->priority) {
                nodeCount++;
            }
        }
        for (auto wit = std::begin(testOsmChange->ways); wit != std::end(testOsmChange->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            if (way->priority) {
                wayCount++;
            }
        }
        for (auto rit = std::begin(testOsmChange->relations); rit != std::end(testOsmChange->relations); ++rit) {
            osmobjects::OsmRelation *relation = rit->get();
            if (relation->priority) {
                relCount++;
            }
        }
    }
    return nodeCount + wayCount + relCount;
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
            }
        }
        for (auto wit = std::begin(testOsmChange->ways); wit != std::end(testOsmChange->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            if (!way->priority) {
                result = false;
            }
        }
    }
    return result;
}

void
clearChanges(TestOsmChange &osmchange) {
    osmchange.changes.clear();
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
    multipolygon_t polyHalfSmall;
    boost::geometry::read_wkt("MULTIPOLYGON(((91.08695983886719 25.195485830324174,91.08697056770325 25.192155906163805,91.08929872512817 25.192126781061106,91.08922362327574 25.195505246524604,91.08695983886719 25.195485830324174)))", polyHalfSmall);
    multipolygon_t polyEmpty;

    // -- ChangeSets

    // Small changeset in Bangladesh
    std::string changesetFile(DATADIR);
    changesetFile += "/testsuite/testdata/areafilter-test.osc";
    std::string osmchangeFile(DATADIR);
    osmchangeFile += "/testsuite/testdata/areafilter-test.osm";
    TestChangeset changeset;
    TestOsmChange osmchange;
    changesets::ChangeSet *testChangeset;

    // ChangeSet - Whole world
    changeset.readChanges(changesetFile);
    changeset.areaFilter(polyWholeWorld);
    testChangeset = changeset.changes.front().get();
    if (testChangeset && testChangeset->priority) {
        runtest.pass("ChangeSet areaFilter - true (whole world)");
    } else {
        runtest.fail("ChangeSet areaFilter - true (whole world)");
        return 1;
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
        return 1;
    }

    // ChangeSet - Half area
    changeset.readChanges(changesetFile);
    changeset.areaFilter(polyHalf);
    testChangeset = changeset.changes.front().get();
    if (testChangeset && testChangeset->priority) {
        runtest.pass("ChangeSet areaFilter - true (half area)");
    } else {
        runtest.fail("ChangeSet areaFilter - true (half area)");
        return 1;
    }

    // -- OsmChanges

    // OsmChange - Empty poly
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polyEmpty);
    if (getPriority(osmchange) && countFeatures(osmchange) == 62) {
        runtest.pass("OsmChange areaFilter - true (Empty poly)");
    } else {
        runtest.fail("OsmChange areaFilter - true (Empty poly)");
        return 1;
    }

    // OsmChange - Whole world
    osmchange.buildGeometriesFromNodeCache();
    osmchange.areaFilter(polyWholeWorld);
    if (getPriority(osmchange) && countFeatures(osmchange) == 62) {
        runtest.pass("OsmChange areaFilter - true (whole world)");
    } else {
        runtest.fail("OsmChange areaFilter - true (whole world)");
        return 1;
    }

    // OsmChange - Small area
    clearChanges(osmchange);
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polyHalf);
    if (countFeatures(osmchange) == 35) {
        runtest.pass("OsmChange areaFilter - true (small area)");
    } else {
        runtest.fail("OsmChange areaFilter - true (small area)");
        return 1;
    }

    // OsmChange - Smaller area
    clearChanges(osmchange);
    osmchange.readChanges(osmchangeFile);
    osmchange.areaFilter(polyHalfSmall);
    if (countFeatures(osmchange) == 17) {
        runtest.pass("OsmChange areaFilter - true (smaller area)");
    } else {
        runtest.fail("OsmChange areaFilter - true (smaller area)");
        return 1;
    }


}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
