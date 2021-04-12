//
// Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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

// #include "hotosm.hh"
#include "osmstats/changeset.hh"
// #include "osmstats/osmstats.hh"
#include "data/geoutil.hh"
// #include "osmstats/replication.hh"
#include "data/import.hh"
#include "data/threads.hh"
#include "data/underpass.hh"
#include "timer.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace underpass;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace replication {
class StateFile;
};

TestState runtest;

int
main(int argc, char* argv[])
{
    std::string basedir = DATADIR;
    Timer timer;
    
    Underpass under("underpass");
    
    if (under.sdb->is_open()) {
        runtest.pass("Underpass::connect");
    } else {
        runtest.fail("Underpass::connect");
    }

    ptime tstamp = time_from_string("2020-09-18 12:03:29.891418-06");
    auto state = under.getState(replication::minutely, tstamp);
    if (state->path.empty()) {
        runtest.fail("Underpass::getState(minutely)");
    } else {
        runtest.pass("Underpass::getState(minutely)");
    }

    osmstats::QueryOSMStats ostats;                    ///< OSM Stats database access
    ostats.connect("osmstats");
    ptime last = ostats.getLastUpdate();
    
    under.connect("underpass");
    state = under.getState(replication::changeset, last);
    std::cout << "Last minutely is " << last  << std::endl;
    // url = "https://planet.openstreetmap.org/replication/minute/004/308/210";
    // std::thread mstate (threads::startMonitor, state->path);
    // threads::startMonitor(state->path);

    // Changesets have a bounding box, so we want to find the
    // country the changes were made in.
    double min_lat = -2.8042325;
    double min_lon = 29.5842812;
    double max_lat = -2.7699398;
    double max_lon = 29.6012844;

    timer.startTimer();
    auto country = under.getCountry(max_lat, max_lon, min_lat, min_lon);
    // country->dump();
    if (country->id == 191 && country->name == "Rwanda" && country->abbrev == "RW") {
        runtest.pass("Underpass::getCountry()");
    } else {
        runtest.fail("Underpass::getCountry()");
    }
    timer.endTimer();
    
    std::cout << "Done..." << std::endl;
};

