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

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/dll/import.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "validate/hotosm.hh"
#include "validate/validate.hh"
#include "osmstats/osmstats.hh"
#include "data/osmobjects.hh"
#include "log.hh"

using namespace logger;

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace osmstats;

TestState runtest;

typedef std::shared_ptr<Validate>(plugin_t)();

void test_geom(std::shared_ptr<Validate> &plugin);
void test_plugin(std::shared_ptr<Validate> &plugin);

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
    auto status = plugin->checkTag("building", "yes");
    if (status->hasStatus(correct)) {
        runtest.pass("Validate::checkTag(good tag)");
    } else {
        runtest.fail("Validate::checkTag(good tag)");
    }
    status = plugin->checkTag("building", "");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(empty value)");
    } else {
        runtest.fail("Validate::checkTag(empty value)");
    }

    status = plugin->checkTag("foo bar", "bar");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(space in key)");
    } else {
        runtest.fail("Validate::checkTag(space in key)");
    }

    osmobjects::OsmNode node;
    node.id = 11111;
    node.change_id = 22222;
    status = plugin->checkPOI(node, "building");
    if (status->osm_id == 11111 && status->hasStatus(notags)) {
        runtest.pass("Validate::checkPOI(no tags)");
    } else {
        runtest.fail("Validate::checkPOI(no tags)");
    }

    node.addTag("building", "yes");
    status = plugin->checkPOI(node, "building");
    // status->dump();
    if (status->osm_id == 11111 && status->hasStatus(correct)) {
        runtest.pass("Validate::checkPOI(incomplete but correct tagging)");
    } else {
        runtest.fail("Validate::checkPOI(incomplete but correct tagging)");
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

    osmobjects::OsmWay way;
    way.id = 333333;
    way.addRef(1234);
    way.addRef(234);
    way.addRef(345);
    way.addRef(456);
    way.addRef(1234);
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(notags)) {
        runtest.pass("Validate::checkWay(no tags)");
    } else {
        runtest.fail("Validate::checkWay(no tags)");
        way.dump();
    }

    way.addTag("building", "yes");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(correct)) {
        runtest.pass("Validate::checkWay(incomplete but correct tagging)");
    } else {
        runtest.fail("Validate::checkWay(incomplete but incorrect tagging)");
    }

    way.addTag("building:material", "sponge");
    status = plugin->checkWay(way, "building");
    // status->dump();
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(bad value)");
    } else {
        runtest.fail("Validate::checkWay(bad value)");
    }

    way.addTag("building:material", "wood");
    way.addTag("building:levels", "3");
    way.addTag("building:roof", "tiles");

    status = plugin->checkWay(way, "building");
    // status->dump();
    if (status->hasStatus(complete)) {
        runtest.pass("Validate::checkWay(complete)");
    } else {
        runtest.fail("Validate::checkWay(complete)");
    }

    test_geom(plugin);
}

void
test_geom(std::shared_ptr<Validate> &plugin)
{
    osmchange::OsmChangeFile ocf;
    std::string filespec = SRCDIR;
    filespec += "/rect.osc";
    ocf.readChanges(filespec);
//    ocf.dump();

    for (auto it = std::begin(ocf.changes); it != std::end(ocf.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        // change->dump();
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            auto status = plugin->checkWay(*way, "building");
            // status->dump();
            // std::cerr << way->tags["note"] << std::endl;
            if (way->id == -101790) {
                if (status->hasStatus(incomplete) && !status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(incomplete rectangle)");
                } else {
                    runtest.pass("Validate::checkWay(incomplete rectangle)");
                }
            }
            if (way->id == 838311812) {
                if (status->hasStatus(incomplete) && !status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(incomplete complex rectangle)");
                } else {
                    runtest.pass("Validate::checkWay(incomplete complex rectangle)");
                }
            }
            if (way->id == 824015796) {
                if (status->hasStatus(incomplete) && status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(incomplete triangle)");
                } else {
                    runtest.pass("Validate::checkWay(incomplete triangle)");
                }
            }
            if (way->id == 821663069) {
                if (status->hasStatus(incomplete) && status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(incomplete bad geometry)");
                } else {
                    runtest.pass("Validate::checkWay(incomplete bad geometry)");
                }
            }
            if (way->id == -101806) {
                if (status->hasStatus(incomplete) && status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom rectangle)");
                } else {
                    runtest.pass("Validate::checkWay(badgeom rectangle)");
                }
            }
            if (way->id == 856945340) {
                if (status->hasStatus(incomplete) && !status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom big round)");
                } else {
                    runtest.pass("Validate::checkWay(badgeom big round)");
                }
            }
            if (way->id == 961600809) {
                if (status->hasStatus(incomplete) && status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom small round)");
                } else {
                    runtest.pass("Validate::checkWay(badgeom small round)");
                }
            }
        }
    }
    
#if 0
    auto change = ocf.changes.front();
    auto way = change->ways.front();
    plugin->checkWay(*way, "building");
    plugin->cornerAngle(way->linestring);
    ocf.changes.pop_front();

    way = change->ways.front();
    plugin->checkWay(*way, "building");
    plugin->cornerAngle(way->linestring);
    ocf.changes.pop_front();
#endif
}
