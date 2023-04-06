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
#include "stats/querystats.hh"
#include "osm/osmobjects.hh"
#include "utils/log.hh"

using namespace logger;

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace querystats;

TestState runtest;
class TestOsmChange : public osmchange::OsmChangeFile {};

typedef std::shared_ptr<Validate>(plugin_t)();

void test_semantic_building(std::shared_ptr<Validate> &plugin);
void test_semantic_highway(std::shared_ptr<Validate> &plugin);
void test_geometry_building(std::shared_ptr<Validate> &plugin);

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
        log_debug("Loaded plugin hotosm!");
    } catch (std::exception& e) {
        log_debug("Couldn't load plugin! %1%", e.what());
        exit(0);
    }
    auto plugin = creator();
#else
    auto plugin = std::make_shared<hotosm::Hotosm>();
#endif

    test_semantic_building(plugin);
    test_semantic_highway(plugin);
    test_geometry_building(plugin);
}

osmobjects::OsmWay readOsmWayFromFile(std::string filename) {
    TestOsmChange osmchange;
    std::string filespec = DATADIR;
    filespec = DATADIR;
    filespec += filename;
    if (boost::filesystem::exists(filespec)) {
        osmchange.readChanges(filespec);
    } else {
        log_debug("Couldn't load ! %1%", filespec);
    };
    return *osmchange.changes.front().get()->ways.front().get();
}

void
test_semantic_highway(std::shared_ptr<Validate> &plugin) {
     // Way - checkWay()

    auto way = readOsmWayFromFile("/src/testsuite/testdata/validation/highway.osc");

    way.addTag("highway", "primary");

    // Has an invalid key=value
    way.addTag("surface", "sponge");
    auto status = plugin->checkWay(way, "highway");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(bad value) [semantic highway]");
    } else {
        runtest.fail("Validate::checkWay(bad value) [semantic highway]");
    }

    way.addTag("surface", "unpaved");
    way.addTag("smoothness", "very_horrible");
    way.addTag("highway", "unclassified");

    // Has all valid tags
    status = plugin->checkWay(way, "highway");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(no bad values) [semantic highway]");
    } else {
        runtest.fail("Validate::checkWay(no bad values) [semantic highway]");
    }
}

void
test_semantic_building(std::shared_ptr<Validate> &plugin) {
    // checkTag()

    // Existence of key=value
    auto status = plugin->checkTag("building", "yes");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(good tag) [semantic building]");
    } else {
        runtest.fail("Validate::checkTag(good tag) [semantic building]");
    }

    // Empty value
    status = plugin->checkTag("building", "");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(empty value) [semantic building]");
    } else {
        runtest.fail("Validate::checkTag(empty value) [semantic building]");
    }

    // Invalid tag, not listed into the config file (ex: foo bar=bar)
    status = plugin->checkTag("foo bar", "bar");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkTag(space in key) [semantic building]");
    } else {
        runtest.fail("Validate::checkTag(space in key) [semantic building]");
    }

    // Node - checkPOI()

    osmobjects::OsmNode node;
    node.id = 11111;
    node.change_id = 22222;

    // Node with no tags
    status = plugin->checkPOI(node, "building");
    if (status->osm_id == 11111 && status->hasStatus(notags)) {
        runtest.pass("Validate::checkPOI(no tags) [semantic building]");
    } else {
        runtest.fail("Validate::checkPOI(no tags) [semantic building]");
    }

    osmobjects::OsmNode node_place;
    node_place.id = 11111;
    node_place.change_id = 22222;

    // Has valid tags, but it's incomplete
    node_place.addTag("place", "city");
    status = plugin->checkPOI(node_place, "place");
    if (!status->hasStatus(badvalue) && status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkPOI(incomplete but correct tagging) [semantic place]");
    } else {
        runtest.fail("Validate::checkPOI(incomplete but correct tagging) [semantic place]");
    }

    node_place.addTag("name", "Electric City");
    status = plugin->checkPOI(node_place, "place");
    if (!status->hasStatus(badvalue) && !status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkPOI(complete and correct tagging) [semantic place]");
    } else {
        runtest.fail("Validate::checkPOI(complete and correct tagging) [semantic place]");
    }

    node.addTag("building", "yes");

    // Has an invalid key=value ...
    node.addTag("building:material", "sponge");
    status = plugin->checkPOI(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkPOI(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkPOI(bad value) [semantic building]");
    }

    // But it's complete
    status = plugin->checkPOI(node, "building");
    if (status->hasStatus(complete)) {
        runtest.pass("Validate::checkPOI(complete) [semantic building]");
    } else {
        runtest.fail("Validate::checkPOI(complete) [semantic building]");
    }

    node.addTag("building:material", "wood");
    node.addTag("building:levels", "3");
    node.addTag("roof:material", "roof_tiles");

    // Has all valid tags
    status = plugin->checkPOI(node, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkPOI(no bad values) [semantic building]");
    } else {
        runtest.fail("Validate::checkPOI(no bad values) [semantic building]");
    }

    // Way - checkWay()

    auto way = readOsmWayFromFile("/src/testsuite/testdata/validation/building.osc");

    status = plugin->checkWay(way, "building");
    // Way with no tags
    if (status->hasStatus(notags)) {
        runtest.pass("Validate::checkWay(no tags) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(no tags) [semantic building]");
        way.dump();
    }

    way.addTag("building", "yes");

    // // Has an invalid key=value ...
    way.addTag("building:material", "sponge");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(bad value) [semantic building]");
    }

    // But it's complete
    way.addTag("building:material", "sponge");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(complete)) {
        runtest.pass("Validate::checkWay(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(bad value) [semantic building]");
    }

    way.addTag("building:material", "wood");
    way.addTag("building:levels", "3");
    way.addTag("roof:material", "roof_tiles");

    // Has all valid tags
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(no bad values) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(no bad values) [semantic building]");
    }
}

// Geometry tests for buildings
void
test_geometry_building(std::shared_ptr<Validate> &plugin)
{

    // Bad geometry

    osmchange::OsmChangeFile ocf;
    std::string filespec = DATADIR;
    filespec += "/src/testsuite/testdata/validation/rect.osc";
    if (boost::filesystem::exists(filespec)) {
        ocf.readChanges(filespec);
    }
    for (auto it = std::begin(ocf.changes); it != std::end(ocf.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            auto status = plugin->checkWay(*way, "building");
            // status->dump();
            // std::cerr << way->tags["note"] << std::endl;

            // Good geometry rectangle
            if (way->id == -101790) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry rectangle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry rectangle) [geometry building]");
                }
            }

            // Good geometry complex rectangle
            if (way->id == 838311812) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry complex rectangle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry complex rectangle) [geometry building]");
                }
            }

            // Bad geometry (triangle)
            if (way->id == 824015796) {
                if (status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(bad geometry triangle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(bad geometry triangle) [geometry building]");
                }
            }

            // Good geometry (rectangle)
            if (way->id == 821663069) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry) [geometry building]");
                }
            }

            // Bad geometry rectangle
            if (way->id == -101806) {
                if (status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom rectangle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(badgeom rectangle) [geometry building]");
                }
            }

            // Good geometry big circle
            if (way->id == 856945340) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry big circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry big circle) [geometry building]");
                }
            }

            // Good geometry really big circle
            if (way->id == 821664154) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry really big circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry really big circle) [geometry building]");
                }
            }

            // Bad geometry small circle
            if (way->id == 961600809) {
                if (status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom small circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(badgeom small circle) [geometry building]");
                }
            }

            // Bad geometry really bad circle
            if (way->id == 821644720) {
                if (status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom really bad circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(badgeom really bad circle) [geometry building]");
                }
            }
        }
    }

    auto way = readOsmWayFromFile("/src/testsuite/testdata/validation/building.osc");
    auto way2 = readOsmWayFromFile("/src/testsuite/testdata/validation/building2.osc");
    auto way3 = readOsmWayFromFile("/src/testsuite/testdata/validation/rect-no-duplicate-building.osc");

    std::list<std::shared_ptr<osmobjects::OsmWay>> allways;
    allways.push_back(std::make_shared<osmobjects::OsmWay>(way2));
    std::list<std::shared_ptr<osmobjects::OsmWay>> allways2;
    allways2.push_back(std::make_shared<osmobjects::OsmWay>(way3));

    // Overlapping (function)
    if (plugin->overlaps(allways, way)) {
        runtest.pass("Validate::overlaps() [geometry building]");
    } else {
        runtest.fail("Validate::overlaps() [geometry building]");
    }
    if (plugin->overlaps(allways, way3)) {
        runtest.pass("Validate::overlaps(no) [geometry building]");
    } else {
        runtest.fail("Validate::overlaps(no) [geometry building]");
    }

    // Duplicate (function)
    allways.push_back(std::make_shared<osmobjects::OsmWay>(way));
    if (plugin->duplicate(allways, way2)) {
        runtest.pass("Validate::duplicate() [geometry building]");
    } else {
        runtest.fail("Validate::duplicate() [geometry building]");
    }
    if (!plugin->duplicate(allways2, way2)) {
        runtest.pass("Validate::duplicate(no) [geometry building]");
    } else {
        runtest.fail("Validate::duplicate(no) [geometry building]");
    }

    // Overlapping, duplicate
    osmchange::OsmChangeFile osmfoverlaping;
    const multipolygon_t poly;
    filespec = DATADIR;
    filespec += "/src/testdata/validation/rect-overlap-and-duplicate-building.osc";
    if (boost::filesystem::exists(filespec)) {
        osmfoverlaping.readChanges(filespec);
    } else {
        log_debug("Couldn't load ! %1%", filespec);
    }
    for (auto it = std::begin(osmfoverlaping.changes); it != std::end(osmfoverlaping.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto nit = std::begin(change->ways); nit != std::end(change->ways); ++nit) {
            osmobjects::OsmWay *way = nit->get();
            way->priority = true;
        }
    }
    auto wayval = osmfoverlaping.validateWays(poly, plugin);
    for (auto sit = wayval->begin(); sit != wayval->end(); ++sit) {
        auto status = *sit->get();
        if (status.hasStatus(overlaping)) {
            runtest.pass("Validate::validateWays(overlaping) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(overlaping) [geometry building]");
        }
        if (status.hasStatus(duplicate)) {
            runtest.pass("Validate::validateWays(duplicate) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(duplicate) [geometry building]");
        }
    }

    // No overlapping, no duplicate
    osmchange::OsmChangeFile osmfnooverlaping;
    filespec = DATADIR;
    filespec += "/src/testsuite/testdata/validation/rect-no-overlap-and-duplicate-building.osc";
    if (boost::filesystem::exists(filespec)) {
        osmfnooverlaping.readChanges(filespec);
    } else {
        log_debug("Couldn't load ! %1%", filespec);
    }
    for (auto it = std::begin(osmfnooverlaping.changes); it != std::end(osmfnooverlaping.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto nit = std::begin(change->ways); nit != std::end(change->ways); ++nit) {
            osmobjects::OsmWay *way = nit->get();
            way->priority = true;
        }
    }
    wayval = osmfnooverlaping.validateWays(poly, plugin);
    for (auto sit = wayval->begin(); sit != wayval->end(); ++sit) {
        auto status = *sit->get();
        if (!status.hasStatus(overlaping)) {
            runtest.pass("Validate::validateWays(no overlaping) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(no overlaping) [geometry building]");
        }
        if (!status.hasStatus(duplicate)) {
            runtest.pass("Validate::validateWays(no duplicate) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(no duplicate) [geometry building]");
        }
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
