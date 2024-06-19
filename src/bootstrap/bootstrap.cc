//
// Copyright (c) 2023, 2024 Humanitarian OpenStreetMap Team
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

typedef boost::geometry::model::d2::point_xy<double> point_t;

namespace bootstrap {

Bootstrap::Bootstrap(void) {}

BootstrapQueries
Bootstrap::allTasksQueries(std::shared_ptr<std::vector<BootstrapTask>> tasks) {
    BootstrapQueries queries;
    for (auto it = tasks->begin(); it != tasks->end(); ++it) {
        for (auto itt = it->query.begin(); itt != it->query.end(); ++itt) {
            queries.underpass.push_back(*itt);
        }
        for (auto itt = it->osmquery.begin(); itt != it->osmquery.end(); ++itt) {
            queries.osm.push_back(*itt);
        }
    }
    return queries;
}

void
Bootstrap::start(const underpassconfig::UnderpassConfig &config) {
    std::cout << "Connecting to OSM database ... " << std::endl;
    osmdb = std::make_shared<Pq>();
    if (!osmdb->connect(config.underpass_osm_db_url)) {
        log_error("Could not connect to OSM DB, aborting bootstrapping thread!");
        return;
    }

    std::cout << "Connecting to underpass database ... " << std::endl;
    db = std::make_shared<Pq>();
    if (!db->connect(config.underpass_db_url)) {
        log_error("Could not connect to Underpass DB, aborting bootstrapping thread!");
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
    queryraw = std::make_shared<QueryRaw>(osmdb);
    page_size = config.bootstrap_page_size;
    concurrency = config.concurrency;
    norefs = config.norefs;

    processWays();
    processNodes();
    processRelations();

}

void
Bootstrap::processWays() {

    std::vector<std::string> tables = {
        QueryRaw::polyTable,
        QueryRaw::lineTable
    };

    for (auto table_it = tables.begin(); table_it != tables.end(); ++table_it) {
        std::cout << std::endl << "Processing ways ... ";
        long int total = queryraw->getCount(*table_it);
        long int count = 0;
        int num_chunks = total / page_size;

        long lastid = 0;

        int concurrentTasks = concurrency;
        int taskIndex = 0;
        int percentage = 0;

        for (int chunkIndex = 0; chunkIndex <= (num_chunks/concurrentTasks); chunkIndex++) {

            percentage = (count * 100) / total;

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

                boost::asio::post(pool, boost::bind(&Bootstrap::threadBootstrapWayTask, this, wayTask));
            }

            pool.join();

            auto queries = allTasksQueries(tasks);

            for (auto it = queries.underpass.begin(); it != queries.underpass.end(); ++it) {
                db->query(*it);
            }
            for (auto it = queries.osm.begin(); it != queries.osm.end(); ++it) {
                osmdb->query(*it);
            }

            lastid = ways->back().id;
            for (auto it = tasks->begin(); it != tasks->end(); ++it) {
                count += it->processed;
            }
        }
        percentage = (count * 100) / total;
        std::cout << "\r" << "Processing " << *table_it << ": " << count << "/" << total << " (" << percentage << "%)";
    }
    std::cout << std::endl;

}

void
Bootstrap::processNodes() {

    std::cout << "Processing nodes ... ";
    long int total = queryraw->getCount("nodes");
    long int count = 0;
    int num_chunks = total / page_size;

    long lastid = 0;

    int concurrentTasks = concurrency;
    int taskIndex = 0;
    int percentage = 0;

    for (int chunkIndex = 0; chunkIndex <= (num_chunks/concurrentTasks); chunkIndex++) {

        percentage = (count * 100) / total;

        auto nodes = std::make_shared<std::vector<OsmNode>>();
        nodes = queryraw->getNodesFromDB(lastid, concurrency * page_size);

        auto tasks = std::make_shared<std::vector<BootstrapTask>>(concurrentTasks);
        boost::asio::thread_pool pool(concurrentTasks);
        for (int taskIndex = 0; taskIndex < concurrentTasks; taskIndex++) {
            auto taskNodes = std::make_shared<std::vector<OsmNode>>();
            NodeTask nodeTask {
                taskIndex,
                std::ref(tasks),
                std::ref(nodes),
            };
            std::cout << "\r" << "Processing nodes: " << count << "/" << total << " (" << percentage << "%)";
            boost::asio::post(pool, boost::bind(&Bootstrap::threadBootstrapNodeTask, this, nodeTask));
        }

        pool.join();

        auto queries = allTasksQueries(tasks);
        for (auto it = queries.underpass.begin(); it != queries.underpass.end(); ++it) {
            db->query(*it);
        }
        for (auto it = queries.osm.begin(); it != queries.osm.end(); ++it) {
            osmdb->query(*it);
        }
        lastid = nodes->back().id;
        for (auto it = tasks->begin(); it != tasks->end(); ++it) {
            count += it->processed;
        }
    }
    percentage = (count * 100) / total;
    std::cout << "\r" << "Processing nodes: " << count << "/" << total << " (" << percentage << "%)";
    std::cout << std::endl;

}

void
Bootstrap::processRelations() {

    std::cout << "Processing relations ... ";
    long int total = queryraw->getCount("relations");
    long int count = 0;
    int num_chunks = total / page_size;

    long lastid = 0;

    int concurrentTasks = concurrency;
    int taskIndex = 0;
    int percentage = 0;

    for (int chunkIndex = 0; chunkIndex <= (num_chunks/concurrentTasks); chunkIndex++) {

        percentage = (count * 100) / total;

        auto relations = std::make_shared<std::vector<OsmRelation>>();
        relations = queryraw->getRelationsFromDB(lastid, concurrency * page_size);

        auto tasks = std::make_shared<std::vector<BootstrapTask>>(concurrentTasks);
        boost::asio::thread_pool pool(concurrentTasks);
        for (int taskIndex = 0; taskIndex < concurrentTasks; taskIndex++) {
            auto taskRelations = std::make_shared<std::vector<OsmRelation>>();
            RelationTask relationTask {
                taskIndex,
                std::ref(tasks),
                std::ref(relations),
            };
            std::cout << "\r" << "Processing relations: " << count << "/" << total << " (" << percentage << "%)";
            boost::asio::post(pool, boost::bind(&Bootstrap::threadBootstrapRelationTask, this, relationTask));
        }

        pool.join();

        auto queries = allTasksQueries(tasks);
        for (auto it = queries.underpass.begin(); it != queries.underpass.end(); ++it) {
            db->query(*it);
        }
        for (auto it = queries.osm.begin(); it != queries.osm.end(); ++it) {
            osmdb->query(*it);
        }
        lastid = relations->back().id;
        for (auto it = tasks->begin(); it != tasks->end(); ++it) {
            count += it->processed;
        }
    }
    percentage = (count * 100) / total;
    std::cout << "\r" << "Processing relations: " << count << "/" << total << " (" << percentage << "%)";

    std::cout << std::endl;

}

// This thread get started for every page of way
void
Bootstrap::threadBootstrapWayTask(WayTask wayTask)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("bootstrap::threadBootstrapWayTask(wayTask): took %w seconds\n");
#endif
    auto taskIndex = wayTask.taskIndex;
    auto tasks = wayTask.tasks;
    auto ways = wayTask.ways;

    BootstrapTask task;
    int processed = 0;

    auto wayval = std::make_shared<std::vector<std::shared_ptr<ValidateStatus>>>();

    // Proccesing ways
    for (size_t i = taskIndex * page_size; i < (taskIndex + 1) * page_size; ++i) {
        if (i < ways->size()) {
            auto way = ways->at(i);
            wayval->push_back(validator->checkWay(way, "building"));
            ++processed;
        }
    }

    auto result = queryvalidate->ways(wayval);
    for (auto it = result->begin(); it != result->end(); ++it) {
        task.query.push_back(*it);
    }

    task.processed = processed;
    const std::lock_guard<std::mutex> lock(tasks_change_mutex);
    (*tasks)[taskIndex] = task;

}

// This thread get started for every page of node
void
Bootstrap::threadBootstrapNodeTask(NodeTask nodeTask)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("bootstrap::threadBootstrapNodeTask(nodeTask): took %w seconds\n");
#endif
    auto taskIndex = nodeTask.taskIndex;
    auto tasks = nodeTask.tasks;
    auto nodes = nodeTask.nodes;

    BootstrapTask task;
    int processed = 0;

    auto nodeval = std::make_shared<std::vector<std::shared_ptr<ValidateStatus>>>();

    // Proccesing nodes
    std::vector<std::string> node_tests = {"building", "natural", "place", "waterway"};
    for (size_t i = taskIndex * page_size; i < (taskIndex + 1) * page_size; ++i) {
        if (i < nodes->size()) {
            auto node = nodes->at(i);
            for (auto test_it = std::begin(node_tests); test_it != std::end(node_tests); ++test_it) {
                if (node.containsKey(*test_it)) {
                    nodeval->push_back(validator->checkNode(node, *test_it));
                }
            }
            ++processed;
        }
    }

    auto result = queryvalidate->nodes(nodeval);
    for (auto it = result->begin(); it != result->end(); ++it) {
        task.query.push_back(*it);
    }

    task.processed = processed;
    const std::lock_guard<std::mutex> lock(tasks_change_mutex);
    (*tasks)[taskIndex] = task;

}

// This thread get started for every page of relation
void
Bootstrap::threadBootstrapRelationTask(RelationTask relationTask)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("bootstrap::threadBootstrapRelationTask(relationTask): took %w seconds\n");
#endif
    auto taskIndex = relationTask.taskIndex;
    auto tasks = relationTask.tasks;
    auto relations = relationTask.relations;

    BootstrapTask task;
    int processed = 0;

    auto relationval = std::make_shared<std::vector<std::shared_ptr<ValidateStatus>>>();

    // Proccesing relations
    for (size_t i = taskIndex * page_size; i < (taskIndex + 1) * page_size; ++i) {
        if (i < relations->size()) {
            auto relation = relations->at(i);
            // relationval->push_back(validator->checkRelation(way, "building"));
            // Fill the rel_refs table
            for (auto mit = relation.members.begin(); mit != relation.members.end(); ++mit) {
                task.osmquery.push_back("INSERT INTO rel_refs (rel_id, way_id) VALUES (" + std::to_string(relation.id) + "," + std::to_string(mit->ref) + "); ");
            }
            ++processed;
        }
    }
    // queryvalidate->relations(relationval, task.query);
    task.processed = processed;
    const std::lock_guard<std::mutex> lock(tasks_change_mutex);
    (*tasks)[taskIndex] = task;

}

}
