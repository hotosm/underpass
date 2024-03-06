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

#include <dejagnu.h>
#include <iostream>
#include <string>
#include <pqxx/pqxx>

#include "osm/changeset.hh"
#include "utils/geoutil.hh"
#include "replicator/threads.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
// #include <boost/timer/timer.hpp>

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

    Underpass under("underpass");

    if (under.sdb->is_open()) {
        runtest.pass("Underpass::connect");
    } else {
        runtest.fail("Underpass::connect");
        return 1;
    }

    ptime tstamp = time_from_string("2014-09-18 12:03:29");
    auto state = under.getState(replication::changeset, tstamp);
    // state->dump();
    if (state->sequence > 0) {
        if (state->path.compare("https://planet.openstreetmap.org/replication/changesets/000/972/927")) {
            runtest.fail("Underpass::getState(changeset)");
            return 1;
        } else {
            runtest.pass("Underpass::getState(changeset)");
        }

        tstamp = time_from_string("2014-09-18 14:03:29");
        state = under.getState(replication::minutely, tstamp);
        state->dump();
        if (state->path.compare("https://planet.openstreetmap.org/replication/minute/001/053/674")) {
            runtest.fail("Underpass::getState(minutely)");
            return 1;
        } else {
            runtest.pass("Underpass::getState(minutely)");
        }
    } else {
        if (state->path.compare("https://planet.openstreetmap.org/replication/changesets/000/972/927")) {
            runtest.xfail("Underpass::getState(changeset)");
        } else {
            runtest.pass("Underpass::getState(changeset)");
        }

        tstamp = time_from_string("2014-09-18 14:03:29");
        state = under.getState(replication::minutely, tstamp);
        state->dump();
        if (state->path.compare("https://planet.openstreetmap.org/replication/minute/001/053/674")) {
            runtest.xfail("Underpass::getState(minutely)");
        } else {
            runtest.pass("Underpass::getState(minutely)");
        }
    }

    std::cout << "Done..." << std::endl;
};


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
