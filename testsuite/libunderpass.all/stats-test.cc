//
// Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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
#include <string>
#include <pqxx/pqxx>

#include "hottm.hh"
#include "osmstats/osmstats.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "osmstats/osmstats.hh"
#include "timer.hh"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace osmstats;

TestState runtest;

class TestStats : public QueryOSMStats
{
};

int
main(int argc, char *argv[])
{
    std::string database = "osmstats";

    TestStats testos;
    Timer timer;
    if (testos.connect(database)) {
        runtest.pass("taskingManager::connect()");
    } else {
        runtest.fail("taskingManager::connect()");
    }

    //testos.populate();
    std::vector<long> cids;
    cids.push_back(57293600);
    cids.push_back(77274475);
    cids.push_back(69360891);
    cids.push_back(69365434);
    cids.push_back(69365911);

    timer.startTimer();

// FIXME: test disabled after https://github.com/hotosm/underpass/commit/37046c5430f5701b8e1c5c2a9a8e7c8e24163bc7
#if 0
    testos.getRawChangeSets(cids);
    std::cout << "Operation took " << timer.endTimer() << " milliseconds" << std::endl;

    // modify a counter
    // osmstats::OsmStats os = testos[2];
    // os.updateCounter("roads_added", 12345);
    // if (os["roads_added"] == 12345) {
    //     runtest.pass("OsmStats::updateCounter()");
    // } else {
    //     runtest.fail("OsmStats::updateCounter()");
    // }
    // testos.endTimer();
    //testos.dump();

    // testos.updateRawHashtags("/work/Mapping/HOT/changesets-reduced.osm");
    // testos.updateRawUsers("/work/Mapping/HOT/changesets-reduced.osm");

    // extract some data from the database to store in memory for
    // better performance
    timer.startTimer();
    testos.populate();
    timer.endTimer();


    // Test if the Country data works
    RawCountry rc = testos.getCountryData(73);
    if (rc.id == 73) {
        runtest.pass("Country ID");
    } else {
        runtest.fail("Country ID");
    }

    if (rc.name == "Belize") {
        runtest.pass("Country Name");
    } else {
        runtest.fail("Country Name");
    }

    if (rc.abbrev == "BLZ") {
        runtest.pass("Country Code");
    } else {
        runtest.fail("Country Code");
    }

    // Test if the User data works
    RawUser ru = testos.getUserData(154981);
    if (ru.id == 154981) {
        runtest.pass("User ID");
    } else {
        runtest.fail("User ID");
    }
    if (ru.name == "Bogmen") {
        runtest.pass("User Name");
    } else {
        runtest.fail("User Name");
    }

    long id = testos.getHashtagID("logging-roads-6");
    if (id == 73) {
        runtest.pass("Hashtag ID");
    } else {
        runtest.fail("Hashtag ID");
    }

    // Changesets have a bounding box, so we want to find the
    // country the changes were made in.
    double min_lat = -2.8042325;
    double min_lon = 29.5842812;
    double max_lat = -2.7699398;
    double max_lon = 29.6012844;
#endif
}
