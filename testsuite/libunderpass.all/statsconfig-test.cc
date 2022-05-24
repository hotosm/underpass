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

    // TODO: 
    // [ ] Get stats from osmchange file using YAML configuration file
    // [ ] Assert results

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("statsconfig-test.log");
    dbglogfile.setVerbosity(3);

    statsconfig::StatsConfigFile statsconfigfile;
    std::string filename = DATADIR;
    filename += "/testsuite/testdata/statsconfig.yaml";    
    std::vector<statsconfig::StatsConfig> statsconfig;
    statsconfigfile.read_yaml(filename, statsconfig);

    // for (int i = 0; i < statsconfig.size(); ++i) {
    //     std::cout << "Stat: " << statsconfig[i].name << std::endl;
    //     std::cout << " area: " << std::endl;
    //     dump_tags(statsconfig[i].area);
    //     std::cout << " poi: " << std::endl;
    //     dump_tags(statsconfig[i].poi);
    //     std::cout << " line: " << std::endl;
    //     dump_tags(statsconfig[i].line);
    // }

    if (
        statsconfig[0].area.count("building") && statsconfig[0].area.at("building")[1] == "school" &&
        statsconfig[1].poi.count("amenity") && statsconfig[1].poi.at("amenity")[0] == "emergency" &&
        statsconfig[2].line.count("highway") && statsconfig[2].line.at("highway")[0] == "yes"
    ) {
        runtest.pass("StatsConfigFile::read_yaml()");
    } else {
        runtest.fail("StatsConfigFile::read_yaml()");
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
