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
#include <unistd.h>
#include <fcntl.h>

#include "osmstats/osmstats.hh"
#include "data/yaml.hh"
#include "log.hh"

using namespace logger;

using namespace osmstats;

TestState runtest;

int
main(int argc, char *argv[])
{
    Timer timer;

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("val-test.log");
    dbglogfile.setVerbosity(3);

    yaml::Yaml yaml;
    std::string filespec = DATADIR;
    filespec += "/testsuite/libunderpass.all/test.yaml";
    yaml.read(filespec);
    yaml.dump();
    
    if (yaml.tags.size() > 0) {
        runtest.pass("Yaml::read()");
    } else {
        runtest.fail("Yaml::read()");
    }

    if (yaml.containsKey("building:material") && yaml.containsKey("building:roof")) {
        runtest.pass("Yaml::containsKey()");
    } else {
        runtest.fail("Yaml::containsKey()");
    }

    if (yaml.containsValue("building:material", "metal")&& yaml.containsValue("building:roof", "tiles")) {
        runtest.pass("Yaml::containsValue(good)");
    } else {
        runtest.fail("Yaml::containsValue(good)");
    }

    if (yaml.containsKey("building:material") && yaml.containsKey("building:roof")) {
        runtest.pass("Yaml::containsKey()");
    } else {
        runtest.fail("Yaml::containsKey()");
    }

    if (yaml.containsKey("building:levels") && yaml["building:levels"].size() == 0) {
        runtest.pass("Yaml::containsValue(none)");
    } else {
        runtest.fail("Yaml::containsValue(none)");
    }
}
