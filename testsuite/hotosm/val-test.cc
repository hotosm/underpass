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

#include "hottm.hh"
#include "osmstats/osmstats.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#include "data/validate.hh"
#include "data/osmobjects.hh"

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace osmstats;

TestState runtest;

class TestVal : public validate::Validate
{
public:
    TestVal(void) { };
};

int
main(int argc, char *argv[])
{

    TestVal tv;
    
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
    if (tv.checkPOI(node)) {
        runtest.pass("Validate::checkNode(tag)");
    } else {
        runtest.fail("Validate::checkNode(tag)");
    }

    osmobjects::OsmWay way;
    way.addTag("building", "yes");
    if (tv.checkWay(way)) {
        runtest.pass("Validate::checkWay(empty way)");
    } else {
        runtest.fail("Validate::checkWay(empty way)");
    }

    way.addRef(1234);
    way.addRef(234);
    way.addRef(345);
    way.addRef(456);
    way.addRef(1234);
    if (tv.checkWay(way)) {
        runtest.pass("Validate::checkWay(building with tags)");
    } else {
        runtest.fail("Validate::checkWay(building with tags)");
    }
    way.tags.clear();
    if (tv.checkWay(way) == false) {
        runtest.pass("Validate::checkWay(not building)");
    } else {
        runtest.fail("Validate::checkWay(n0t building)");
    }
    
    if (tv.checkWay(way) == false) {
        runtest.pass("Validate::checkWay(no tags)");
    } else {
        runtest.fail("Validate::checkWay(no tags)");
    }

    way.addTag("building", "");
    if (tv.checkWay(way) == false) {
        runtest.pass("Validate::checkWay(empty value)");
    } else {
        runtest.fail("Validate::checkWay(empty value)");
    }
    way.addTag("foo bar", "yes");
    if (tv.checkWay(way) == false) {
        runtest.pass("Validate::checkWay(space)");
    } else {
        runtest.fail("Validate::checkWay(space)");
    }
}
