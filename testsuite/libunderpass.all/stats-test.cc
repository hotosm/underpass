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

#include <dejagnu.h>
#include "galaxy/osmchange.hh"
#include <boost/geometry.hpp>
#include <iostream>
#include "log.hh"

using namespace logger;

class TestOsmChanges : public osmchange::OsmChangeFile {
};

TestState runtest;

int
main(int argc, char *argv[]) {

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("stats-test.log");
    dbglogfile.setVerbosity(3);

    std::string test_data_dir(DATADIR);
    test_data_dir += "/testsuite/testdata/";
    TestOsmChanges osmchanges;
    osmchanges.readChanges(test_data_dir + "test_stats.osc");
    
    multipolygon_t poly;
    boost::geometry::read_wkt("MULTIPOLYGON(((-180 90,180 90, 180 -90, -180 -90,-180 90)))", poly);

    auto stats = osmchanges.collectStats(poly);    
    auto changestats = std::begin(*stats)->second;

    if (changestats->added.at("highway") == 3) {
        runtest.pass("Calculating added (created) highways");
    } else {
        runtest.fail("Calculating added (created) highways");
    }   

    if (changestats->modified.at("highway") == 2) {
        runtest.pass("Calculating modified highways");
    } else {
        runtest.fail("Calculating modified highways");
    }

    if (changestats->added.at("building") == 1) {
        runtest.pass("Calculating added (created) buildings");
    } else {
        runtest.fail("Calculating added (created) buildings");
    }   

    if (changestats->modified.at("building") == 1) {
        runtest.pass("Calculating modified buildings");
    } else {
        runtest.fail("Calculating modified buildings");
    }   

    if (changestats->added.at("waterway") == 1) {
        runtest.pass("Calculating added (created) waterways");
    } else {
        runtest.fail("Calculating added (created) waterways");
    }   

    if (changestats->modified.at("waterway") == 1) {
        runtest.pass("Calculating modified waterways");
    } else {
        runtest.fail("Calculating modified waterways");
    }   


}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
