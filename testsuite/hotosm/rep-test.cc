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

#include "hottm.hh"
#include "osmstats/osmstats.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "osmstats/replication.hh"
#include "data/osmobjects.hh"
#include "timer.hh"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace osmstats;

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
    Timer timer;

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
