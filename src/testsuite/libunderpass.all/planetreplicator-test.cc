//
// Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
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
#include <vector>
#include "utils/log.hh"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/program_options.hpp>
#include "boost/format.hpp"
#include "osm/osmchange.hh"
#include "underpassconfig.hh"
#include "replicator/planetreplicator.hh"
#include "osm/changeset.hh"
#include "replicator/replication.hh"
#include <string>
#include "utils/yaml.hh"
#include <iterator>

using namespace logger;
using namespace underpassconfig;
using namespace planetreplicator;
using namespace replication;
using namespace boost::posix_time;

namespace opts = boost::program_options;

class TestCO : public osmchange::OsmChangeFile {
};

class TestPlanet : public replication::Planet {
};

TestState runtest;

void testPath(underpassconfig::UnderpassConfig config) {
    planetreplicator::PlanetReplicator replicator;
    auto osmchange = replicator.findRemotePath(config, config.start_time);
    TestCO change;
    if (boost::filesystem::exists(osmchange->filespec)) {
        change.readChanges(osmchange->filespec);
    } else {
        TestPlanet planet;
        auto data = planet.downloadFile(osmchange->getURL()).data;
        auto xml = planet.processData(osmchange->filespec, *data);
        std::istream& input(xml);
        change.readXML(input);
    }

    ptime timestamp = change.changes.back()->final_entry;
    auto timestamp_string_debug = to_simple_string(timestamp);
    auto start_time_string_debug = to_simple_string(config.start_time);

    time_duration delta = timestamp - config.start_time;
    if (delta.hours() > -24 && delta.hours() < 24) {
        runtest.pass("Find remote path from timestamp +/- 24 hours (" + start_time_string_debug + ") (" + timestamp_string_debug + ")");
    } else {
        runtest.fail("Find remote path from timestamp +/- 24 hours (" + start_time_string_debug + ") (" + timestamp_string_debug + ")");
    }
}

int
main(int argc, char *argv[]) {

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("planetreplicator-test.log");
    dbglogfile.setVerbosity(3);

    UnderpassConfig config;
    config.planet_server = config.planet_servers[0].domain + "/replication";

    // Declare the supported options.
    opts::positional_options_description p;
    opts::variables_map vm;
    opts::options_description desc("Allowed options");
    desc.add_options()
        ("timestamp,t", opts::value<std::vector<std::string>>(), "Starting timestamp")
    ;
    opts::store(opts::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    opts::notify(vm);

    if (vm.count("timestamp")) {
        auto timestamps = vm["timestamp"].as<std::vector<std::string>>();
        config.start_time = time_from_string(timestamps[0]);
        testPath(config);
    } else {
        std::vector<std::string> dates = {
            "-01-01 00:00:00",
            "-03-07 00:00:00",
            "-06-12 00:00:00",
            "-08-17 00:00:00",
            "-10-22 00:00:00",
            "-12-28 00:00:00",
        };

        ptime now = boost::posix_time::microsec_clock::universal_time();
        int next_year = now.date().year();

        for (int i = 2013; i != next_year + 1; ++i) {
            for (auto it = std::begin(dates); it != std::end(dates); ++it) {

                std::string year_string = std::to_string(i);
                std::string ts(year_string + *it);
                config.start_time = time_from_string(ts);

                time_duration diffWithNow = now - config.start_time;

                if (diffWithNow.hours() > 0) {
                    testPath(config);
                }
            }
        }
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
