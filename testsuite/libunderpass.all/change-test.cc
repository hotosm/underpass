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

#include <cmath>
#include <dejagnu.h>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "data/geoutil.hh"
#include "log.hh"
#include "osmstats/changeset.hh"
#include "osmstats/osmchange.hh"
#include "osmstats/osmstats.hh"
#include "osmstats/replication.hh"

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time.hpp>
#include <boost/geometry.hpp>

using namespace logger;
using namespace changeset;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestCS : public changeset::ChangeSetFile
{
};

class TestCO : public osmchange::OsmChangeFile
{
};

class TestStateFile : public replication::StateFile
{
  public:
    TestStateFile(const std::string &file, bool memory)
        : replication::StateFile(file, memory){};
    // Accessors for the private data
    ptime getTimestamp(void) { return timestamp; };
    long getSequence(void) { return sequence; };
};

int
main(int argc, char *argv[]) {
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("change-test.log");
    dbglogfile.setVerbosity(3);

    const std::string basedir{getenv("UNDERPASS_SOURCE_TREE_ROOT")
                                  ? getenv("UNDERPASS_SOURCE_TREE_ROOT")
                                  : DATADIR};

    const auto test_data_dir{basedir + "/testsuite/testdata"};

    // Read the changeset state file
    TestStateFile statefile(test_data_dir + "/993.state.txt", false);
    // statefile.dump();
    if (statefile.getSequence() == 4517993 &&
        to_simple_string(statefile.getTimestamp()) == "2021-Apr-28 03:03:09") {
        runtest.pass("StateFile::Statefile(disk)");
    } else {
        runtest.fail("StateFile::Statefile(disk)");
    }

    std::string buf =
        "---\nlast_run: 2020-10-08 22:30:01.737719000 +00:00\nsequence: 4139993\n";
    TestStateFile mem(buf, true);
    // mem.dump();
    if (mem.getSequence() == 4139993 &&
        to_simple_string(statefile.getTimestamp()) == "2021-Apr-28 03:03:09") {
        runtest.pass("StateFile::Statefile(buffer)");
    } else {
        runtest.fail("StateFile::Statefile(buffer)");
    }

    TestCO testco;
    testco.readChanges(test_data_dir + "/123.osc");
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
    if (twf->change_id == 99069879L && twf->id == 474556695L &&
        twf->uid == 1041828L) {
        runtest.pass("ChangeSetFile::readChanges(first change, first way)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(first change, first way)");
    }
    auto tnb = tf->nodes.back();
    // tnb->dump();
    if (tnb->change_id == 94802322L && tnb->id == 289112823L) {
        runtest.pass("ChangeSetFile::readChanges(first change, last node)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(first change, last node)");
    }
    auto twb = tf->ways.back();
    // twb->dump();
    if (twb->change_id == 99063443L && twb->id == 67365141L &&
        twb->uid == 1137406L) {
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

    testco.readChanges(test_data_dir + "/changeset-data.osm");

    if (testco.changes.size() == 3 && testco.changes.back()->obj->id > 0) {
        runtest.pass("ChangeSetFile::readChanges(uncompressed)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(uncompressed)");
    }

    auto rc = testco.readChanges(basedir + "/changeset-data2.osm.gz");
    if (testco.changes.size() == 3 && testco.changes.back()->obj->id > 0) {
        runtest.pass("ChangeSetFile::readChanges(compressed)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(compressed)");
    }

    // Test crash when build with default optimization (release mode)
    multipolygon_t null_island_poly;
    boost::geometry::read_wkt(
        "MULTIPOLYGON(((0 0, 0 0.1, 0.1 0.1, 0.1 0, 0 0)))", null_island_poly);

    testco.areaFilter(null_island_poly);
    if (testco.changes.size() == 0) {
        runtest.pass("ChangeSetFile::areaFilter(null_island_poly)");
    } else {
        runtest.fail("ChangeSetFile::areaFilter(null_island_poly)");
    }

    // Contains a single node from the hospital lat="22.9890996" lon="114.4398219
    multipolygon_t single_node_poly;
    boost::geometry::read_wkt(
        "MULTIPOLYGON(((114.4397 22.988, 114.4397 22.99, 114.4399 22.99, 114.4399 22.988, 114.4397 22.988)))",
        single_node_poly);
    testco.changes.clear();
    testco.nodecache.clear();
    testco.readChanges(test_data_dir + "/123.osc");
    testco.areaFilter(single_node_poly);
    if (testco.changes.size() == 1 &&
        testco.changes.front()->obj->id == 67365141L) {
        runtest.pass("ChangeSetFile::areaFilter(single_node_poly)");
    } else {
        runtest.fail("ChangeSetFile::areaFilter(single_node_poly)");
    }
};
