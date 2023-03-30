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

#include <cmath>
#include <dejagnu.h>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "utils/geoutil.hh"
#include "utils/log.hh"
#include "osm/changeset.hh"
#include "osm/osmchange.hh"
#include "stats/querystats.hh"
#include "replicator/replication.hh"

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time.hpp>
#include <boost/geometry.hpp>
#include <boost/program_options.hpp>

namespace opts = boost::program_options;

using namespace logger;
using namespace changeset;
using namespace boost::posix_time;
using namespace boost::gregorian;

#define VERIFY(condition, message)                                 \
    if (condition) {                                               \
        runtest.pass(message);                                     \
    } else {                                                       \
        runtest.fail(message);                                     \
        std::cerr << "Failing at: " << __FILE__ << ':' << __LINE__ \
                  << std::endl;                                    \
        exit(EXIT_FAILURE);                                        \
    }

#define COMPARE(first, second, message)                                    \
    if (first == second) {                                                 \
        runtest.pass(message);                                             \
    } else {                                                               \
        runtest.fail(message);                                             \
        std::cerr << "Failing at: " << __FILE__ << ':' << __LINE__         \
                  << std::endl;                                            \
        std::cerr << "Values are not equal: " << first << " != " << second \
                  << std::endl;                                            \
        exit(EXIT_FAILURE);                                                \
    }

TestState runtest;

class TestCS : public changeset::ChangeSetFile {
};

class TestCO : public osmchange::OsmChangeFile {
};

class TestStateFile : public replication::StateFile {
  public:
    TestStateFile(const std::string &file, bool memory)
        : replication::StateFile(file, memory){};
    // Accessors for the private data
    ptime getTimestamp(void) { return timestamp; };
    long getSequence(void) { return sequence; };
};

int
main(int argc, char *argv[])
{
    opts::positional_options_description p;
    opts::variables_map vm;
    try {
        opts::options_description desc("Allowed options");
        // clang-format off
        desc.add_options()
            ("help,h", "display help")
            ("osmchange,o", opts::value<std::string>(), "Import osmchange (*.osc) file")
            ("changefile,c", opts::value<std::string>(), "Import change (*.osm) file");

        opts::store(opts::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        opts::notify(vm);
        if (vm.count("help")) {
            std::cout << "Usage: options_description [options]" << std::endl;
            std::cout << desc << std::endl;
            exit(0);
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    if (vm.count("changefile")) {
        std::string file = vm["changefile"].as<std::string>();
        std::cout << "Importing change file " << file << std::endl;
        auto changeset = std::make_shared<changeset::ChangeSetFile>();
        changeset->readChanges(file);
        changeset->dump();
        exit(0);
    }

    if (vm.count("osmchange")) {
        std::string file = vm["osmchange"].as<std::string>();
        std::cout << "Importing osmchange file " << file << std::endl;
        auto changes = std::make_shared<osmchange::OsmChangeFile>();
        changes->readChanges(file);
        changes->dump();
        exit(0);
    }

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("change-test.log");
    dbglogfile.setVerbosity(3);

    const std::string basedir{getenv("UNDERPASS_SOURCE_TREE_ROOT")
                                  ? getenv("UNDERPASS_SOURCE_TREE_ROOT")
                                  : SRCDIR};

    std::string test_data_dir(DATADIR);
    test_data_dir += "/testsuite/testdata/";

    std::cerr << test_data_dir << std::endl;

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

    // Tracking all changeset from file
    std::vector<long> changeset_ids = {8580445, 94802322, 98934881, 98935384, 98935857, 99055952, 99056962, 99062100, 99063443, 99069702, 99069879};
    std::map<long, long> changeset_ids_found;
    for (const auto &change: testco.changes) {
        for (const auto &node: change->nodes) {
            changeset_ids_found.insert(std::pair<long, long>(node->change_id, node->change_id));
        }
        for (const auto &way: change->ways) {
            changeset_ids_found.insert(std::pair<long, long>(way->change_id, way->change_id));
        }
        for (const auto &relation: change->relations) {
            changeset_ids_found.insert(std::pair<long, long>(relation->change_id, relation->change_id));
        }
    }
    bool all_changesets_tracked = true;
    for (auto it = std::begin(changeset_ids); it != std::end(changeset_ids); ++it) {
        if (!changeset_ids_found.count(*it)) {
            all_changesets_tracked = false;
            break;
        }
    }
    if (all_changesets_tracked) {
        runtest.pass("ChangeSetFile::readChanges(tracking all changesets from file)");
    } else {
        runtest.fail("ChangeSetFile::readChanges(tracking all changesets from file)");
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

    std::list<std::shared_ptr<osmobjects::OsmNode>> priority_nodes;
    for (const auto &change: testco.changes) {
        for (const auto &node: change->nodes) {
            if (node->priority) {
                priority_nodes.push_back(node);
            }
        }
    }

    COMPARE(priority_nodes.size(), 0, "ChangeSetFile::areaFilter(null_island_poly)");

    // Contains a single node from the hospital lat="22.9890996" lon="114.4398219
    multipolygon_t single_node_poly;
    boost::geometry::read_wkt(
        "MULTIPOLYGON(((114.43 22.98, 114.43 22.99, 114.44 22.99, 114.44 22.98, 114.43 22.98)))",
        single_node_poly);
    testco.changes.clear();
    testco.nodecache.clear();
    testco.readChanges(test_data_dir + "/123.osc");
    testco.areaFilter(single_node_poly);

    priority_nodes.clear();
    for (const auto &change: testco.changes) {
        for (const auto &node: change->nodes) {
            if (node->priority) {
                priority_nodes.push_back(node);
            }
        }
    }
    COMPARE(priority_nodes.size(), 1,
            "ChangeSetFile::areaFilter(single_node_poly) - size");
    COMPARE(priority_nodes.front()->id, 5776216755L,
            "ChangeSetFile::areaFilter(single_node_poly) - id");

    // Test relations
    testco.changes.clear();
    testco.nodecache.clear();

    const auto xml{R"xml(<?xml version='1.0' encoding='UTF-8'?>
      <osmChange version="0.6" generator="Osmosis 0.47.4">
        <create>
          <node id="1" version="2" timestamp="2021-02-11T01:49:51Z" uid="1" user="bored_developer" changeset="99069702" lat="22.1" lon="114.1">
           <tag k="name" v="node_1"/>
          </node>
          <node id="2" version="2" timestamp="2021-02-11T01:49:51Z" uid="1" user="bored_developer" changeset="99069702" lat="22.2" lon="114.2">
          <tag k="name" v="node_1"/>
          </node>
          <way id="3" version="2" timestamp="2021-02-11T01:56:35Z" uid="1" user="bored_developer" changeset="99069879">
            <nd ref="1"/>
            <nd ref="2"/>
            <tag k="name" v="node_1_rel_node_2"/>
          </way>
          <relation id="4" user="some_bored_user" uid="2" version="1" changeset="1" timestamp="2012-07-10T00:00:00Z">
             <member type="way" ref="3" role="a_role"/>
             <tag k="type" v="route"/>
          </relation>
        </create>
      </osmChange>
    )xml"};

    std::stringstream xml_stream{xml};
    testco.readXML(xml_stream);
    VERIFY(testco.changes.size() == 1 &&
               testco.changes.front()->nodes.size() == 2 &&
               testco.changes.front()->relations.size() == 1 &&
               testco.changes.front()->ways.size() == 1,
           "ChangeSetFile::readXML(xml) - relations");

    const auto relation{testco.changes.front()->relations.front()};
    COMPARE(relation->action, osmobjects::create,
            "ChangeSetFile::readXML(xml) - relation.action");
    COMPARE(relation->type, osmobjects::relation,
            "ChangeSetFile::readXML(xml) - relation.type");
    COMPARE(relation->members.size(), 1,
            "ChangeSetFile::readXML(xml) - relation.members");

    // Check relation member
    const auto member{relation->members.front()};
    COMPARE(member.ref, 3, "ChangeSetFile::readXML(xml) - relation member ref");
    COMPARE(member.role, "a_role",
            "ChangeSetFile::readXML(xml) - relation member role");
    COMPARE(member.type, osmobjects::osmtype_t::way,
            "ChangeSetFile::readXML(xml) - relation member type");
};

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
