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
#include <iostream>
#include <vector>
#include "log.hh"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "galaxy/osmchange.hh"
#include "replicatorconfig.hh"
#include "galaxy/planetreplicator.hh"
#include "galaxy/changeset.hh"
#include "galaxy/replication.hh"

using namespace logger;
using namespace replicatorconfig;
using namespace planetreplicator;
using namespace replication;
using namespace boost::posix_time;

class TestCO : public osmchange::OsmChangeFile {
};

class TestPlanet : public replication::Planet {
};

TestState runtest;

int
main(int argc, char *argv[]) {

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("planetreplicator-test.log");
    dbglogfile.setVerbosity(3);

    ReplicatorConfig config;
    config.planet_server = config.planet_servers[0].domain + "/replication";

    std::vector<std::string> dates = {
        "-01-01T00:00:00",
        "-01-07T00:00:00",
        "-02-12T00:00:00",
        "-02-17T00:00:00",
        "-03-22T00:00:00",
        "-03-28T00:00:00",
    };

    ptime now = boost::posix_time::microsec_clock::universal_time();
    int next_year = now.date().year() + 1;

    for (int i = 2017; i != next_year; ++i) {
        for (auto it = std::begin(dates); it != std::end(dates); ++it) {

            std::string year_string = std::to_string(i);
            std::string ts(year_string + *it);
            config.start_time = from_iso_extended_string(ts);

            time_duration diffWithNow = now - config.start_time;

            if (diffWithNow.hours() > 0) {

                planetreplicator::PlanetReplicator replicator;
                auto osmchange = replicator.findRemotePath(config, config.start_time);
                TestCO change;
                if (boost::filesystem::exists(osmchange->filespec)) {
                    change.readChanges(osmchange->filespec);
                } else { 
                    TestPlanet planet;
                    auto data = planet.downloadFile(osmchange->getURL());
                    auto xml = planet.processData(osmchange->filespec, *data);
                    std::istream& input(xml);
                    change.readXML(input);
                }

                ptime timestamp = change.changes.back()->final_entry;
                auto timestamp_string = to_simple_string(timestamp);
                auto start_time_string = to_simple_string(config.start_time);

                time_duration delta = timestamp - config.start_time;
                if (delta.hours() > -24 && delta.hours() < 12) {
                    runtest.pass("Find remote path from timestamp +/- 1 day (" + start_time_string + ") (" + timestamp_string + ")");
                } else {
                    runtest.fail("Find remote path from timestamp +/- 1 day (" + start_time_string + ") (" + timestamp_string + ")");
                }
            }
        }
    }
    
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
