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
#include <string>
#include <pqxx/pqxx>
#include <libxml++/libxml++.h>

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "utils/geoutil.hh"
using namespace geoutil;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestGU : public GeoUtil
{
    bool testFile(const std::string &filespec) {
        // std::string basedir="DATADIR";
        return true;
    };
};

int
main(int argc, char* argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("geo-test.log");
    dbglogfile.setVerbosity(3);

    TestGU tgu;

    std::string test_data_dir(DATADIR);

    if (!tgu.readFile("../xxx/priority.geojson")) {
        runtest.pass("Read file with bad relative path");
    } else {
        runtest.fail("Read file with bad relative path");
        return 1;
    }

    if (tgu.readFile("ETCDIR/priority.geojson")) {
        runtest.pass("Read file with absolute path");
    } else {
        runtest.fail("Read file with absolute path");
        return 1;
    }

};

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
