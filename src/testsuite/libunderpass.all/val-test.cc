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
#include <unistd.h>
#include <fcntl.h>

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/dll/import.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "validate/defaultvalidation.hh"
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

int test_semantic(std::shared_ptr<Validate> &plugin);
int test_geospatial(std::shared_ptr<Validate> &plugin);

int
main(int argc, char *argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("val-test.log");
    dbglogfile.setVerbosity(3);

    std::string plugins(PKGLIBDIR);
    boost::dll::fs::path lib_path(plugins);
    boost::function<plugin_t> creator;
    try {
        creator = boost::dll::import_alias<plugin_t>(
            lib_path / "libunderpass.so", "create_plugin",
            boost::dll::load_mode::append_decorations
        );
        log_debug("Loaded plugin!");
    } catch (std::exception& e) {
        log_debug("Couldn't load plugin! %1%", e.what());
        exit(0);
    }
    std::string testValidationConfig = DATADIR;
    testValidationConfig += "/testsuite/testdata/validation/config";
    auto plugin = creator();
    plugin->loadConfig(testValidationConfig);

    test_semantic(plugin);
    test_geospatial(plugin);
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
    osmchange.buildGeometriesFromNodeCache();
    return *osmchange.changes.front().get()->ways.front().get();
}

int
test_semantic(std::shared_ptr<Validate> &plugin) {

    // Node - checkNode()

    osmobjects::OsmNode node;
    node.id = 11111;
    node.changeset = 22222;

    // Node with no tags
    auto status = plugin->checkNode(node, "building");
    if (status->osm_id == 11111 && status->hasStatus(notags)) {
        runtest.pass("Validate::checkNode(no tags) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(no tags) [semantic building]");
        return 1;
    }

    // Node with tag with no value
    node.addTag("building", "");
    status = plugin->checkNode(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(no value) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(no value) [semantic building]");
        return 1;
    }

    node.addTag("building", "\"yes\"");
    status = plugin->checkNode(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(double quotes) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(double quotes) [semantic building]");
        return 1;
    }

    node.addTag("building", "'yes'");
    status = plugin->checkNode(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(single quotes) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(single quotes) [semantic building]");
        return 1;
    }

    node.addTag("building", "yes ");
    status = plugin->checkNode(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(extra space) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(extra space) [semantic building]");
        return 1;
    }

    node.addTag("building", "yes");
    status = plugin->checkNode(node, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(good value) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(good value) [semantic building]");
        return 1;
    }

    osmobjects::OsmNode node_place;
    node_place.id = 11111;
    node_place.changeset = 22222;

    // Has valid tags, but it's incomplete
    node_place.addTag("place", "city");
    status = plugin->checkNode(node_place, "place");

    if (!status->hasStatus(badvalue) && status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkNode(incomplete but correct tagging) [semantic place]");
    } else {
        runtest.fail("Validate::checkNode(incomplete but correct tagging) [semantic place]");
        return 1;
    }

    node_place.addTag("name", "Electric City");
    status = plugin->checkNode(node_place, "place");
    if (!status->hasStatus(badvalue) && !status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkNode(complete and correct tagging) [semantic place]");
    } else {
        runtest.fail("Validate::checkNode(complete and correct tagging) [semantic place]");
        return 1;
    }

    node.addTag("building", "yes");
    // Has an invalid key=value ...
    node.addTag("building:material", "sponge");
    status = plugin->checkNode(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(bad value) [semantic building]");
        return 1;
    }

    // But it's complete
    status = plugin->checkNode(node, "building");
    if (!status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkNode(complete) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(complete) [semantic building]");
        return 1;
    }

    node.addTag("building:material", "wood");
    node.addTag("building:levels", "3");
    node.addTag("roof:material", "roof_tiles");

    // Has all valid tags
    status = plugin->checkNode(node, "building");
    if (!status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(no bad values) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(no bad values) [semantic building]");
        return 1;
    }

    // Has an invalid key=value
    node.addTag("__building", "yes");
    status = plugin->checkNode(node, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkNode(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkNode(bad value) [semantic building]");
        return 1;
    }

    // Way - checkWay()

    auto way = readOsmWayFromFile("/testsuite/testdata/validation/building.osc");

    status = plugin->checkWay(way, "building");
    // Way with no tags
    if (status->hasStatus(notags)) {
        runtest.pass("Validate::checkWay(no tags) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(no tags) [semantic building]");
        return 1;
    }

    way.addTag("building", "yes");

    // // Has an invalid key=value ...
    way.addTag("building:material", "sponge");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(bad value) [semantic building]");
        return 1;
    }

    // But it's complete
    way.addTag("building:material", "sponge");
    status = plugin->checkWay(way, "building");
    if (!status->hasStatus(incomplete)) {
        runtest.pass("Validate::checkWay(bad value) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(bad value) [semantic building]");
        return 1;
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
        return 1;
    }

    // Has spaces
    way.addTag("building:material", "made of wood");
    status = plugin->checkWay(way, "building");
    if (status->hasStatus(badvalue)) {
        runtest.pass("Validate::checkWay(bad values) [semantic building]");
    } else {
        runtest.fail("Validate::checkWay(bad values) [semantic building]");
        return 1;
    }

    return 0;

}

// Geometry tests for buildings
int
test_geospatial(std::shared_ptr<Validate> &plugin)
{

    // Bad geometry

    osmchange::OsmChangeFile ocf;
    std::string filespec = DATADIR;
    filespec += "/testsuite/testdata/validation/rect.osc";
    if (boost::filesystem::exists(filespec)) {
        ocf.readChanges(filespec);
        ocf.buildGeometriesFromNodeCache();
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
                    return 1;
                }
            }

            // Good geometry complex rectangle
            if (way->id == 838311812) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry complex rectangle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry complex rectangle) [geometry building]");
                    return 1;
                }
            }

            // Bad geometry (triangle)
            if (way->id == 824015796) {
                if (status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(bad geometry triangle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(bad geometry triangle) [geometry building]");
                    return 1;
                }
            }

            // Good geometry (rectangle)
            if (way->id == 821663069) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry) [geometry building]");
                    return 1;
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
                    return 1;
                }
            }

            // Good geometry really big circle
            if (way->id == 821664154) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry really big circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry really big circle) [geometry building]");
                    return 1;
                }
            }

            // Bad geometry small circle
            if (way->id == 961600809) {
                if (!status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(good geometry circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(good geometry small circle) [geometry building]");
                    return 1;
                }
            }

            // Bad geometry really bad circle
            if (way->id == 821644720) {
                if (status->hasStatus(badgeom)) {
                    runtest.pass("Validate::checkWay(badgeom really bad circle) [geometry building]");
                } else {
                    runtest.fail("Validate::checkWay(badgeom really bad circle) [geometry building]");
                    return 1;
                }
            }
        }
    }

    auto way = readOsmWayFromFile("/testsuite/testdata/validation/building.osc");
    auto way2 = readOsmWayFromFile("/testsuite/testdata/validation/building2.osc");
    auto way3 = readOsmWayFromFile("/testsuite/testdata/validation/rect-no-duplicate-building.osc");

    std::list<std::shared_ptr<osmobjects::OsmWay>> allways;
    allways.push_back(std::make_shared<osmobjects::OsmWay>(way2));
    std::list<std::shared_ptr<osmobjects::OsmWay>> allways2;
    allways2.push_back(std::make_shared<osmobjects::OsmWay>(way3));

    // Overlapping (function)
    // if (plugin->overlaps(allways, way)) {
    //     runtest.pass("Validate::overlaps() [geometry building]");
    // } else {
    //     runtest.fail("Validate::overlaps() [geometry building]");
    // }
    // if (plugin->overlaps(allways, way3)) {
    //     runtest.pass("Validate::overlaps(no) [geometry building]");
    // } else {
    //     runtest.fail("Validate::overlaps(no) [geometry building]");
    // }

    // Duplicate (function)
    // allways.push_back(std::make_shared<osmobjects::OsmWay>(way));
    // if (plugin->duplicate(allways, way2)) {
    //     runtest.pass("Validate::duplicate() [geometry building]");
    // } else {
    //     runtest.fail("Validate::duplicate() [geometry building]");
    // }
    // if (!plugin->duplicate(allways2, way2)) {
    //     runtest.pass("Validate::duplicate(no) [geometry building]");
    // } else {
    //     runtest.fail("Validate::duplicate(no) [geometry building]");
    // }

    // Overlapping, duplicate
    osmchange::OsmChangeFile osmfoverlapping;
    const multipolygon_t poly;
    filespec = DATADIR;
    filespec += "/testdata/validation/rect-overlap-and-duplicate-building.osc";
    if (boost::filesystem::exists(filespec)) {
        osmfoverlapping.readChanges(filespec);
        osmfoverlapping.buildGeometriesFromNodeCache();
    } else {
        log_debug("Couldn't load ! %1%", filespec);
    }
    for (auto it = std::begin(osmfoverlapping.changes); it != std::end(osmfoverlapping.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto nit = std::begin(change->ways); nit != std::end(change->ways); ++nit) {
            osmobjects::OsmWay *way = nit->get();
            way->priority = true;
        }
    }
    auto wayval = osmfoverlapping.validateWays(poly, plugin);
    for (auto sit = wayval->begin(); sit != wayval->end(); ++sit) {
        auto status = *sit->get();
        if (status.hasStatus(overlapping)) {
            runtest.pass("Validate::validateWays(overlapping) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(overlapping) [geometry building]");
            return 1;
        }
        if (status.hasStatus(duplicate)) {
            runtest.pass("Validate::validateWays(duplicate) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(duplicate) [geometry building]");
            return 1;
        }
    }

    // No overlapping, no duplicate
    osmchange::OsmChangeFile osmfnooverlapping;
    filespec = DATADIR;
    filespec += "/testsuite/testdata/validation/rect-no-overlap-and-duplicate-building.osc";
    if (boost::filesystem::exists(filespec)) {
        osmfnooverlapping.readChanges(filespec);
    } else {
        log_debug("Couldn't load ! %1%", filespec);
    }
    for (auto it = std::begin(osmfnooverlapping.changes); it != std::end(osmfnooverlapping.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto nit = std::begin(change->ways); nit != std::end(change->ways); ++nit) {
            osmobjects::OsmWay *way = nit->get();
            way->priority = true;
        }
    }
    wayval = osmfnooverlapping.validateWays(poly, plugin);
    for (auto sit = wayval->begin(); sit != wayval->end(); ++sit) {
        auto status = *sit->get();
        if (!status.hasStatus(overlapping)) {
            runtest.pass("Validate::validateWays(no overlapping) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(no overlapping) [geometry building]");
            return 1;
        }
        if (!status.hasStatus(duplicate)) {
            runtest.pass("Validate::validateWays(no duplicate) [geometry building]");
        } else {
            runtest.fail("Validate::validateWays(no duplicate) [geometry building]");
            return 1;
        }
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
