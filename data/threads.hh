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

#ifndef __THREADS_HH__
#define __THREADS_HH__

/// \file threads.hh
/// \brief Threads for monitoring/synchronizing various data sources:
///         - OSM planet server for replication files.
///         - TM users table DB
///
/// These are the threads used to download and apply the replication files
/// to a database. The thread monitors the OSM planet server for updated
/// replication files.
/// Another thread imports users data from TM database.

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
#include "unconfig.h"
#endif

#include <future>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <ogr_geometry.h>
#include <thread>

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/version.hpp>
#include <boost/thread/future.hpp>
#include <boost/circular_buffer.hpp>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace http = beast::http;
using tcp = net::ip::tcp;

#include "data/osm2pgsql.hh"
#include "galaxy/galaxy.hh"
#include "galaxy/replication.hh"
#include "replicatorconfig.hh"
#include "validate/validate.hh"
#include <ogr_geometry.h>

namespace replication {
class StateFile;
class RemoteURL;
}; // namespace replication

typedef std::shared_ptr<Validate>(plugin_t)();

/// \namespace threads
namespace threads {

// std::pair<ptime, std::string>
// getClosestProcessedChange(std::shared_ptr<std::vector<std::pair<ptime, std::string>>> processed, ptime now);

/// This monitors the planet server for new changesets files.
/// It does a bulk download to catch up the database, then checks for the
/// minutely change files and processes them.
extern void
startMonitorChangesets(std::shared_ptr<replication::RemoteURL> &remote,
		       const multipolygon_t &poly,
                       const replicatorconfig::ReplicatorConfig &config);

/// This monitors the planet server for new OSM changes files.
/// It does a bulk download to catch up the database, then checks for the
/// minutely change files and processes them.
extern void
startMonitorChanges(std::shared_ptr<replication::RemoteURL> &remote,
		    const multipolygon_t &poly,
                    const replicatorconfig::ReplicatorConfig &config);

/// Updates the raw_hashtags, raw_users, and raw_changesets_countries tables
/// from a changeset file
void threadOsmChange(std::shared_ptr<replication::RemoteURL> &remote,
		std::shared_ptr<replication::Planet> &planet,
		const multipolygon_t &poly,
		std::shared_ptr<galaxy::QueryGalaxy> &galaxy,
		std::shared_ptr<osm2pgsql::Osm2Pgsql> &rawosm,
		std::shared_ptr<Validate> &plugin,
		std::shared_ptr<std::vector<long>> removals,
		std::shared_ptr<std::vector<std::pair<std::string, ptime>>> tasks);

/// This updates several fields in the raw_changesets table, which are part of
/// the changeset file, and don't need to be calculated.
// extern bool threadChangeSet(const std::string &file);
void
threadChangeSet(std::shared_ptr<replication::RemoteURL> &remote,
		std::shared_ptr<replication::Planet> &planet,
                const multipolygon_t &poly,
		std::shared_ptr<galaxy::QueryGalaxy> galaxy,
		std::shared_ptr<std::vector<std::pair<std::string, ptime>>> tasks);

// extern bool threadChangeSet(const std::string &file, std::promise<bool> && result);

/// This updates the calculated fields in the raw_changesets table, based on
/// the data in the OSM stats database. These should be calculated by the
/// OsmChange thread, but as changesets and osmchange files are on different
/// timestamps, this looks for anything that got missed.
extern void
threadStatistics(const std::string &database, ptime &timestamp);

/// This updates an OSM database, which can be used for extracts and other
/// validation.
extern void
threadOSM(const std::string &database, ptime &timestamp);

/// \brief threadTMUsersSync starts a cron-like monitor task that synchronizes
///        users from TM database.
/// \param tmUserSyncIsActive reference to an atomic flag that will exit the loop when FALSE,
///        this allows to abort the function fro client code.
/// \param tmDbUrl the TM database connection string in the form HOST or
///           USER:PASSSWORD@HOST/DATABASENAME
/// \param galaxyDbUrlthe Galaxy database connection string in the form HOST or
///          USER:PASSSWORD@HOST/DATABASENAME
/// \param tmusersfrequency TM user sync frequency in seconds (-1 -> disabled, 0 ->
///          single shot, > 0 -> interval)
extern void
threadTMUsersSync(std::atomic<bool> &tmUserSyncIsActive, const replicatorconfig::ReplicatorConfig &config);

} // namespace threads

#endif // EOF __THREADS_HH__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
