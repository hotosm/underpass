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

using namespace logger;
using namespace replicatorconfig;
using namespace replicator;
using namespace boost::posix_time;

int
main(int argc, char *argv[]) {

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("rep-test.log");
    dbglogfile.setVerbosity(3);

    ReplicatorConfig config;
    config.planet_server = config.planet_servers[0].domain + "/replication";
    std::string ts("2020-01-01T00:00:00");
    config.start_time = from_iso_extended_string(ts);

    replicator::Replicator replicator;
    auto osmchange = replicator.findRemotePath(config, config.start_time);
    osmchange->dump();

    // if () {
    //     runtest.pass("Find remote path from timestamp");    
    // } else {
    //     runtest.fail("Find remote path from timestamp");    
    // }
    
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
