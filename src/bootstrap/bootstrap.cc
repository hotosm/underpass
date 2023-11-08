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

    std::cout << "Connecting to the database (" << config.underpass_db_url << ") ..." << std::endl;
    auto db = std::make_shared<Pq>();
    if (!db->connect(config.underpass_db_url)) {
        log_error("Could not connect to Underpass DB, aborting monitoring thread!");
        return;
    }

    std::cout << "Loading plugins ... " << std::endl;
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
        log_debug("Loaded plugin!");
    } catch (std::exception &e) {
        log_debug("Couldn't load plugin! %1%", e.what());
        exit(0);
    }
    auto validator = creator();

    auto queryvalidate = std::make_shared<QueryValidate>(db);
    auto queryraw = std::make_shared<QueryRaw>(db);
    std::vector<std::string> tables = {
        QueryRaw::polyTable,
        QueryRaw::lineTable
    };
    
    for (auto table_it = tables.begin(); table_it != tables.end(); ++table_it) {
        std::cout << std::endl << "Counting geometries ... " << std::endl;
        int total = queryraw->getWaysCount(*table_it);
        std::cout << "Total: " << total << std::endl;
        if (total > 0) {
            int count = 0;
            long lastid = 0;
            while (count < total) {
                int percentage = (count * 100) / total;
                std::cout << "\r" << "Processing " << *table_it << ": " << count << "/" << total << " (" << percentage << "%)";
                auto task = std::make_shared<BootstrapTask>();
                WayTask wayTask;
                wayTask.plugin = validator;
                wayTask.queryvalidate = queryvalidate;
                wayTask.queryraw = queryraw;
                wayTask.task = task;
                wayTask.lastid = lastid;
                
                processWays(wayTask, *table_it, config);
                db->query(task->query);
                lastid = wayTask.lastid;
                count += wayTask.processed;
            }
        }
    }

}

// This thread get started for every page of way
void
processWays(WayTask &wayTask, const std::string &tableName, const underpassconfig::UnderpassConfig &config)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("bootstrap::processWays(wayTask): took %w seconds\n");
#endif

    auto plugin = wayTask.plugin;
    auto task = wayTask.task;
    auto queryvalidate = wayTask.queryvalidate;
    auto queryraw = wayTask.queryraw;
    auto lastid = wayTask.lastid;

    auto ways = std::make_shared<std::vector<OsmWay>>();
    if (!config.norefs) {
        ways = queryraw->getWaysFromDB(lastid, tableName);
    } else {
        ways = queryraw->getWaysFromDBWithoutRefs(lastid, tableName);
    }
    wayTask.processed = ways->size();
    if (wayTask.processed > 0) {
        // Proccesing ways
        for (auto way = ways->begin(); way != ways->end(); ++way) {
            auto status = plugin->checkWay(*way, "building");
            for (auto status_it = status->status.begin(); status_it != status->status.end(); ++status_it) {
                task->query += queryvalidate->applyChange(*status, *status_it);
            }
            // Fill the way_refs table
            if (!config.norefs) {
                for (auto ref = way->refs.begin(); ref != way->refs.end(); ++ref) {
                    task->query += "INSERT INTO way_refs (way_id, node_id) VALUES (" + std::to_string(way->id) + "," + std::to_string(*ref) + "); ";
                }
            }
        }
        wayTask.lastid = ways->back().id;
    }

}

}