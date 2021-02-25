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
#include "osmstats/changeset.hh"
#include "osmstats/osmchange.hh"
#include "osmstats/replication.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace changeset;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestCS : public changeset::ChangeSetFile
{
    bool testFile(const std::string &filespec) {
        // std::string basedir="DATADIR";
    };
    bool testMem(const std::string &data);
};

class TestCO : public osmchange::OsmChangeFile
{
    bool testFile(const std::string &filespec) {
        // std::string basedir="DATADIR";
    };
    bool testMem(const std::string &data);
};

class TestStateFile : public replication::StateFile
{
public:
    TestStateFile(const std::string &file, bool memory) : replication::StateFile(file, memory) {
    };
    // Accessors for the private data
    ptime getTimestamp(void) { return timestamp; };
    long getSequence(void) { return sequence; };
};

int
main(int argc, char* argv[])
{
    std::string basedir = DATADIR;

    // Read the changeset state file
    std::string buf = "---\nlast_run: 2020-10-08 22:30:01.737719000 +00:00\nsequence: 4139993\n";
    TestStateFile statefile(basedir + "/993.state.txt", false);
    if (statefile.getSequence() == 4139992) {
        runtest.pass("Changeset state file from disk");
    } else {
        runtest.fail("Changeset state file from disk");
    }

    TestStateFile mem(buf, true);
    if (mem.getSequence() == 4139993) {
        runtest.pass("Changeset state file from memory");
    } else {
        runtest.fail("Changeset state file from memory");
    }

    // Read a change file state file
    TestStateFile minstate(basedir + "/996.state.txt", false);
    if (minstate.getSequence() == 4230996) {
        runtest.pass("Change file state file from disk");
    } else {
        runtest.fail("Change file state file from disk");
    }

    Timer timer;
    TestCS tests;
    // auto geou = std::make_shared<geoutil::GeoUtil>();

    // geou->readFile(basedir + "/../../data/geoboundaries.osm", true);
    // tests.setupBoundaries(geou);            

    // tests.importChanges(basedir + "/foo.osm");
    std::cout << "Operation took " << timer.endTimer() << " milliseconds" << std::endl;

    timer.startTimer();
    tests.readChanges(basedir + "/changeset-data.osm");
    std::cout << "Operation readChanges(uncompressed) took " << timer.endTimer() << " milliseconds" << std::endl;
    if (tests[3].id > 0) {
        runtest.pass("ChangeSetFile::readChanges(uncompressed)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(uncompressed)");
    }
    timer.startTimer();
    tests.readChanges(basedir + "/changeset-data2.osm.gz");
    std::cout << "Operation readChanges(compressed) took " << timer.endTimer() << " milliseconds" << std::endl;
    if (tests[3].id > 0) {
        runtest.pass("ChangeSetFile::readChanges(compressed)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(compressed)");
    }

    TestCO testco;
    timer.startTimer();
    // testco.readChanges("/tmp/y");
    testco.readChanges(basedir + "/294.osc.gz");
    std::cout << "Operation read osc took " << timer.endTimer() << " milliseconds" << std::endl;
    testco.dump();
    timer.startTimer();
    testco.collectStats();
    std::cout << "Operation collect stats took " << timer.endTimer() << " milliseconds" << std::endl;

    // if (timer.connect("mystats")) {
    //     runtest.pass("QueryOsmStats::connect()");
    // } else {
    //     runtest.fail("QueryOsmStats::connect()");
    // }

    std::cout << "Done..." << std::endl;
};

