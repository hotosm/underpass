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
#include <boost/algorithm/string.hpp>
#include <boost/geometry.hpp>
#include "data/geoutil.hh"

TestState runtest;

using namespace logger;

class TestChangeset : public changeset::ChangeSetFile {};

int
main(int argc, char *argv[])
{

    // Changeset with 5 hashtags
    std::string changesetFile(DATADIR);
    changesetFile += "/testsuite/testdata/hashtags-test.osc";
    TestChangeset changeset;
    changeset::ChangeSet *change;
    changeset.readChanges(changesetFile);
    multipolygon_t polyEmpty;
    changeset.areaFilter(polyEmpty);
    change = changeset.changes.front().get();

    // for (auto it = std::begin(change->hashtags); it != std::end(change->hashtags); ++it) {
    //     std::cout << *it << std::endl;
    // }

    if (change->hashtags.size() == 7) {
        runtest.pass("ChangeSet hashtags");
    } else {
        runtest.fail("ChangeSet hashtags");
    }

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
