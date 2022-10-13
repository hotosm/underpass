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
#include "galaxy/galaxy.hh"
#include "data/osmobjects.hh"
#include "log.hh"

using namespace logger;

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace galaxy;

TestState runtest;

typedef std::shared_ptr<Validate>(plugin_t)();

void test_geom(std::shared_ptr<Validate> &plugin);
void test_plugin(std::shared_ptr<Validate> &plugin);

int
main(int argc, char *argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("val-test.log");
    dbglogfile.setVerbosity(3);

#if 1
    std::string plugins;
    if (boost::filesystem::exists("../../validate/.libs")) {
        plugins = "../../validate/.libs";
    } else if (boost::filesystem::exists("./validate/.libs")) {
        plugins = "./validate/.libs";
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

    // checkTag()

    // Existence of key=value
    auto status = plugin->checkTag("building", "yes");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(good tag)");
    } else {
        runtest.fail("Validate::checkTag(good tag)");
    }

    // Empty value
    status = plugin->checkTag("building", "");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(empty value)");
    } else {
        runtest.fail("Validate::checkTag(empty value)");
    }

    // Invalid tag, not listed into the config file (ex: foo bar=bar)
    status = plugin->checkTag("foo bar", "bar");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(space in key)");
    } else {
        runtest.fail("Validate::checkTag(space in key)");
    }

    // Node - checkPOI()

    osmobjects::OsmNode node;
    node.id = 11111;
    node.change_id = 22222;

    // Node with no tags
    status = plugin->checkPOI(node, "building");
    if (status->osm_id == 11111 && status->hasStatus(notags)) {
        runtest.pass("Validate::checkPOI(no tags)");
    } else {
        runtest.fail("Validate::checkPOI(no tags)");
    }

    // Has valid tags, but it's incomplete
    node.addTag("building", "yes");
    status = plugin->checkPOI(node, "building");
    if (status->osm_id == 11111 && !status->hasStatus(badvalue) && status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkPOI(incomplete but correct tagging)");
    } else {
        runtest.fail("Validate::checkPOI(incomplete but correct tagging)");
    }

    // Has an invalid key=value
    node.addTag("building:material", "sponge");
    status = plugin->checkPOI(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkPOI(bad value)");
    } else {
        runtest.fail("Validate::checkPOI(bad value)");
    }

    node.addTag("building:material", "wood");
    node.addTag("building:levels", "3");
    node.addTag("building:roof", "tile");

    // Has all valid tags
    status = plugin->checkPOI(node, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkPOI(no bad values)");
    } else {
        runtest.fail("Validate::checkPOI(no bad values)");
    }

    // Way - checkWay()

    osmobjects::OsmWay way;
    way.id = 333333;
    way.addRef(1234);
    way.addRef(234);
    way.addRef(345);
    way.addRef(456);
    way.addRef(1234);
    status = plugin->checkWay(way, "building");

    // Way with no tags
    if (status->hasStatus(notags)) {
        runtest.pass("Validate::checkWay(no tags)");
    } else {
        runtest.fail("Validate::checkWay(no tags)");
        way.dump();
    }

    // Existence of key=value
    way.addTag("building", "yes");
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badvalue) && status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkWay(incomplete but correct tagging)");
    } else {
        runtest.fail("Validate::checkWay(incomplete but correct tagging)");
    }

    // Has an invalid key=value
    way.addTag("building:material", "sponge");
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(bad value)");
    } else {
        runtest.fail("Validate::checkWay(bad value)");
    }

    way.addTag("building:material", "wood");
    way.addTag("building:levels", "3");
    way.addTag("building:roof", "tile");

    // Has all valid tags
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(no bad values)");
    } else {
        runtest.fail("Validate::checkWay(no bad values)");
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

    // Create fake buildings
    auto way1 = std::make_shared<osmobjects::OsmWay>(1101898);
    std::string p1 = "POLYGON((29.3858834 -1.9609767,29.3858954 -1.9610071,29.3859234 -1.9609961,29.3859114 -1.9609657,29.3858834 -1.9609767))";
    boost::geometry::read_wkt(p1, way1->polygon);
    way1->addTag("layer", "1");
    way1->addTag("building", "yes");

    std::string p2 = "POLYGON((29.3858553 -1.9609767,29.3858673 -1.9610071,29.3858953 -1.9609961,29.3858833 -1.9609657,29.3858553 -1.9609767))";
    auto way2 = std::make_shared<osmobjects::OsmWay>(1101898);
    boost::geometry::read_wkt(p2, way2->polygon);
    way2->addTag("layer", "2");
    way2->addTag("building", "yes");

    std::string p3 = "POLYGON((29.385826 -1.9609716,29.385838 -1.961002,29.385866 -1.960991,29.385854 -1.9609606,29.385826 -1.9609716))";
    auto way3 = std::make_shared<osmobjects::OsmWay>(1101899);
    boost::geometry::read_wkt(p3, way3->polygon);
    way3->addTag("layer", "3");
    way3->addTag("building", "yes");

    std::list<std::shared_ptr<osmobjects::OsmWay>> ways;
    ways.push_back(way1);
    ways.push_back(way2);
    ways.push_back(way3);

    if (plugin->overlaps(ways, *way1)) {
        runtest.fail("Validate::overlaps()");
    } else {
        runtest.pass("Validate::overlaps()");
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
