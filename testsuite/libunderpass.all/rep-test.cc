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
#include "log.hh"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "galaxy/osmchange.hh"
#include "replicatorconfig.hh"
#include "galaxy/replicator.hh"
#include "galaxy/changeset.hh"

using namespace logger;
using namespace replicatorconfig;
using namespace replicator;
using namespace boost::posix_time;

class TestCO : public osmchange::OsmChangeFile {
};

TestState runtest;

int
main(int argc, char *argv[]) {

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("rep-test.log");
    dbglogfile.setVerbosity(3);

    ReplicatorConfig config;
    config.planet_server = config.planet_servers[0].domain + "/replication";

    // TODO: generate dates for testing 
    std::string ts("2020-03-30T00:00:00");
    config.start_time = from_iso_extended_string(ts);

    replicator::Replicator replicator;
    auto osmchange = replicator.findRemotePath(config, config.start_time);
    TestCO change;
    if (boost::filesystem::exists(osmchange->filespec)) {
        change.readChanges(osmchange->filespec);
    } else { 
        // TODO: download file
        std::cout << "File not found: " << osmchange->filespec << std::endl;
    }

    ptime timestamp = change.changes.back()->final_entry;
    auto timestamp_string = to_simple_string(timestamp);
    auto start_time_string = to_simple_string(config.start_time);

    if (timestamp.date().year() == 2020 && timestamp.date().month() == 3 && timestamp.date().day() >= 29 && timestamp.date().day() <= 31) {
        runtest.pass("Find remote path from timestamp +/- 1 day (" + start_time_string + ") (" + timestamp_string + ")");
    } else {
        runtest.fail("Find remote path from timestamp +/- 1 day (" + start_time_string + ") (" + timestamp_string + ")");
    }
    
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
