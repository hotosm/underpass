//
// Copyright (c) 2020, Humanitarian OpenStreetMap Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#ifndef __THREADS_HH__
#define __THREADS_HH__

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
# include "hotconfig.h"
#endif

#include <string>
#include <vector>
#include <iostream>
#include <pqxx/pqxx>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;
using namespace boost::gregorian;

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/http/parser.hpp>

namespace beast = boost::beast;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
namespace http = beast::http;
using tcp = net::ip::tcp;

#include "osmstats/replication.hh"

namespace replication {
class StateFile;
};


/// \file threads.hh
/// \brief Threads for monitoring the OSM planet server for replication files.
///
/// These are the threads used to download and apply the replication files
/// to a database. The monitor the OSM planet server for updated replication
/// files.

/// \namespace threads
namespace threads {
class ThreadManager
{
public:
    ThreadManager(void);

    /// Get the number of processor cores in this machine
    int numThreads(void) { return std::thread::hardware_concurrency(); };

    //void startStateThreads(const std::string &base, std::vector<std::string> &files);

//replication::Planet planet;
};

extern void startStateThreads(const std::string &base, std::vector<std::string> &files);
extern void startMonitor(const std::string &url);

/// Updates the states table in the Underpass database
extern std::shared_ptr<replication::StateFile> threadStateFile(ssl::stream<tcp::socket> &stream,
                            const std::string &file);

/// Updates the raw_hashtags, raw_users, and raw_changesets_countries tables
/// from a changeset file
extern void threadOsmChange(const std::string &database, ptime &timestamp);

/// This updates several fields in the raw_changesets table, which are part of
/// the changeset file, and don't need to be calculated.
extern void threadChangeSet(const std::string &database, ptime &timestamp);

/// This updates the calculated fields in the raw_changesets table, based on
/// the data in the OSM stats database. These should be calculated by the
/// OsmChange thread, but as changesets and osmchange files are on different
/// timestamps, this looks for anything that got missed.
extern void threadStatistics(const std::string &database, ptime &timestamp);

/// This updates an OSM database, which can be used for extracts and other
/// validation.
extern void threadOSM(const std::string &database, ptime &timestamp);

} // EOF threads namespace

#endif  // EOF __THREADS_HH__
