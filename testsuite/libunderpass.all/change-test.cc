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
#include "log.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace logger;
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
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("change-test.log");
    dbglogfile.setVerbosity(3);

    std::string basedir = DATADIR;

    // Read the changeset state file
    TestStateFile statefile(basedir + "/993.state.txt", false);
    // statefile.dump();
    if (statefile.getSequence() == 4517993 && to_simple_string(statefile.getTimestamp()) == "2021-Apr-28 03:03:09") {
        runtest.pass("StateFile::Statefile(disk)");
    } else {
        runtest.fail("StateFile::Statefile(disk)");
    }

    std::string buf = "---\nlast_run: 2020-10-08 22:30:01.737719000 +00:00\nsequence: 4139993\n";
    TestStateFile mem(buf, true);
    // mem.dump();
    if (mem.getSequence() == 4139993 && to_simple_string(statefile.getTimestamp()) == "2021-Apr-28 03:03:09") {
        runtest.pass("StateFile::Statefile(buffer)");
    } else {
        runtest.fail("StateFile::Statefile(buffer)");
    }

    TestCO testco;
    // testco.readChanges("/tmp/y");
    testco.readChanges(basedir + "/123.osc");
    // testco.dump();
    if (testco.changes.size() >= 1) {
        runtest.pass("ChangeSetFile::readChanges(parsed file)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(parsed file)");
    }

    auto tf = testco.changes.front();
    auto tnf = tf->nodes.front();
    if (tnf->change_id == 99069702 && tnf->id == 5776216755) {
        runtest.pass("ChangeSetFile::readChanges(first change, first node)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(first change, first node)");
    }
    auto twf = tf->ways.front();
    // twf->dump();
    if (twf->change_id ==  99069879 && twf->id == 474556695 &&  twf->uid == 1041828) {
        runtest.pass("ChangeSetFile::readChanges(first change, first way)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(first change, first way)");
    }
    auto tnb = tf->nodes.back();
    // tnb->dump();
    if (tnb->change_id == 94802322 && tnb->id == 289112823) {
        runtest.pass("ChangeSetFile::readChanges(first change, last node)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(first change, last node)");
    }
    auto twb = tf->ways.back();
    // twb->dump();
    if (twb->change_id == 99063443 && twb->id == 67365141 &&  twb->uid == 1137406) {
        runtest.pass("ChangeSetFile::readChanges(first change, last way)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(first change, last way)");
    }
    
    auto tb = testco.changes.back();
    // tb->dump();
    if (tb) {
        runtest.pass("ChangeSetFile::readChanges(last change)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(last change)");
    }
    
#if 0
    tests.readChanges(basedir + "/changeset-data.osm");
    tests.dump();
    
    // if (tests[3].id > 0) {
    //     runtest.pass("ChangeSetFile::readChanges(uncompressed)");
    // } else {
    //     runtest.fail("ChangeSetFile::readChanges(uncompressed)");
    // }
    auto rc = tests.readChanges(basedir + "/changeset-data2.osm.gz");
    tests.dump();
    // if (tests[3].id > 0) {
    //     runtest.pass("ChangeSetFile::readChanges(compressed)");
    // } else {
    //     runtest.fail("ChangeSetFile::readChanges(compressed)");
    // }

#endif
    
    std::cout << "Done..." << std::endl;
};

