//
// Copyright (c) 2023 Humanitarian OpenStreetMap Team
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

#include <dejagnu.h>
#include <iostream>
#include "validate/validate.hh"
#include "validate/defaultvalidation.hh"
#include "osm/osmobjects.hh"
#include "osm/osmchange.hh"
#include "utils/log.hh"

using namespace logger;

TestState runtest;
class TestOsmChange : public osmchange::OsmChangeFile {};

typedef std::shared_ptr<Validate>(plugin_t)();

osmobjects::OsmWay readOsmWayFromFile(std::string filename) {
    TestOsmChange osmchange;
    std::string filespec = DATADIR;
    filespec += filename;
    if (boost::filesystem::exists(filespec)) {
        osmchange.readChanges(filespec);
    } else {
        log_debug("Couldn't load ! %1%", filespec);
    };
    osmchange.buildGeometriesFromNodeCache();
    return *osmchange.changes.front().get()->ways.front().get();
}

int
main(int argc, char *argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("val-unsquared-test.log");
    dbglogfile.setVerbosity(3);

     auto plugin = std::make_shared<defaultvalidation::DefaultValidation>();

    // Squared building 1
    auto way = readOsmWayFromFile("/testsuite/testdata/validation/squared-building-1.osm");
    auto status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badgeom)) {
        runtest.pass("Validate::checkWay(no badgeom) [squared building 1]");
    } else {
        runtest.fail("Validate::checkWay(no badgeom) [squared building 1]");
        return 1;
    }

    // Squared building 2
    way = readOsmWayFromFile("/testsuite/testdata/validation/squared-building-2.osm");
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badgeom)) {
        runtest.pass("Validate::checkWay(no badgeom) [squared building 2]");
    } else {
        runtest.fail("Validate::checkWay(no badgeom) [squared building 2]");
        return 1;
    }

    // Squared building 3
    way = readOsmWayFromFile("/testsuite/testdata/validation/squared-building-3.osm");
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badgeom)) {
        runtest.pass("Validate::checkWay(no badgeom) [squared building 3]");
    } else {
        runtest.fail("Validate::checkWay(no badgeom) [squared buildin 3]");
        return 1;
    }

    // Un-squared building 1
    way = readOsmWayFromFile("/testsuite/testdata/validation/unsquared-building-1.osm");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(badgeom)) {
        runtest.pass("Validate::checkWay(badgeom) [unsquared building 1]");
    } else {
        runtest.fail("Validate::checkWay(badgeom) [unsquared building 1]");
        return 1;
    }

    // Un-squared building 2
    way = readOsmWayFromFile("/testsuite/testdata/validation/unsquared-building-2.osm");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(badgeom)) {
        runtest.pass("Validate::checkWay(badgeom) [unsquared building 2]");
    } else {
        runtest.fail("Validate::checkWay(badgeom) [unsquared building 2]");
        return 1;
    }

    // Un-squared building 3
    way = readOsmWayFromFile("/testsuite/testdata/validation/unsquared-building-3.osm");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(badgeom)) {
        runtest.pass("Validate::checkWay(badgeom) [unsquared building 3]");
    } else {
        runtest.fail("Validate::checkWay(badgeom) [unsquared building 3]");
        return 1;
    }

}
