//
// Copyright (c) 2020, Humanitarian OpenStreetMap Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

    testos.startTimer();
    testos.getRawChangeSet(cids);
    std::cout << "Operation took " << testos.endTimer() << " milliseconds" << std::endl;
    testos.dump();
    
    // testos.updateRawHashtags("/work/Mapping/HOT/changesets-reduced.osm");
    // testos.updateRawUsers("/work/Mapping/HOT/changesets-redu ced.osm");

    // extract some data from the database to store in memory for
    // better performance
    testos.startTimer();
    testos.populate();
    std::cout << "Operation took " << testos.endTimer() << " milliseconds" << std::endl;

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
}
