//
// Copyright (c) 2020, Humanitarian OpenStreetMap Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <dejagnu.h>
#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include <libxml++/libxml++.h>

#include "hottm.hh"
#include "osmstats/osmstats.hh"
#include "osmstats/changeset.hh"
#include "osmstats/changefile.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "osmstats/geoutil.hh"
using namespace geoutil;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestGU : public GeoUtil
{
    bool testFile(const std::string &filespec) {
        // std::string basedir="DATADIR";
    };
};

int
main(int argc, char* argv[])
{
    TestGU tgu;
    std::string basedir = DATADIR;

    // Read a single polygon
    // tgu.readFile(basedir + "/include.osm", false);
    // Read a multipolygon
    tgu.startTimer();
    tgu.readFile(basedir + "/data/geoboundaries.osm", true);
    tgu.endTimer();

    // See if it worked, which it needs to if any other tests will work
    tgu.dump();

    // Changesets have a bounding box, so we want to find the
    // country the changes were made in.
    double min_lat = -2.8042325;
    double min_lon = 29.5842812;
    double max_lat = -2.7699398;
    double max_lon = 29.6012844;

    tgu.startTimer();
    GeoCountry &country = tgu.inCountry(max_lat, max_lon, min_lat, min_lon);
    tgu.endTimer();
    if (country.getName() == "Rwanda" || country.getAbbreviation(2) == "RW") {
        runtest.pass("GeoUtil::inCountry()");
    } else {
        runtest.fail("GeoUtil::inCountry()");
    }
    country.dump();
};

