//
// Copyright (c) 2023 Humanitarian OpenStreetMap Team
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

#include "validate/queryvalidate.hh"
#include "raw/queryraw.hh"
#include "data/pq.hh"
#include "bootstrap/bootstrap.hh"
#include "underpassconfig.hh"

#include <boost/filesystem.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/function.hpp>
#include <boost/dll/import.hpp>
#include <boost/timer/timer.hpp>
#include <string.h>

#include "utils/log.hh"

using namespace queryvalidate;
using namespace queryraw;
using namespace underpassconfig;
using namespace logger;

namespace bootstrap {

void startProcessingWays(const underpassconfig::UnderpassConfig &config) {

    auto db = std::make_shared<Pq>();
    if (!db->connect(config.underpass_db_url)) {
        log_error("Could not connect to Underpass DB, aborting monitoring thread!");
        return;
    }

    std::string plugins;
    if (boost::filesystem::exists("src/validate/.libs")) {
        plugins = "src/validate/.libs";
    } else {
        plugins = PKGLIBDIR;
    }
    boost::dll::fs::path lib_path(plugins);
    boost::function<plugin_t> creator;
    try {
        creator = boost::dll::import_alias<plugin_t>(lib_path / "libunderpass.so", "create_plugin", boost::dll::load_mode::append_decorations);
        log_debug("Loaded plugin hotosm!");
    } catch (std::exception &e) {
        log_debug("Couldn't load plugin! %1%", e.what());
        exit(0);
    }
    auto validator = creator();

    auto queryvalidate = std::make_shared<QueryValidate>(db);
    auto queryraw = std::make_shared<QueryRaw>(db);
    
    int total = queryraw->getWaysCount();
    
    if (total > 0) {
        int count = 0;
        long lastid = 0;
        while (count <= total) {
            int percentage = (count * 100) / total;
            std::cout << "\r" << "Processing : " << count << "/" << total << " (" << percentage << "%)";
            auto task = std::make_shared<BootstrapTask>();
            WayTask wayTask;
            wayTask.plugin = validator;
            wayTask.queryvalidate = queryvalidate;
            wayTask.queryraw = queryraw;
            wayTask.task = task;
            wayTask.lastid = lastid;
            processWays(wayTask);
            db->query(task->query);
            lastid = wayTask.lastid;
            count += wayTask.processed;
        }
    }

}

// This thread get started for every page of way
void
processWays(WayTask &wayTask)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("bootstrap::processWays(wayTask): took %w seconds\n");
#endif

    auto plugin = wayTask.plugin;
    auto task = wayTask.task;
    auto queryvalidate = wayTask.queryvalidate;
    auto queryraw = wayTask.queryraw;
    auto lastid = wayTask.lastid;

    auto ways = queryraw->getWaysFromDB(lastid);
    wayTask.processed = ways->size();

    // Proccesing ways
    for (auto way = ways->begin(); way != ways->end(); ++way) {
        if (way->refs.front() == way->refs.back()) {
            log_debug("Way Id: %1%", way->id);

            // Bad geometry
            if (way->containsKey("building") && (boost::geometry::num_points(way->linestring) - 1 < 4 ||
                plugin->unsquared(way->linestring))
            ) {
                auto status = ValidateStatus(*way);
                status.timestamp = boost::posix_time::microsec_clock::universal_time();
                status.source = "building";
                boost::geometry::centroid(way->linestring, status.center);
                task->query += queryvalidate->applyChange(status, badgeom);
            }

            // Fill the way_refs table
            for (auto ref = way->refs.begin(); ref != way->refs.end(); ++ref) {
                task->query += "INSERT INTO way_refs (way_id, node_id) VALUES (" + std::to_string(way->id) + "," + std::to_string(*ref) + "); ";
            }
        }
    }
    wayTask.lastid = ways->back().id;

}

}