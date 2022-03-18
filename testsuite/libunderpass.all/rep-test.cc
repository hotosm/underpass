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
#include <iostream>
#include <string>
#include <pqxx/pqxx>

#include "hottm.hh"
#include "galaxy/galaxy.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "galaxy/replication.hh"
#include "data/osmobjects.hh"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace galaxy;

TestState runtest;

class TestRep : public replication::Planet
{
public:
    TestRep(void) {
    };
};

int
main(int argc, char *argv[])
{

    TestRep tr;

    // 000 tests
    auto result = tr.findData(replication::changeset, time_from_string("2014-09-07 10:23"));
    std::cout << result << std::endl;
    if (result->path == "000/956/986") {
        runtest.pass("Planet::findData(no state 000)");
    } else {
        runtest.fail("Planet::findData(no state 000)");
    }
    // auto foo = tr.getState(replication::changeset, time_from_string("2014-11-07 10:23"));
    // if (foo->path == "000/962/746") {
    //     runtest.pass("Planet::findData(no state 000)");
    // } else {
    //     runtest.fail("Planet::findData(no state 000)");
    // }

    // 001 tests
    result = tr.findData(replication::changeset, time_from_string("2016-08-04 09:03"));
    if (result->path == "001/958/981") {
        runtest.pass("Planet::findData(no state 001)");
    } else {
        runtest.fail("Planet::findData(no state 001)");
    }

    // 002 tests
    result = tr.findData(replication::changeset, time_from_string("2017-09-07 18:22"));
    if (result->path == "002/532/990") {
        runtest.pass("Planet::findData(has state)");
    } else {
        runtest.fail("Planet::findData(has state)");
    }

    replication::RemoteURL remote("https://planet.openstreetmap.org/replication/changesets/000/956/986");
    // remote.dump();
    if (remote.minor == 956 && remote.index == 986) {
        runtest.pass("RemoteURL(url)");
    } else {
        runtest.fail("RemoteURL(url)");
    }
    remote.Increment();
    // remote.dump();
    if (remote.minor == 956 && remote.index == 987) {
        runtest.pass("RemoteURL.Increment()");
    } else {
        runtest.fail("RemoteURL.Increment()");
    }
    remote.parse("https://planet.openstreetmap.org/replication/changesets/000/956/999");
    remote.Increment();
    // remote.dump();
    if (remote.minor == 957 && remote.index == 0) {
        runtest.pass("RemoteURL.Increment(minor roll-over)");
    } else {
        runtest.fail("RemoteURL.Increment(minor roll-over)");
    }
    remote.parse("https://planet.openstreetmap.org/replication/changesets/000/999/999");
    remote.Increment();
    // remote.dump();
    if (remote.major == 1 && remote.minor == 0 && remote.index == 1) {
        runtest.pass("RemoteURL.Increment(major roll-over)");
    } else {
        runtest.fail("RemoteURL.Increment(major roll-over)");
    }
    
    // if (tr.checkTag("building", "yes") == true) {
    //     runtest.pass("Validate::checkTag(good tag)");
    // } else {
    //     runtest.fail("Validate::checkTag(good tag)");
    // }

}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
