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
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <mutex>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <string.h>

#include "utils/log.hh"

using namespace queryvalidate;
using namespace queryraw;
using namespace underpassconfig;
using namespace logger;

namespace bootstrap {

Bootstrap::Bootstrap(void) {}

std::string
Bootstrap::allTasksQueries(std::shared_ptr<std::vector<BootstrapTask>> tasks) {
    std::string queries = "";
    for (auto it = tasks->begin(); it != tasks->end(); ++it) {
        queries += it->query ;
    }
    return queries;
}

void 
Bootstrap::start(const underpassconfig::UnderpassConfig &config) {
    std::cout << "Connecting to the database ..." << std::endl;
    db = std::make_shared<Pq>();
    if (!db->connect(config.underpass_db_url)) {
        std::cout << "Could not connect to Underpass DB, aborting bootstrapping thread!" << std::endl;
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

    validator = creator();
    queryvalidate = std::make_shared<QueryValidate>(db);
    queryraw = std::make_shared<QueryRaw>(db);
    page_size = config.bootstrap_page_size;
    concurrency = config.concurrency;
    norefs = config.norefs;

    processWays();
    // processNodes();
    // processRels();

}

void 
Bootstrap::processWays() {

    std::vector<std::string> tables = {
        QueryRaw::polyTable,
        QueryRaw::lineTable
    };
    
    for (auto table_it = tables.begin(); table_it != tables.end(); ++table_it) {
        std::cout << std::endl << "Counting geometries ... " << std::endl;
        long int total = queryraw->getWaysCount(*table_it);
        long int count = 0;
        int num_chunks = total / page_size;

        std::cout << "Total: " << total << std::endl;
        std::cout << "Threads: " << concurrency << std::endl;
        std::cout << "Page size: " << page_size << std::endl;
        long lastid = 0;

        int concurrentTasks = concurrency;
        int taskIndex = 0;

        for (int chunkIndex = 1; chunkIndex <= (num_chunks/concurrentTasks); chunkIndex++) {

            int percentage = (count * 100) / total;

            auto ways = std::make_shared<std::vector<OsmWay>>();
            if (!norefs) {
                ways = queryraw->getWaysFromDB(lastid, concurrency * page_size, *table_it);
            } else {
                ways = queryraw->getWaysFromDBWithoutRefs(lastid, concurrency * page_size, *table_it);
            }

            auto tasks = std::make_shared<std::vector<BootstrapTask>>(concurrentTasks);
            boost::asio::thread_pool pool(concurrentTasks);
            for (int taskIndex = 0; taskIndex < concurrentTasks; taskIndex++) {
                auto taskWays = std::make_shared<std::vector<OsmWay>>();
                WayTask wayTask {
                    taskIndex,
                    std::ref(tasks),
                    std::ref(ways),
                };
                std::cout << "\r" << "Processing " << *table_it << ": " << count << "/" << total << " (" << percentage << "%)";
                
                boost::asio::post(pool, boost::bind(&Bootstrap::threadBootstrapTask, this, wayTask));
            }

            pool.join();

            db->query(allTasksQueries(tasks));
            lastid = ways->back().id;
            for (auto it = tasks->begin(); it != tasks->end(); ++it) {
                count += it->processed;
            }
        }
    }
    std::cout << std::endl;

}

// This thread get started for every page of way
void
Bootstrap::threadBootstrapTask(WayTask wayTask)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("bootstrap::threadBootstrapTask(wayTask): took %w seconds\n");
#endif
    auto taskIndex = wayTask.taskIndex;
    auto tasks = wayTask.tasks;
    auto ways = wayTask.ways;

    BootstrapTask task;
    int processed = 0;

    auto wayval = std::make_shared<std::vector<std::shared_ptr<ValidateStatus>>>();

    // Proccesing ways
    for (int i = 0; i < page_size; ++i) {
        if (i * taskIndex < ways->size()) {
            auto way = ways->at(i * (taskIndex + 1));
            wayval->push_back(validator->checkWay(way, "building"));
            // Fill the way_refs table
            if (!norefs) {
                for (auto ref = way.refs.begin(); ref != way.refs.end(); ++ref) {
                    task.query += "INSERT INTO way_refs (way_id, node_id) VALUES (" + std::to_string(way.id) + "," + std::to_string(*ref) + "); ";
                }
            }
            ++processed;
        }
    }
    queryvalidate->ways(wayval, task.query);
    task.processed = processed;
    const std::lock_guard<std::mutex> lock(tasks_change_mutex);
    (*tasks)[taskIndex] = task;

}

}