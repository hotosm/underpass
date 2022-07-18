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
#include "log.hh"

#include "data/statsconfig.hh"
#include "galaxy/osmchange.hh"

using namespace logger;
TestState runtest;

void dump_tags(std::map<std::string, std::vector<std::string>> tags) {
    for (auto tag_it = std::begin(tags); tag_it != std::end(tags); ++tag_it) {
        std::cout << "  " << tag_it->first << std::endl;
        for (auto value_it = std::begin(tag_it->second); value_it != std::end(tag_it->second); ++value_it) {
            std::cout  << "   " << *value_it << std::endl;
        }
    }
}

int
main(int argc, char *argv[])
{

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("statsconfig-test.log");
    dbglogfile.setVerbosity(3);

    statsconfig::StatsConfigFile statsconfigfile;
    std::string filename = DATADIR;
    filename += "/testsuite/testdata/statsconfig.yaml";
    std::shared_ptr<std::vector<statsconfig::StatsConfig>> statsconfig = statsconfigfile.read_yaml(filename);

    if (
        statsconfig->at(0).way.count("building") && statsconfig->at(0).way.at("building")[1] == "school" &&
        statsconfig->at(1).node.count("amenity") && statsconfig->at(1).node.at("amenity")[0] == "emergency" &&
        statsconfig->at(2).way.count("highway") && statsconfig->at(2).way.at("highway")[0] == "yes"
    ) {
        runtest.pass("StatsConfigFile::read_yaml()");
    } else {
        runtest.fail("StatsConfigFile::read_yaml()");
    }

    statsconfig::StatsConfigSearch search;
    if (search.tag_value("building", "school", osmchange::way, statsconfig) == "buildings") {
        runtest.pass("StatsConfigSearch::tag_value()");
    } else {
        runtest.fail("StatsConfigSearch::tag_value()");
    }

    if (search.tag_value("underpass_tag", "underpass_test", osmchange::way, statsconfig) == "buildings") {
        runtest.pass("StatsConfigSearch::tag_value() - way custom tags");
    } else {
        runtest.fail("StatsConfigSearch::tag_value() - way custom tags");
    }

    if (search.tag_value("underpass_tag", "underpass_test", osmchange::node, statsconfig) == "buildings") {
        runtest.pass("StatsConfigSearch::tag_value() - node custom tags");
    } else {
        runtest.fail("StatsConfigSearch::tag_value() - node custom tags");
    }

    if (search.tag_value("underpass_tag2", "underpass_test", osmchange::node, statsconfig) == "underpass_category") {
        runtest.pass("StatsConfigSearch::tag_value() - node custom tags and custom category");
    } else {
        runtest.fail("StatsConfigSearch::tag_value() - node custom tags and custom category");
    }

    if (search.tag_value("underpass_tag2", "underpass_test", osmchange::way, statsconfig) == "underpass_category") {
        runtest.pass("StatsConfigSearch::tag_value() - way custom tags and custom category");
    } else {
        runtest.fail("StatsConfigSearch::tag_value() - way custom tags and custom category");
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
