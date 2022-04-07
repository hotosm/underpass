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

#include "data/osmobjects.hh"
#include "data/threads.hh"
#include "hottm.hh"
#include "log.hh"
#include "galaxy/changeset.hh"
#include "galaxy/osmchange.hh"
#include "galaxy/galaxy.hh"
#include "galaxy/replication.hh"
#include "validate/validate.hh"
#include <jemalloc/jemalloc.h>

std::mutex stream_mutex;

using namespace logger;
using namespace galaxy;
using namespace tmdb;

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace threads {

std::mutex remove_mutex;
std::mutex time_mutex;
static ptime lastosc;
static ptime lastosm;
static bool osmdone =  false;
static bool oscdone =  false;


// Starting with this URL, download the file, incrementing
void
startMonitorChangesets(std::shared_ptr<replication::RemoteURL> &remote,
		       const multipolygon_t &poly,
		       const replicatorconfig::ReplicatorConfig &config)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("startMonitorChangesets: took %w seconds\n");
#endif
    // This function is for changesets only!
    assert(inr.frequency == frequency_t::changeset);

    std::shared_ptr<galaxy::QueryGalaxy> ostats = std::make_shared<galaxy::QueryGalaxy>();
    if (!ostats->connect(config.galaxy_db_url)) {
        log_error(_("Could not connect to galaxy DB, aborting monitoring thread!"));
        return;
    }

    // osm2pgsql::Osm2Pgsql osm2pgsql(config.osm2pgsql_db_url);

    // Support multiple database connections
    std::vector<std::shared_ptr<galaxy::QueryGalaxy>> galaxies;
    std::vector<std::string> servers;
    for (int i = 0; i < config.planet_servers.size(); i++) {
        servers.push_back(config.planet_servers[i].domain);
    }
    std::vector<std::shared_ptr<replication::Planet>> planets;
    int cores = std::thread::hardware_concurrency();
    int i = 0;
    while (i <= cores/4) {
        auto xxx = std::make_shared<galaxy::QueryGalaxy>(config.galaxy_db_url);
        galaxies.push_back(xxx);
	std::rotate(servers.begin(), servers.begin()+1, servers.end());
	auto yyy = std::make_shared<replication::Planet>(*remote);
	yyy->connectServer(servers.front());
        planets.push_back(yyy);
        i++;
    }

    bool mainloop = true;
    while (mainloop) {
	boost::asio::thread_pool pool(cores);
	i = 0;
	while (i++ <= 4) {
	    auto task = boost::bind(threadChangeSet,
				    std::ref(remote),
				    std::ref(planets.front()),
				    std::ref(poly),
				    std::ref(galaxies.front()));
	    boost::asio::post(pool, std::ref(task));
	    std::rotate(galaxies.begin(), galaxies.begin()+1, galaxies.end());
	    std::rotate(planets.begin(), planets.begin()+1, planets.end());
	    remote->Increment();
	    remote->updateDomain(planets.front()->domain);
	    auto delay = std::chrono::milliseconds{100}; // FIXME: this should probably be tuned
	    std::this_thread::sleep_for(delay);
	}
	ptime timestamp;
	ptime now = boost::posix_time::microsec_clock::universal_time();
	if (lastosm != not_a_date_time) {
	    // std::cout << "TIMESTAMP1: " << to_simple_string(lastosm) << std::endl;
	    boost::posix_time::time_duration diff = now - lastosm;
	    // std::cout << "DELTA: " << (diff.hours()*60) + diff.minutes() << std::endl;
	    if ((diff.hours()*60) + diff.minutes() <= 50 || threads::osmdone) {
		//break;
	    }
	}
	// pool.join();
	// std::this_thread::sleep_for(std::chrono::milliseconds{100});
	// std::cout << "Restarting with: " << remote.filespec << std::endl;
    }
    // std::cout << "Caught up with: " << remote.filespec << std::endl;
    auto delay = std::chrono::minutes{1};
    while (mainloop) {
	std::rotate(galaxies.begin(), galaxies.begin()+1, galaxies.end());
	std::rotate(planets.begin(), planets.begin()+1, planets.end());
	// std::cout << "Caught up with: " << remote.filespec << std::endl;
	threadChangeSet(std::ref(remote),
			std::ref(planets.front()),
			std::ref(poly),
			std::ref(galaxies.front()));
	remote->Increment();
	std::this_thread::sleep_for(delay);
    }
}

// Starting with this URL, download the file, incrementing
void
startMonitorChanges(std::shared_ptr<replication::RemoteURL> &remote,
		    const multipolygon_t &poly,
		    const replicatorconfig::ReplicatorConfig &config)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("startMonitorChanges: took %w seconds\n");
#endif
    if (remote->frequency == frequency_t::changeset) {
        log_error(_("Could not start monitoring thread for OSM changes: URL %1% does not appear to be a valid URL for changes!"), remote->filespec);
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
        log_debug(_("Loaded plugin hotosm!"));
    } catch (std::exception &e) {
        log_debug(_("Couldn't load plugin! %1%"), e.what());
        exit(0);
    }
    auto validator = creator();

#ifdef MEMORY_DEBUG
    size_t sz, active1, active2;
#endif	// JEMALLOC memory debugging
    // Support multiple database connections
    std::vector<std::shared_ptr<galaxy::QueryGalaxy>> galaxies;
    std::vector<std::string> servers;
    for (int i = 0; i < config.planet_servers.size(); i++) {
        servers.push_back(config.planet_servers[i].domain);
    }
    std::vector<std::shared_ptr<replication::Planet>> planets;
    std::vector<std::shared_ptr<osm2pgsql::Osm2Pgsql>> rawosm;
    int cores = std::thread::hardware_concurrency();
    int i = 0;
    while (i <= cores/4) {
        auto xxx = std::make_shared<galaxy::QueryGalaxy>(config.galaxy_db_url);
        galaxies.push_back(xxx);
	std::rotate(servers.begin(), servers.begin()+1, servers.end());
        auto yyy = std::make_shared<replication::Planet>(*remote);
	yyy->connectServer(servers.front());
        planets.push_back(yyy);
        auto zzz = std::make_shared<osm2pgsql::Osm2Pgsql>(config.osm2pgsql_db_url);
        rawosm.push_back(zzz);
        i++;
    }

    // Process OSM changes
    bool mainloop = true;
    auto removals = std::make_shared<std::vector<long>>();
    while (mainloop) {
	boost::asio::thread_pool pool(cores);
	i = 0;
	while (i++ <= cores*2) {
	    auto task = boost::bind(threadOsmChange,
				    std::ref(remote),
				    std::ref(planets.front()),
				    std::ref(poly),
				    std::ref(galaxies.front()),
				    std::ref(rawosm.front()),
				    std::ref(validator),
				    std::ref(removals));
	    std::rotate(galaxies.begin(), galaxies.begin()+1, galaxies.end());
	    std::rotate(planets.begin(), planets.begin()+1, planets.end());
	    std::rotate(rawosm.begin(), rawosm.begin()+1, rawosm.end());
	    boost::asio::post(pool, task);
	    remote->Increment();
	    auto delay = std::chrono::milliseconds{100}; // FIXME: this should probably be tuned
	    std::this_thread::sleep_for(delay);
	    // remote.dump();
	}
	pool.join();
	ptime timestamp;
	ptime now = boost::posix_time::microsec_clock::universal_time();
	if (lastosc != not_a_date_time) {
	    boost::posix_time::time_duration delta = now - lastosc;
	    auto delay = std::chrono::minutes{1};
	    // std::cout << "TIMESTAMP2: " << to_simple_string(lastosc) << std::endl;
	    if ((delta.hours()*60) + delta.minutes() <= 50 || threads::oscdone) {
		break;
	    }
	}
	std::cout << "Removals: " << removals->size() << std::endl;
	for (auto it = std::begin(*removals); it != std::end(*removals); ++it) {
	    std::rotate(galaxies.begin(), galaxies.begin()+1, galaxies.end());
	    long osm_id = *it;
	    galaxies.front()->updateValidation(osm_id);
	}
	removals->clear();
    }
    // std::cout << "Caught up with: " << remote.filespec << std::endl;
    auto delay = std::chrono::minutes{1};
    while (mainloop) {
	std::rotate(galaxies.begin(), galaxies.begin()+1, galaxies.end());
	std::rotate(planets.begin(), planets.begin()+1, planets.end());
	std::rotate(rawosm.begin(), rawosm.begin()+1, rawosm.end());
	// std::cout << "Caught up with: " << remote.filespec << std::endl;
	threadOsmChange(std::ref(remote), std::ref(planets.front()),
			std::ref(poly), std::ref(galaxies.front()),
			std::ref(rawosm.front()),
			std::ref(validator),
			std::ref(removals));
	remote->Increment();
	std::this_thread::sleep_for(delay);
    }
}

// This thread get started for every osmChange file
std::shared_ptr<osmchange::OsmChangeFile>
threadOsmChange(std::shared_ptr<replication::RemoteURL> &remote,
		std::shared_ptr<replication::Planet> &planet,
		const multipolygon_t &poly,
		std::shared_ptr<galaxy::QueryGalaxy> &galaxy,
                std::shared_ptr<osm2pgsql::Osm2Pgsql> &o2pgsql,
		std::shared_ptr<Validate> &plugin,
		std::shared_ptr<std::vector<long>> removals)
{
    std::vector<std::string> result;
    auto osmchanges = std::make_shared<osmchange::OsmChangeFile>();
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("threadOsmChange: took %w seconds\n");
#endif
    // log_debug(_("Processing osmChange: %1%"), remote.filespec);
    auto data = planet->downloadFile(*remote.get());
    if (data->size() == 0) {
        log_error(_("osmChange file not found: %1% %2%"), remote->filespec, ".osc.gz");
	oscdone = true;
        return osmchanges;
    } else {
        try {
            std::istringstream changes_xml;
            // Scope to deallocate buffers
            {
                boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
                inbuf.push(boost::iostreams::gzip_decompressor());
                boost::iostreams::array_source arrs{reinterpret_cast<char const *>(data->data()), data->size()};
                inbuf.push(arrs);
                std::istream instream(&inbuf);
                changes_xml.str(std::string{std::istreambuf_iterator<char>(instream), {}});
            }

            try {
                osmchanges->readXML(changes_xml);

            } catch (std::exception &e) {
                log_error(_("Couldn't parse: %1%"), remote->filespec);
                std::cerr << e.what() << std::endl;
            }

        } catch (std::exception &e) {
            log_error(_("%1% is corrupted!"), remote->filespec);
            std::cerr << e.what() << std::endl;
        }
    }

#if 0
    for (auto cit = std::begin(osmchanges->changes); cit != std::end(osmchanges->changes); ++cit) {
        osmchange::OsmChange *change = cit->get();
        // change->dump();
        for (auto nit = std::begin(change->nodes); nit != std::end(change->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
        }
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
        }
    }
#endif
    osmchanges->areaFilter(poly);

    if (osmchanges->changes.size() > 0) {
	if (osmchanges->changes.back()->final_entry != not_a_date_time) {
	    threads::lastosc = osmchanges->changes.back()->final_entry;
	}
    }
    if (o2pgsql->isOpen()) {
        // o2pgsql.updateDatabase(osmchanges);
    }

    // These stats are for the entire file
    auto stats = osmchanges->collectStats(poly);
    for (auto it = std::begin(*stats); it != std::end(*stats); ++it) {
        if (it->second->added.size() == 0 && it->second->modified.size() == 0) {
            continue;
        }
        // it->second->dump();
        galaxy->applyChange(*it->second);
    }
#if 1
    // Delete existing entries in the validation table to remove features that have been fixed
    for (auto it = std::begin(osmchanges->changes); it != std::end(osmchanges->changes); ++it) {
        OsmChange *change = it->get();
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
	    osmobjects::OsmWay *way = wit->get();
	    if (way->action == osmobjects::remove) {
		const std::lock_guard<std::mutex> lock(remove_mutex);
		removals->push_back(way->id);
	    }
	    // galaxy->updateValidation(way->id);
	}
        for (auto nit = std::begin(change->nodes); nit != std::end(change->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
	    if (node->action == osmobjects::remove) {
		const std::lock_guard<std::mutex> lock(remove_mutex);
		removals->push_back(node->id);
	    }
	    // galaxy->updateValidation(node->id);
	}
    }
#endif
    auto nodeval = osmchanges->validateNodes(poly, plugin);
    // std::cerr << "SIZE " << nodeval->size() << std::endl;
    for (auto it = nodeval->begin(); it != nodeval->end(); ++it) {
        galaxy->applyChange(*it->get());
    }
    auto wayval = osmchanges->validateWays(poly, plugin);
    // std::cerr << "SIZE " << wayval->size() << std::endl;
    for (auto it = wayval->begin(); it != wayval->end(); ++it) {
        galaxy->applyChange(*it->get());
    }

    return osmchanges;
}

// This parses the changeset file into changesets
std::unique_ptr<changeset::ChangeSetFile>
threadChangeSet(std::shared_ptr<replication::RemoteURL> &remote,
		std::shared_ptr<replication::Planet> &planet,
		const multipolygon_t &poly,
		std::shared_ptr<galaxy::QueryGalaxy> galaxy)
{
#ifdef TIMING_DEBUG
    boost::timer::auto_cpu_timer timer("threadChangeSet: took %w seconds\n");
#endif
    auto changeset = std::make_unique<changeset::ChangeSetFile>();
    auto data = std::make_shared<std::vector<unsigned char>>();

    // remote->dump();
    log_debug(_("Processing ChangeSet: %1%"), remote->filespec);
    try {
	data = planet->downloadFile(*remote);
    } catch (const std::exception &ex) {
	log_debug(_("Download error for %1%: %2%"), remote->filespec, ex.what());
    }

    if (data->size() == 0) {
        log_error(_("ChangeSet file not found: %1%"), remote->filespec);
        changeset->download_error = true;
	threads::osmdone = true;
    } else {
	auto xml = planet->processData(remote->filespec, *data);
	std::istream& input(xml);
	changeset->readXML(input);
    }

   std::vector<long> changesetsToDelete = changeset->areaFilter(poly);

   for (auto it = std::begin(changesetsToDelete); it != std::end(changesetsToDelete); ++it) {
        galaxy->deleteChangeset(*it);
   }

   // std::scoped_lock write_lock{time_mutex};
   for (auto cit = std::begin(changeset->changes); cit != std::end(changeset->changes); ++cit) {
       //cit->get()->dump();
       changeset::ChangeSet *change = cit->get();
       galaxy->applyChange(*cit->get());
       if (change->closed_at != not_a_date_time) {
	   threads::lastosm = change->closed_at;
       } else if (change->created_at != not_a_date_time) {
	   threads::lastosm = change->closed_at;
       }
   }
   return changeset;
}

// This updates the calculated fields in the raw_changesets table, based on
// the data in the OSM stats database.
void
threadStatistics(const std::string &database, ptime &timestamp)
{
    //galaxy::QueryGalaxy ostats;
    replication::Replication repl;
}

void
threadTMUsersSync(std::atomic<bool> &tmUserSyncIsActive, const replicatorconfig::ReplicatorConfig &config)
{
    // There is a lot of DB URI manipulations in this program, if the URL
    // contains a plain hostname we need to add a database name too
    // FIXME: handle all DB URIs in a consistent and documented way
    auto galaxyDbUrlWithDbName{config.galaxy_db_url};
    if (config.galaxy_db_url.find('/') == std::string::npos) {
        galaxyDbUrlWithDbName.append("/galaxy");
    }
    auto galaxy{QueryGalaxy()};
    // Connection errors are fatal: exit!
    if (!galaxy.connect(galaxyDbUrlWithDbName)) {
        log_error("ERROR: couldn't connect to OSM Stats Underpass server: %1%!", galaxyDbUrlWithDbName);
        return;
    }

    TaskingManager taskingManager;
    // FIXME: handle all DB URIs in a consistent and documented way
    auto tmDbUrlWithDbName{config.taskingmanager_db_url};
    if (config.taskingmanager_db_url.find('/') == std::string::npos) {
        tmDbUrlWithDbName.append("/taskingmanager");
    }

    if (!taskingManager.connect(tmDbUrlWithDbName)) {
        log_error("ERROR: couldn't connect to Tasking Manager server: %1%!", tmDbUrlWithDbName);
        return;
    }

    const auto tmusersfrequency{config.taskingmanager_users_update_frequency};

    do {

        auto start{std::chrono::system_clock::now()};
        const auto users{taskingManager.getUsers()};
        // Sync and delete missing
        const auto results{galaxy.syncUsers(users, true)};
        auto end{std::chrono::system_clock::now()};
        auto elapsed{std::chrono::duration_cast<std::chrono::seconds>(end - start)};

        log_debug("Users sync TM->OS executed in %1% seconds.", elapsed.count());
        log_debug("Users created: %1%, updated: %2%, deleted: %3%", results.created, results.updated, results.deleted);

        if (tmusersfrequency > 0) {
            if (elapsed.count() < tmusersfrequency) {
                log_debug("Users sync TM->OS sleeping for %1% seconds...",
                          std::chrono::seconds(tmusersfrequency - elapsed.count()).count());
                std::this_thread::sleep_for(std::chrono::seconds(tmusersfrequency - elapsed.count()));
            }
        }

    } while (tmusersfrequency > 0 && tmUserSyncIsActive);
};

} // namespace threads

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
