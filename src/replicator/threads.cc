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

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
#include "unconfig.h"
#endif

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <range/v3/all.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <chrono>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/pthread/shared_mutex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/version.hpp>
#include <boost/date_time.hpp>
#include <boost/dll/import.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/circular_buffer.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
using tcp = net::ip::tcp;         // from <boost/asio/ip/tcp.hpp>

#include <boost/config.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/function.hpp>
#include <boost/timer/timer.hpp>

#include "osm/osmobjects.hh"
#include "replicator/threads.hh"
#include "utils/log.hh"
#include "osm/changeset.hh"
#include "osm/osmchange.hh"
#include "stats/querystats.hh"
#include "validate/queryvalidate.hh"
#include "replicator/replication.hh"
#include "validate/validate.hh"
#include <jemalloc/jemalloc.h>
#include "data/pq.hh"

std::mutex stream_mutex;

using namespace logger;
using namespace stats;
using namespace queryvalidate;

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace threads {

std::string
allTasksQueries(std::shared_ptr<std::vector<ReplicationTask>> tasks) {
    std::string queries = "";
    for (auto it = tasks->begin(); it != tasks->end(); ++it) {
        queries += it->query;
    }
    return queries;
}

// Get closest change from a list of tasks
std::shared_ptr<ReplicationTask>
getClosest(std::shared_ptr<std::vector<ReplicationTask>> tasks, ptime now) {
    ReplicationTask closest;
    if (tasks->size() > 0) {
        closest = tasks->at(0);
        for (auto it = tasks->begin(); it != tasks->end(); ++it) {
            if (it->timestamp != not_a_date_time) {
                boost::posix_time::time_duration delta = now - it->timestamp;
                boost::posix_time::time_duration delta_oldest = now - closest.timestamp;
                if (delta.hours() * 60 + delta.minutes() < delta_oldest.hours() * 60 + delta_oldest.minutes()) {
                    closest = *it;
                }
            }
        }
    }
    return std::make_shared<ReplicationTask>(closest);
}

// Starting with this URL, download the file, incrementing
void
startMonitorChangesets(std::shared_ptr<replication::RemoteURL> &remote,
               const multipolygon_t &poly,
               const underpassconfig::UnderpassConfig &config)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("startMonitorChangesets: took %w seconds\n");
#endif
    // This function is for changesets only!
    assert(remote->frequency == frequency_t::changeset);

    int cores = config.concurrency;

    pq::Pq db = pq::Pq();
    if (!db.connect(config.underpass_db_url)) {
        log_error("Could not connect to Underpass DB, aborting monitoring thread!");
        return;
    }

    // Support multiple OSM planet servers
    std::vector<std::shared_ptr<replication::Planet>> planets;
    std::vector<std::string> servers;
    for (auto it = std::begin(config.planet_servers); it != std::end(config.planet_servers); ++it) {
        servers.push_back(it->domain);
    }

    // Rotate OSM Planet servers
    int i = 0;
    while (i <= cores/4) {
        std::rotate(servers.begin(), servers.begin()+1, servers.end());
        auto replicationPlanet = std::make_shared<replication::Planet>(*remote);
        replicationPlanet->connectServer(servers.front());
        planets.push_back(replicationPlanet);
        i++;
    }

    // Process Changesets replication files
    ReplicationTask closest;
    auto delay = std::chrono::seconds{0};
    auto last_task = std::make_shared<ReplicationTask>();
    bool caughtUpWithNow = false;
    bool monitoring = true;
    while (monitoring) {
        auto tasks = std::make_shared<std::vector<ReplicationTask>>();
        i = cores*2;
        boost::asio::thread_pool pool(i);
        while (--i) {
            std::this_thread::sleep_for(delay);
            if (last_task->status == reqfile_t::success ||
                (last_task->status == reqfile_t::remoteNotFound && !caughtUpWithNow)) {
                remote->Increment();
            }
            auto task = boost::bind(threadChangeSet,
                std::make_shared<replication::RemoteURL>(remote->getURL()),
                std::ref(planets.front()),
                std::ref(poly),
                std::ref(tasks)
            );
            boost::asio::post(pool, task);
            std::rotate(planets.begin(), planets.begin()+1, planets.end());
            remote->updateDomain(planets.front()->domain);
        }

        pool.join();
        db.query(allTasksQueries(tasks));

        ptime now  = boost::posix_time::second_clock::universal_time();
        last_task = getClosest(tasks, now);
        if (last_task->timestamp != not_a_date_time) {
            closest.url = std::string(last_task->url);
            closest.timestamp = ptime(last_task->timestamp);
            if (last_task->timestamp >= config.end_time) {
                monitoring = false;
            }
        }
        if (!caughtUpWithNow) {
            // Check if caught up with now
            boost::posix_time::time_duration delta_closest = now - closest.timestamp;
            if (delta_closest.hours() * 60 + delta_closest.minutes() <= 2) {
                caughtUpWithNow = true;
                log_debug("Caught up with: %1%", closest.url);
                remote->updatePath(
                    std::stoi(closest.url.substr(0, 3)),
                    std::stoi(closest.url.substr(4, 3)),
                    std::stoi(closest.url.substr(8, 3))
                );
                cores = 1;
                delay = std::chrono::seconds{45};
            }
        }
    }
}

// Starting with this URL, download the file, incrementing
void
startMonitorChanges(std::shared_ptr<replication::RemoteURL> &remote,
            const multipolygon_t &poly,
            const underpassconfig::UnderpassConfig &config)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("startMonitorChanges: took %w seconds\n");
#endif
    if (remote->frequency == frequency_t::changeset) {
        log_error("Could not start monitoring thread for OSM changes: URL %1% does not appear to be a valid URL for changes!", remote->filespec);
        return;
    }

    std::string plugins;
    if (boost::filesystem::exists("validate/.libs")) {
        plugins = "validate/.libs";
    } else {
        plugins = PKGLIBDIR;
    }
    boost::dll::fs::path lib_path(plugins);
    boost::function<plugin_t> creator;
    try {
        creator = boost::dll::import_alias<plugin_t>(lib_path / "libhotosm", "create_plugin", boost::dll::load_mode::append_decorations);
        log_debug("Loaded plugin hotosm!");
    } catch (std::exception &e) {
        log_debug("Couldn't load plugin! %1%", e.what());
        exit(0);
    }
    auto validator = creator();

#ifdef MEMORY_DEBUG
    size_t sz, active1, active2;
#endif    // JEMALLOC memory debugging
    pq::Pq db = pq::Pq();
    if (!db.connect(config.underpass_db_url)) {
        log_error("Could not connect to Underpass DB, aborting monitoring thread!");
        return;
    }
    std::vector<std::string> servers;
    for (auto it = std::begin(config.planet_servers); it != std::end(config.planet_servers); ++it) {
        servers.push_back(it->domain);
    }
    std::vector<std::shared_ptr<replication::Planet>> planets;
    int cores = config.concurrency;
    int i = 0;
    while (i <= cores/4) {
        std::rotate(servers.begin(), servers.begin()+1, servers.end());
        auto replicationPlanet = std::make_shared<replication::Planet>(*remote);
        replicationPlanet->connectServer(servers.front());
        planets.push_back(replicationPlanet);
        i++;
    }

    // Process OSM changes
    auto delay = std::chrono::seconds{0};
    ReplicationTask closest;
    auto last_task = std::make_shared<ReplicationTask>();
    bool caughtUpWithNow = false;
    bool monitoring = true;
    while (monitoring) {
        auto tasks = std::make_shared<std::vector<ReplicationTask>>();
        i = cores*2;
        boost::asio::thread_pool pool(i);
        while (--i) {
            std::this_thread::sleep_for(delay);
            if (last_task->status == reqfile_t::success ||
                (last_task->status == reqfile_t::remoteNotFound && !caughtUpWithNow)) {
                remote->Increment();
            }
            auto task = boost::bind(threadOsmChange,
                std::make_shared<replication::RemoteURL>(remote->getURL()),
                std::ref(planets.front()),
                std::ref(poly),
                std::ref(validator),
                std::ref(tasks)
            );
            boost::asio::post(pool, task);
            std::rotate(planets.begin(), planets.begin()+1, planets.end());
        }
        
        pool.join();
        db.query(allTasksQueries(tasks));

        ptime now  = boost::posix_time::second_clock::universal_time();
        last_task = getClosest(tasks, now);
        if (last_task->timestamp != not_a_date_time) {
            closest.url = std::string(last_task->url);
            closest.timestamp = ptime(last_task->timestamp);
            if (last_task->timestamp >= config.end_time) {
                monitoring = false;
            }
        }
        // Check if caught up with now
        if (!caughtUpWithNow) {
            boost::posix_time::time_duration delta_closest = now - closest.timestamp;
            if (delta_closest.hours() * 60 + delta_closest.minutes() <= 2) {
                caughtUpWithNow = true;
                log_debug("Caught up with: %1%", closest.url);
                remote->updatePath(
                    std::stoi(closest.url.substr(0, 3)),
                    std::stoi(closest.url.substr(4, 3)),
                    std::stoi(closest.url.substr(8, 3))
                );
                cores = 1;
                delay = std::chrono::seconds{45};
            }
        }
    }
}

// This parses the changeset file into changesets
void
threadChangeSet(std::shared_ptr<replication::RemoteURL> &remote,
        std::shared_ptr<replication::Planet> &planet,
        const multipolygon_t &poly,
        std::shared_ptr<std::vector<ReplicationTask>> tasks)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("threadChangeSet: took %w seconds\n");
#endif
    ReplicationTask task;
    task.url = remote->subpath;
    auto file = planet->downloadFile(*remote.get());
    task.status = file.status;
    stats::QueryStats querystats;

    if (file.status == reqfile_t::success) {
        auto changeset = std::make_unique<changeset::ChangeSetFile>();
        log_debug("Processing ChangeSet: %1%", remote->filespec);
        auto xml = planet->processData(remote->filespec, *file.data);
        std::istream& input(xml);
        changeset->readXML(input);
        if (changeset->last_closed_at != not_a_date_time) {
            task.timestamp = changeset->last_closed_at;
        } else if (changeset->changes.size() && changeset->changes.back()->created_at != not_a_date_time) {
            task.timestamp = changeset->changes.back()->created_at;
        }
        log_debug("ChangeSet last_closed_at: %1%", task.timestamp);
        changeset->areaFilter(poly);
        for (auto cit = std::begin(changeset->changes); cit != std::end(changeset->changes); ++cit) {
            task.query = querystats.applyChange(*cit->get());
        }
    }
    const std::lock_guard<std::mutex> lock(tasks_changeset_mutex);
    tasks->push_back(task);
}

// This thread get started for every osmChange file
void
threadOsmChange(std::shared_ptr<replication::RemoteURL> &remote,
        std::shared_ptr<replication::Planet> &planet,
        const multipolygon_t &poly,
        std::shared_ptr<Validate> &plugin,
        std::shared_ptr<std::vector<ReplicationTask>> tasks)
{
    auto osmchanges = std::make_shared<osmchange::OsmChangeFile>();
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("threadOsmChange: took %w seconds\n");
#endif
    log_debug("Processing OsmChange: %1%", remote->filespec);
    ReplicationTask task;
    task.url = remote->subpath;
    auto file = planet->downloadFile(*remote.get());
    task.status = file.status;
    QueryStats querystats;
    QueryValidate queryvalidate;

    if (file.status == replication::success) {
        try {
            std::istringstream changes_xml;
            // Scope to deallocate buffers
            {
                boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
                inbuf.push(boost::iostreams::gzip_decompressor());
                boost::iostreams::array_source arrs{reinterpret_cast<char const *>(file.data->data()), file.data->size()};
                inbuf.push(arrs);
                std::istream instream(&inbuf);
                changes_xml.str(std::string{std::istreambuf_iterator<char>(instream), {}});
            }

            try {
                osmchanges->readXML(changes_xml);  
                if (osmchanges->changes.size() > 0) {
                    task.timestamp = osmchanges->changes.back()->final_entry;
                    log_debug("OsmChange final_entry: %1%", task.timestamp);
                }
            } catch (std::exception &e) {
                log_error("Couldn't parse: %1%", remote->filespec);
                boost::filesystem::remove(remote->filespec);
                std::cerr << e.what() << std::endl;
            }

        } catch (std::exception &e) {
            log_error("%1% is corrupted!", remote->filespec);
            boost::filesystem::remove(remote->filespec);
            std::cerr << e.what() << std::endl;
        }
    }

    osmchanges->areaFilter(poly);

    // These stats are for the entire file
    auto stats = osmchanges->collectStats(poly);
    for (auto it = std::begin(*stats); it != std::end(*stats); ++it) {
        if (it->second->added.size() == 0 && it->second->modified.size() == 0) {
            continue;
        }
        task.query = querystats.applyChange(*it->second);
    }

    auto removals = std::make_shared<std::vector<long>>();
    // Delete existing entries in the validation table to remove features that have been fixed
    for (auto it = std::begin(osmchanges->changes); it != std::end(osmchanges->changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            if (way->action == osmobjects::remove) {
                removals->push_back(way->id);
            }
        }
        for (auto nit = std::begin(change->nodes); nit != std::end(change->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
            if (node->action == osmobjects::remove) {
                removals->push_back(node->id);
            }
        }
    }

    task.query += queryvalidate.updateValidation(removals);

    auto nodeval = osmchanges->validateNodes(poly, plugin);
    for (auto it = nodeval->begin(); it != nodeval->end(); ++it) {
        task.query += queryvalidate.applyChange(*it->get());
    }
    auto wayval = osmchanges->validateWays(poly, plugin);
    for (auto it = wayval->begin(); it != wayval->end(); ++it) {
        task.query += queryvalidate.applyChange(*it->get());
    }

    const std::lock_guard<std::mutex> lock(tasks_change_mutex);
    tasks->push_back(task);

}

} // namespace threads

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
