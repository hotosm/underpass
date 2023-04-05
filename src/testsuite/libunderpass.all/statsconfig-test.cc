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
#include "utils/log.hh"

#include "stats/statsconfig.hh"
#include "osm/osmchange.hh"

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
    statsconfig::StatsConfig::setConfigurationFile("../src/testsuite/testdata/stats/statsconfig.yaml");
    auto statsconfig = statsconfig::StatsConfig();

    if (statsconfig.search("building", "school", osmchange::way) == "buildings") {
        runtest.pass("StatsConfigSearch::search()");
    } else {
        runtest.fail("StatsConfigSearch::search()");
    }

    if (statsconfig.search("underpass_tag", "underpass_test", osmchange::way) == "buildings") {
        runtest.pass("StatsConfigSearch::search() - way custom tags");
    } else {
        runtest.fail("StatsConfigSearch::search() - way custom tags");
    }

    if (statsconfig.search("underpass_tag", "underpass_test", osmchange::node) == "buildings") {
        runtest.pass("StatsConfigSearch::search() - node custom tags");
    } else {
        runtest.fail("StatsConfigSearch::search() - node custom tags");
    }

    if (statsconfig.search("underpass_tag2", "underpass_test", osmchange::node) == "underpass_category") {
        runtest.pass("StatsConfigSearch::search() - node custom tags and custom category");
    } else {
        runtest.fail("StatsConfigSearch::search() - node custom tags and custom category");
    }

    if (statsconfig.search("underpass_tag2", "underpass_test", osmchange::way) == "underpass_category") {
        runtest.pass("StatsConfigSearch::search() - way custom tags and custom category");
    } else {
        runtest.fail("StatsConfigSearch::search() - way custom tags and custom category");
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
