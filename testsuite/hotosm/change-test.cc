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
#include <cmath>

#include "data/geoutil.hh"
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
    Timer timer;

#if 0
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
    auto rc = tests.readChanges(basedir + "/changeset-data2.osm.gz");
    std::cout << "Operation readChanges(compressed) took " << timer.endTimer() << " milliseconds" << std::endl;
    if (tests[3].id > 0) {
        runtest.pass("ChangeSetFile::readChanges(compressed)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(compressed)");
    }
#endif
    TestCO testco;
    timer.startTimer();
    // testco.readChanges("/tmp/y");
    testco.readChanges(basedir + "/123.osc");
    std::cout << "Operation read osc took " << timer.endTimer() << " milliseconds" << std::endl;

    if (testco.changes.size() >= 3) {
        runtest.pass("ChangeSetFile::readChanges()");
    } else {
        runtest.fail("ChangeSetFile::readChanges()");
    }
    
    testco.dump();
    timer.startTimer();
    // Create 
    // 2 hospital nodes, 1 hospital way
    // 2 school nodes, 1 school way
    // 1 place way
    //
    // Modify
    // 2 place nodes get name added
    std::string testdata = TOPSRCDIR;
    testdata += "/data/priority.geojson";
    geoutil::GeoUtil geou;
    geou.readFile(testdata);

    auto stats = testco.collectStats(geou.boundary);
    std::cout << "Operation collect stats took " << timer.endTimer() << " milliseconds" << std::endl;

#if 0
    if (stats->added["school"] == 3) {
        runtest.pass("school added");
    } else {
        runtest.fail("schools added");
    }
#endif
    std::map<std::string, int> hits;
    for (auto it = std::begin(*stats); it != std::end(*stats); ++it) {
        it->second->dump();
        // New feature statistics
        std::map<std::string, int> tmp = it->second->added;
        for (auto sit = std::begin(tmp); sit != std::end(tmp); ++sit) {
            hits[sit->first] += sit->second;
        }
    }

    if (hits["city"] == 1) {
        runtest.pass("ChangeSetFile::collectStats(cities added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(cities added)");
    }

    if (hits["town"] == 0) {
        runtest.pass("ChangeSetFile::collectStats(town added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(town added)");
    }

    // hamlet is only in modified, so this should be 0
    if (hits["hamlet"] == 0) {
        runtest.pass("ChangeSetFile::collectStats(hamlet added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(hamlet added)");
    }

    if (hits["village"] == 1) {
        runtest.pass("ChangeSetFile::collectStats(village added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(village added)");
    }

    if (hits["school"] == 3) {
        runtest.pass("ChangeSetFile::collectStats(schools added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(schools added)");
    }

    if (hits["hospital"] == 3) {
        runtest.pass("ChangeSetFile::collectStats(hospitals added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(hospitals added)");
    }

    if (hits["building"] == 8) {
        runtest.pass("ChangeSetFile::collectStats(buildings added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(buildings added)");
    }

    if (hits["place"] == 2) {
        runtest.pass("ChangeSetFile::collectStats(places added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(places added)");
    }
    
    if (hits["highway"] == 1) {
        runtest.pass("ChangeSetFile::collectStats(highway added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(highway added)");
    }

    if (hits["waterway"] == 1) {
        runtest.pass("ChangeSetFile::collectStats(waterway added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(waterway added)");
    }

    // distance are in meters, not kilometers for better accuracy
    if (hits["highway_km"] == 4236) {
        runtest.pass("ChangeSetFile::collectStats(highway length added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(highway length added)");
    }

    if (hits["waterway_km"] == 1980) {
        runtest.pass("ChangeSetFile::collectStats(waterway length added)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(waterway length added)");
    }
    
    for (auto it = std::begin(hits); it != std::end(hits); ++it) {
        std::cout << "Created: " << it->first << " = " <<  it->second << std::endl;
    }

    std::map<std::string, int> mods;
    for (auto it = std::begin(*stats); it != std::end(*stats); ++it) {
        it->second->dump();
        // New feature statistics
        std::map<std::string, int> tmp = it->second->modified;
        for (auto sit = std::begin(tmp); sit != std::end(tmp); ++sit) {
            mods[sit->first] += sit->second;
        }
    }
    if (mods["place"] == 4) {
        runtest.pass("ChangeSetFile::collectStats(place name modified)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(place name modified)");
    }

    if (mods["village"] == 1) {
        runtest.pass("ChangeSetFile::collectStats(village name modified)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(village name modified)");
    }

    if (mods["town"] == 1) {
        runtest.pass("ChangeSetFile::collectStats(town name modified)");
    } else {
        runtest.fail("ChangeSetFile::collectStats(town name modified)");
    }

    for (auto it = std::begin(mods); it != std::end(mods); ++it) {
        std::cout << "Modified: " << it->first << " = " <<  it->second << std::endl;
    }

    std::cout << "Done..." << std::endl;
};

