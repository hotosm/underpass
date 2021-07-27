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

#include "hottm.hh"
#include "osmstats/osmstats.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "data/validate.hh"
#include "data/osmobjects.hh"
#include "timer.hh"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace osmstats;

TestState runtest;

class TestVal : public validate::Validate
{
public:
    TestVal(void) {};
};

int
main(int argc, char *argv[])
{

    TestVal tv;
    Timer timer;
    
    if (tv.checkTag("building", "yes") == true) {
        runtest.pass("Validate::checkTag(good tag)");
    } else {
        runtest.fail("Validate::checkTag(good tag)");
    }

    if (tv.checkTag("building", "") == false) {
        runtest.pass("Validate::checkTag(empty value)");
    } else {
        runtest.fail("Validate::checkTag(empty value)");
    }

    if (tv.checkTag("foo bar", "bar") == false) {
        runtest.pass("Validate::checkTag(space)");
    } else {
        runtest.fail("Validate::checkTag(space)");
    }

    osmobjects::OsmNode node;
    node.addTag("traffic-light", "yes");
    if (tv.checkPOI(&node)) {
        runtest.pass("Validate::checkNode(tag)");
    } else {
        runtest.fail("Validate::checkNode(tag)");
    }

    osmobjects::OsmWay way(11111);
    way.addTag("building", "yes");
    if (tv.checkWay(&way)) {
        runtest.pass("Validate::checkWay(empty way)");
    } else {
        runtest.fail("Validate::checkWay(empty way)");
    }

    way.addRef(1234);
    way.addRef(234);
    way.addRef(345);
    way.addRef(456);
    way.addRef(1234);
    timer.startTimer();
    if (tv.checkWay(&way)) {
        runtest.pass("Validate::checkWay(building with tags)");
    } else {
        runtest.fail("Validate::checkWay(building with tags)");
    }
    timer.endTimer();
    way.tags.clear();
    timer.startTimer();
    if (tv.checkWay(&way) == false) {
        runtest.pass("Validate::checkWay(not building)");
    } else {
        runtest.fail("Validate::checkWay(not building)");
    }
    timer.endTimer();
    
    if (tv.checkWay(&way) == false) {
        runtest.pass("Validate::checkWay(no tags)");
    } else {
        runtest.fail("Validate::checkWay(no tags)");
    }

    way.addTag("building", "");
    if (tv.checkWay(&way) == false) {
        runtest.pass("Validate::checkWay(empty value)");
    } else {
        runtest.fail("Validate::checkWay(empty value)");
    }
    way.addTag("foo bar", "yes");
    if (tv.checkWay(&way) == false) {
        runtest.pass("Validate::checkWay(space)");
    } else {
        runtest.fail("Validate::checkWay(space)");
    }
}
