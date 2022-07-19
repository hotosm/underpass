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

#include <iostream>
#include <dejagnu.h>
#include "galaxy/changeset.hh"
#include <boost/geometry.hpp>
#include "data/geoutil.hh"

TestState runtest;

using namespace logger;

class TestChangeset : public changeset::ChangeSetFile {
};

int
main(int argc, char *argv[])
{
    // Small changeset in Bangladesh
    std::string changesetFile(DATADIR);
    changesetFile += "/testsuite/testdata/areafilter-test.osc";
    
    TestChangeset changeset;
    multipolygon_t poly;
    changeset::ChangeSet *change;

    // Whole world
    changeset.readChanges(changesetFile);
    boost::geometry::read_wkt("MULTIPOLYGON(((-180 90,180 90, 180 -90, -180 -90,-180 90)))", poly);
    changeset.areaFilter(poly);
    change = changeset.changes.front().get();
    if (change && change->priority) {
        runtest.pass("ChangeSet areaFilter - true");
    } else {
        runtest.fail("ChangeSet areaFilter - true");
    }

    // Small area in north Africa
    boost::geometry::read_wkt("MULTIPOLYGON (((20 35, 45 20, 30 5, 10 10, 10 30, 20 35)))", poly);
    changeset.readChanges(changesetFile);
    changeset.areaFilter(poly);
    change = changeset.changes.front().get();
    if (change && change->priority) {
        runtest.fail("ChangeSet areaFilter - false");
    } else {
        runtest.pass("ChangeSet areaFilter - false");
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
