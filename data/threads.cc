//
// Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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
#include <future>
#include <iostream>
#include <iterator>
#include <mutex>
#include <range/v3/all.hpp>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
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

#include "data/osmobjects.hh"
#include "data/threads.hh"
#include "data/underpass.hh"
#include "hottm.hh"
#include "log.hh"
#include "osmstats/changeset.hh"
#include "osmstats/osmchange.hh"
#include "osmstats/osmstats.hh"
#include "osmstats/replication.hh"
#include "timer.hh"
#include "validate/validate.hh"

std::mutex stream_mutex;

using namespace logger;
using namespace osmstats;
using namespace tmdb;

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace threads {

// Starting with this URL, download the file, incrementing
void
startMonitor(const replication::RemoteURL &inr, const multipolygon_t &poly,
             const std::string &dburl)
{
    underpass::Underpass under;
    under.connect(dburl);

    osmstats::QueryOSMStats ostats;
    ostats.connect(dburl);

    replication::RemoteURL remote = inr;
    auto planet = std::make_shared<replication::Planet>(remote);
    bool mainloop = true;

    std::string plugins;
    if (boost::filesystem::exists("validate/.libs")) {
        plugins = "validate/.libs";
    } else {
        plugins = PKGLIBDIR;
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
    auto validator = creator();
    while (mainloop) {
        // Look for the statefile first
#if 0
	std::shared_ptr<replication::StateFile> exists;
	exists = under.getState(remote.frequency, remote.subpath);
	if (exists) {
	    log_warning(_("Already stored in database: %1%"), remote.subpath);
	    break;
	} else {
	    log_info(_("Downloading StateFile: %1% %2%"), remote.subpath ".state.txt");
	    auto state = threadStateFile(planet->stream, remote.subpath + ".state.txt");
	    if (state->timestamp != boost::posix_time::not_a_date_time && (state->sequence != 0 && state->path.size() != 0)) {
		// state->dump();
		under.writeState(*state);
		break;
	    }
	}
#endif
        Timer timer;
        // remote.dump();
        if (remote.frequency == replication::changeset) {
            timer.startTimer();
            auto changefile = threadChangeSet(remote, poly, ostats);
            for (auto it = std::begin(changefile->changes);
                 it != std::end(changefile->changes); ++it) {
                ostats.applyChange(*it->get());
            }
            timer.endTimer("changeSet");
            ptime now = boost::posix_time::microsec_clock::local_time();
            boost::posix_time::time_duration delta;
            if (changefile->changes.size() > 0) {
                delta = now - changefile->changes.front()->created_at;
                // log_debug("DELTA: %1%", (delta.hours()*60) + delta.minutes());
                if ((delta.hours() * 60) + delta.minutes() <= 1) {
                    std::this_thread::sleep_for(std::chrono::minutes{1});
                }
            } else {
                log_debug(_("Processed ChangeSet: %1%"), remote.url);
            }
        } else {
            std::string file = remote.url + ".osc.gz";
            ptime now = boost::posix_time::microsec_clock::local_time();
            timer.startTimer();
            auto osmchange = threadOsmChange(remote, poly, ostats, validator);
            // FIXME: There is probably a better way to determine when to delay,
            // or when to just keep processing files when catching up.
            timer.endTimer("osmChange");
            boost::posix_time::time_duration delta;
            if (osmchange->changes.size() > 0 &&
                osmchange->changes.front()->nodes.size() > 0) {
                delta =
                    now - osmchange->changes.front()->nodes.back()->timestamp;
                // log_debug("DELTA: %1%", (delta.hours()*60) + delta.minutes());
            }
            if ((delta.hours() * 60) + delta.minutes() <= 1) {
                // log_debug("DELTA: %1%", (delta.hours()*60) + delta.minutes());
                // planet->disconnectServer();
                if (remote.frequency == replication::minutely) {
                    std::this_thread::sleep_for(std::chrono::minutes{1});
                } else if (remote.frequency == replication::hourly) {
                    std::this_thread::sleep_for(std::chrono::hours{1});
                } else if (remote.frequency == replication::daily) {
                    std::this_thread::sleep_for(std::chrono::hours{24});
                }
                // planet.reset(new replication::Planet);
            } else {
                log_debug(_("Processed OsmChange: %1%"), remote.url);
            }
        }
        remote.Increment();
        // remote.dump();
        // planet->endTimer("change file");
    }
}

void
startStateThreads(const std::string &base, const std::string &file)
{
    // std::map<std::string, std::thread> thread_pool;

    //return;                     // FIXME:

    // boost::system::error_code ec;
    // underpass::Underpass under;
    // under.connect();
    // auto planet = std::make_shared<replication::Planet>();
    // planet->connectServer();

    // #if 1
    //     for (auto it = std::begin(files); it != std::end(files); ++it) {
    //         // There are no state,txt file before this directory
    //         // https://planet.openstreetmap.org/replication/changesets/002/008

    //         //  state = [planet](const std::string &path)->bool {
    //         std::string path = base + it->substr(0, 3);
    //         std::shared_ptr<replication::StateFile> state = threadStateFile(planet->stream, path + ".state.txt");
    //         if (!state->path.empty()) {
    //             under.writeState(*state);
    //             state->dump();
    //         } else {
    //             std::cerr << "ERROR: No StateFile returned: " << path << std::endl;
    //             // planet.reset(new replication::Planet);
    //             // planet.reset(new replication::Planet());
    //             std::this_thread::sleep_for(std::chrono::seconds{1});
    //             state = threadStateFile(planet->stream, path + ".state.txt");
    //             if (!state->path.empty()) {
    //                 under.writeState(*state);
    //                 state->dump();
    //             }
    //         }
    //     }
    // #else
    //     // boost::asio::thread_pool pool(20);
    //     boost::asio::thread_pool pool(/* std::thread::hardware_concurrency() */ );

    //     // Note this uses ranges, which only got added in C++20, so
    //     // for now use the ranges-v3 library, which is the implementation.
    //     // The planet server drops the network connection after 111
    //     // GET requests, so break the 1000 strings into smaller chunks
    //     // 144, 160, 176, 192, 208, 224
    //     auto rng  = files | ranges::views::chunk(200);

    //     // underpass::Underpass under;
    //     // under.connect();
    //     //Timer timer;
    //     //timer.startTimer();
    //     for (auto cit = std::begin(rng); cit != std::end(rng); ++cit) {
    //         log_debug(_("Chunk data: %1%"), *cit));
    //         for (auto it = std::begin(*cit); it != std::end(*cit); ++it) {
    //             // There are no state,txt file before this directory
    //             // https://planet.openstreetmap.org/replication/changesets/002/008
    //             if (boost::filesystem::extension(*it) != ".txt") {
    //                 continue;
    //             }
    //             std::string subpath = base + it->substr(0, it->size() - 10);
    //             auto exists = under.getState(subpath);
    //             if (!exists->path.empty()) {
    //                 log_debug(_("Already stored: %1%"), subpath);
    //                 continue;
    //             }
    //             // Add a thread to the pool for this file
    //             if (!it->size() <= 1) {
    // #ifdef USE_MULTI_LOADER
    //                 boost::asio::post(pool, [subpath, state]{state(subpath);});
    // #else
    //                 auto state = threadStateFile(planet->stream, base + *it);
    //                 if (!state->path.empty()) {
    //                     // under.writeState(*state);
    //                     state->dump();
    //                     continue;
    //                 } else {
    //                     log_error(_("No StateFile returned"));
    //                 }
    // #endif
    //             }
    //         }
    //         //timer.endTimer("chunk ");
    //         // Don't hit the server too hard while testing, it's not polite
    //         // std::this_thread::sleep_for(std::chrono::seconds{1});
    //         planet->disconnectServer();
    //         planet.reset(new replication::Planet);
    //     }
    // #ifdef USE_MULTI_LOADER
    //     pool.join();
    // #endif
    //     planet->ioc.reset();
    // #endif
    //     // planet->stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    //     //timer.endTimer("directory ");
}

// This thread get started for every osmChange file
std::shared_ptr<osmchange::OsmChangeFile>
threadOsmChange(const replication::RemoteURL &remote,
                const multipolygon_t &poly, osmstats::QueryOSMStats &ostats,
                std::shared_ptr<Validate> &plugin)
{
    // osmstats::QueryOSMStats ostats;
    std::vector<std::string> result;
    auto osmchanges = std::make_shared<osmchange::OsmChangeFile>();

    auto data = std::make_shared<std::vector<unsigned char>>();
    // If the file is stored on disk, read it in instead of downloading
    if (boost::filesystem::exists(remote.filespec)) {
        log_debug(_("Reading osmChange: %1%"), remote.filespec);
        // Since we want to read in the entire file so it can be
        // decompressed, blow off C++ streaming and just load the
        // entire thing.
        int size = boost::filesystem::file_size(remote.filespec);
        data->reserve(size);
        data->resize(size);
        int fd = open(remote.filespec.c_str(), O_RDONLY);
        char *buf = new char[size];
        //memset(buf, 0, size);
        read(fd, buf, size);
        // FIXME: it would be nice to avoid this copy
        std::copy(buf, buf + size, data->begin());
        close(fd);
    } else {
        log_debug(_("Downloading osmChange: %1%"), remote.url);
        replication::Planet planet(remote);
        data = planet.downloadFile(remote.url);
    }
    if (data->size() == 0) {
        log_error(_("osmChange file not found: %1% %2%"), remote.url,
                  ".osc.gz");
        return osmchanges;
    } else {
#ifdef USE_CACHE
        if (!boost::filesystem::exists(remote.destdir)) {
            boost::filesystem::create_directories(remote.destdir);
        }
        std::ofstream myfile;
        myfile.open(remote.filespec, std::ios::binary);
        myfile.write(reinterpret_cast<char *>(data->data()), data->size() - 1);
        myfile.close();
#endif
        try {
            boost::iostreams::filtering_streambuf<boost::iostreams::input>
                inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            boost::iostreams::array_source arrs{
                reinterpret_cast<char const *>(data->data()), data->size()};
            inbuf.push(arrs);
            std::istream instream(&inbuf);
            try {
                osmchanges->readXML(instream);
            } catch (std::exception &e) {
                log_error(_("Couldn't parse: %1%"), remote.url);
                std::cerr << e.what() << std::endl;
                // return false;
            }
            // change.readXML(instream);
        } catch (std::exception &e) {
            log_error(_("%1% is corrupted!"), remote.url);
            std::cerr << e.what() << std::endl;
            // return false;
        }
    }

#if 0
    // Apply the changes to the database
    underpass::Underpass under;
    under.connect();
    replication::StateFile state;
    for (auto it = std::begin(osmchanges.changes); it != std::end(osmchanges.changes); ++it) {
        //state.created_at = it->created_at;
        //state.closed_at = it->closed_at;
        state.frequency = replication::changeset;
        state.path = file;
        under.writeState(state);
    }
#endif
    //boost::timer::cpu_timer timer;
    for (auto it = std::begin(osmchanges->changes);
         it != std::end(osmchanges->changes); ++it) {
        osmchange::OsmChange *change = it->get();
	// change->dump();
        for (auto it = std::begin(change->nodes); it != std::end(change->nodes);
             ++it) {
            osmobjects::OsmNode *node = it->get();
        }
        for (auto it = std::begin(change->ways); it != std::end(change->ways);
             ++it) {
            osmobjects::OsmWay *way = it->get();
        }
    }
    //timer.stop();
    // log_debug("Took %1% to process validation", timer.wall);
    Timer timer;
    //timer.startTimer();
    osmchanges->areaFilter(poly);
    //timer.endTimer("osmchanges::areaFilter");

    timer.startTimer();
    // These stats are for the entire file
    auto stats = osmchanges->collectStats(poly);
    for (auto it = std::begin(*stats); it != std::end(*stats); ++it) {
        if (it->second->added.size() == 0 && it->second->modified.size() == 0) {
            continue;
        }
        // it->second->dump();
        ostats.applyChange(*it->second);
    }
    timer.endTimer("collectStats");

    timer.startTimer();
    auto nodeval = osmchanges->validateNodes(poly, plugin);
    // std::cerr << "SIZE " << nodeval->size() << std::endl;
    for (auto it = nodeval->begin(); it != nodeval->end(); ++it) {
	ostats.applyChange(*it->get());
    }
    timer.endTimer("validate nodes");
    timer.startTimer();
    auto wayval = osmchanges->validateWays(poly, plugin);
    // std::cerr << "SIZE " << wayval->size() << std::endl;
    for (auto it = wayval->begin(); it != wayval->end(); ++it) {
	ostats.applyChange(*it->get());
    }
    timer.endTimer("validate ways");
    
    return osmchanges;
}

// This updates several fields in the changesets table, which are part of
// the changeset file, and don't need to be calculated.
//void threadChangeSet(const std::string &file, std::promise<bool> &&result)
std::shared_ptr<changeset::ChangeSetFile>
threadChangeSet(const replication::RemoteURL &remote,
                const multipolygon_t &poly, osmstats::QueryOSMStats &ostats)
{
    auto changeset = std::make_shared<changeset::ChangeSetFile>();
    auto state = std::make_shared<replication::StateFile>();
    auto data = std::make_shared<std::vector<unsigned char>>();
    // FIXME: this this be the datadir from the command line

    if (boost::filesystem::exists(remote.filespec)) {
        log_debug(_("Reading ChangeSet: %1%"), remote.filespec);
        // Since we want to read in the entire file so it can be
        // decompressed, blow off C++ streaming and just load the
        // entire thing.
        int size = boost::filesystem::file_size(remote.filespec);
        data->reserve(size);
        data->resize(size);
        int fd = open(remote.filespec.c_str(), O_RDONLY);
        char *buf = new char[size];
        //memset(buf, 0, size);
        read(fd, buf, size);
        // FIXME: it would be nice to avoid this copy
        std::copy(buf, buf + size, data->begin());
        close(fd);
    } else {
        log_debug(_("Downloading ChangeSet: %1%"), remote.url);
        replication::Planet planet(remote);
        data = planet.downloadFile(remote.url);
    }
    if (data->size() == 0) {
        log_error(_("ChangeSet file not found: %1%"), remote.url);
        //result.set_value(false);
        return changeset;
    } else {
        //result.set_value(true);
        // XML parsers expect every line to have a newline, including the end of file
#ifdef USE_CACHE
        if (!boost::filesystem::exists(remote.destdir)) {
            boost::filesystem::create_directories(remote.destdir);
        }
        std::ofstream myfile;
        myfile.open(remote.filespec, std::ios::binary);
        myfile.write(reinterpret_cast<char *>(data->data()), data->size() - 1);
        myfile.close();
#endif
        //data->push_back('\n');
        try {
            boost::iostreams::filtering_streambuf<boost::iostreams::input>
                inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            // data->push_back('\n');
            boost::iostreams::array_source arrs{
                reinterpret_cast<char const *>(data->data()), data->size()};
            inbuf.push(arrs);
            std::istream instream(&inbuf);
            try {
                changeset->readXML(instream);
            } catch (std::exception &e) {
                log_error(_("Couldn't parse: %1% %2%"), remote.url, e.what());
                // return false;
            }
            // change.readXML(instream);
        } catch (std::exception &e) {
            log_error(_("%1% is corrupted!"), remote.url);
            // return false;
        }
    }
    // Apply the changes to the database
    // for (auto it = std::begin(changeset.changes); it != std::end(changeset.changes); ++it) {
    //     ostats.applyChange(*it);
    // }
    // changeset.dump();

    //Timer timer;
    //timer.startTimer();
    changeset->areaFilter(poly);
    //timer.endTimer("changeset::areaFilter");

    // Apply the changes to the database
    for (auto it = std::begin(changeset->changes);
         it != std::end(changeset->changes); ++it) {
        ostats.applyChange(*it->get());
    }
    //     changeset.dump();

#if 0
    // Create a stubbed state file to update the underpass database with more
    // accurate timestamps, also used if there is no state.txt file.
    if (changeset->changes.size() > 0) {
        state->timestamp = changeset->changes.begin()->created_at;
        state->created_at = changeset->changes.begin()->created_at;
        state->closed_at = changeset->changes.end()->created_at;
    }
#endif
    return changeset;
}

// This updates the calculated fields in the raw_changesets table, based on
// the data in the OSM stats database.
void
threadStatistics(const std::string &database, ptime &timestamp)
{
    //osmstats::QueryOSMStats ostats;
    replication::Replication repl;
}

// Updates the states table in the Underpass database
std::shared_ptr<replication::StateFile>
threadStateFile(ssl::stream<tcp::socket> &stream, const std::string &file)
{
    std::string server;

    std::vector<std::string> result;
    boost::split(result, file, boost::is_any_of("/"));
    server = result[2];

    // This buffer is used for reading and must be persistant
    boost::beast::flat_buffer buffer;
    boost::beast::error_code ec;

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, file, 11};

    req.keep_alive();
    req.set(http::field::host, server);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    log_debug(_("(%1%)Downloading %2%"), std::this_thread::get_id(), file);

    // Stays locked till the function exits
    const std::lock_guard<std::mutex> lock(stream_mutex);

    // Send the HTTP request to the remote host
    // std::lock_guard<std::mutex> guard(stream_mutex);
    boost::beast::http::response_parser<http::string_body> parser;

    http::write(stream, req);
    boost::beast::http::read(stream, buffer, parser, ec);
    if (ec == http::error::partial_message) {
        log_network(_("ERROR: partial read: %1%"), ec.message());
        std::this_thread::yield();
        http::write(stream, req);
        boost::beast::http::read(stream, buffer, parser, ec);
        // Give the network a chance to recover
        // std::this_thread::sleep_for(std::chrono::seconds{1});
        //return std::make_shared<replication::StateFile>();
    }
    if (ec == http::error::end_of_stream) {
        log_error(_("end of stream read failed: %1%"), ec.message());
        // Give the network a chance to recover
        // stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        return std::make_shared<replication::StateFile>();
    } else if (ec) {
        log_network(_("ERROR: stream read failed: %1%"), ec.message());
        return std::make_shared<replication::StateFile>();
    }
    if (parser.get().result() == boost::beast::http::status::not_found) {
        // continue;
    }

    // File never downloaded, return empty
    if (parser.get().body().size() < 10) {
        log_error(_("failed to download: %1%"), file);
        return std::make_shared<replication::StateFile>();
    }

    //const std::lock_guard<std::mutex> unlock(stream_mutex);
    auto data = std::make_shared<std::vector<unsigned char>>();
    for (auto body = std::begin(parser.get().body());
         body != std::end(parser.get().body()); ++body) {
        data->push_back((unsigned char)*body);
    }
    if (data->size() == 0) {
        log_error(_("StateFile not found: %1%"), file);
        return std::make_shared<replication::StateFile>();
    } else {
        std::string tmp(reinterpret_cast<const char *>(data->data()));
        auto state = std::make_shared<replication::StateFile>(tmp, true);
        if (!file.empty()) {
            state->path = file.substr(0, file.size() - 10);
        }
        return state;
    }
}

void
threadTMUsersSync(std::atomic<bool> &tmUserSyncIsActive,
                  const std::string &tmDbUrl, const std::string &osmStatsDbUrl,
                  long tmusersfrequency)
{
    // There is a lot of DB URI manipulations in this program, if the URL
    // contains a plain hostname we need to add a database name too
    // FIXME: handle all DB URIs in a consistent and documented way
    auto osmStatsDbUrlWithDbName{osmStatsDbUrl};
    if (osmStatsDbUrl.find('/') == std::string::npos) {
        osmStatsDbUrlWithDbName.append("/osmstats");
    }
    auto osmStats{QueryOSMStats()};
    // Connection errors are fatal: exit!
    if (!osmStats.connect(osmStatsDbUrlWithDbName)) {
        log_error("ERROR: couldn't connect to OSM Stats Underpass server: %1%!",
                  osmStatsDbUrlWithDbName);
        return;
    }

    TaskingManager taskingManager;
    // FIXME: handle all DB URIs in a consistent and documented way
    auto tmDbUrlWithDbName{tmDbUrl};
    if (tmDbUrl.find('/') == std::string::npos) {
        tmDbUrlWithDbName.append("/taskingmanager");
    }

    if (!taskingManager.connect(tmDbUrlWithDbName)) {
        log_error("ERROR: couldn't connect to Tasking Manager server: %1%!",
                  tmDbUrlWithDbName);
        return;
    }

    do {

        auto start{std::chrono::system_clock::now()};
        const auto users{taskingManager.getUsers()};
        // Sync and delete missing
        const auto results{osmStats.syncUsers(users, true)};
        auto end{std::chrono::system_clock::now()};
        auto elapsed{
            std::chrono::duration_cast<std::chrono::seconds>(end - start)};

        log_debug("Users sync TM->OS executed in %1% seconds.",
                  elapsed.count());
        log_debug("Users created: %1%, updated: %2%, deleted: %3%",
                  results.created, results.updated, results.deleted);

        if (tmusersfrequency > 0) {
            if (elapsed.count() < tmusersfrequency) {
                log_debug(
                    "Users sync TM->OS sleeping for %1% seconds...",
                    std::chrono::seconds(tmusersfrequency - elapsed.count())
                        .count());
                std::this_thread::sleep_for(
                    std::chrono::seconds(tmusersfrequency - elapsed.count()));
            }
        }

    } while (tmusersfrequency > 0 && tmUserSyncIsActive);
};

} // namespace threads

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
