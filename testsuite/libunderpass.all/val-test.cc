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

#include "validate/hotosm.hh"
#include "validate/validate.hh"
#include "osmstats/osmstats.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/dll/import.hpp>

#include "validate/validate.hh"
#include "data/osmobjects.hh"
#include "timer.hh"
#include "log.hh"

using namespace logger;

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace osmstats;

TestState runtest;

typedef std::shared_ptr<Validate>(plugin_t)();

int
main(int argc, char *argv[])
{
    Timer timer;

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("val-test.log");
    dbglogfile.setVerbosity(3);

#if 1
    std::string plugins;
    if (boost::filesystem::exists("../../validate/.libs")) {
	plugins = "../../validate/.libs";
    }
    boost::dll::fs::path lib_path(plugins);
    boost::function<plugin_t> creator;
    try {
	creator = boost::dll::import_alias<plugin_t>(
	    lib_path / "libhotosm", "create_plugin",
	    boost::dll::load_mode::append_decorations
	    );
	log_debug(_("Loaded plugin hotosm!"));
    } catch (std::exception& e) {
	log_debug(_("Couldn't load plugin! %1%"), e.what());
	exit(0);
    }
    auto plugin = creator();
#else
    auto plugin = std::make_shared<hotosm::Hotosm>();
#endif
    // plugin->dump();

    osmobjects::OsmNode node;
    node.id = 11111;
    node.change_id = 22222;
    auto status = plugin->checkPOI(node, "building");
    if (status->osm_id == 11111 && status->hasStatus(notags)) {
        runtest.pass("Validate::checkPOI(no tags)");
    } else {
        runtest.fail("Validate::checkPOI(no tags)");
    }

    node.addTag("building", "yes");
    status = plugin->checkPOI(node, "building");
    // status->dump();
    if (status->osm_id == 11111 && status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkPOI(incomplete tagging)");
    } else {
        runtest.fail("Validate::checkPOI(incomplete tagging)");
    }

    node.addTag("building:material", "sponge");
    status = plugin->checkPOI(node, "building");
    // status->dump();
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkPOI(bad value)");
    } else {
        runtest.fail("Validate::checkPOI(bad value)");
    }

    node.addTag("building:material", "wood");
    node.addTag("building:levels", "3");
    node.addTag("building:roof", "tiles");

    status = plugin->checkPOI(node, "building");
    // status->dump();
    if (status->hasStatus(complete)) {
        runtest.pass("Validate::checkPOI(complete)");
    } else {
        runtest.fail("Validate::checkPOI(complete)");
    }
    
#if 0
    if (plugin->checkPOI(node, "building") == true) {
        runtest.pass("Validate::checkPOI(tag)");
    } else {
        runtest.fail("Validate::checkPOI(tag)");
    }
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
#endif
}
