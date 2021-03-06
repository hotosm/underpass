//
// Copyright (c) 2020, 2021 Humanitarian OpenStreetMap Team
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

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
# include "unconfig.h"
#endif

#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <algorithm>
#include <utility>
// #include <pqxx/pqxx>
#include <fstream>
#include <cctype>
#include <cmath>
#include <sstream>
// #include <iterator>
#ifdef LIBXML
#  include <libxml++/libxml++.h>
#endif
#include <gumbo.h>

#include <osmium/io/any_input.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/io/any_output.hpp>

#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/local_time/local_time.hpp"
using namespace boost::posix_time;
using namespace boost::gregorian;
//#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/http/parser.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;   // from <boost/asio/ssl.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

#include "hotosm.hh"
#include "osmstats/replication.hh"
#include "osmstats/changeset.hh"
#include "data/underpass.hh"

/// Control access to the database connection
std::mutex db_mutex;

#include "log.hh"
using namespace logger;

namespace replication {

/// Parse the two state files for a replication file, from
/// disk or memory.

/// There are two types of state files with of course different
/// formats for the same basic data. The simplest one is for
/// a changeset file. which looks like this:
///
/// \-\-\-
/// last_run: 2020-10-08 22:30:01.737719000 +00:00
/// sequence: 4139992
///
/// The other format is used for minutely change files, and
/// has more fields. For now, only the timestamp and sequence
/// number is stored. It looks like this:
///
/// \#Fri Oct 09 10:03:04 UTC 2020
/// sequenceNumber=4230996
/// txnMaxQueried=3083073477
/// txnActiveList=
/// txnReadyList=
/// txnMax=3083073477
/// timestamp=2020-10-09T10\:03\:02Z
///
/// State files are used to know where to start downloading files
StateFile::StateFile(const std::string &file, bool memory)
{
    std::string line;
    std::ifstream state;
    std::stringstream ss;

    // It's a disk file, so read it in.
    if (!memory) {
        try {
            state.open(file, std::ifstream::in);
        }
        catch(std::exception& e) {
            log_debug(_("ERROR opening %1% %2%"), file, e.what());
            // return false;
        }
        // For a disk file, none of the state files appears to be larger than
        // 70 bytes, so read the whole thing into memory without
        // any iostream buffering.
        std::filesystem::path path = file;
        int size = std::filesystem::file_size(path);
        char *buffer = new char[size];
        state.read(buffer, size);
        ss << buffer;
        // FIXME: We do it this way to save lots of extra buffering
        // ss.rdbuf()->pubsetbuf(&buffer[0], size);
    } else {
        // It's in memory
        ss << file;
    }

    // Get the first line
    std::getline(ss, line, '\n');

    // This is a changeset state.txt file
    if (line == "---") {
        // Second line is the last_run timestamp
        std::getline(ss, line, '\n');
        // The timestamp is the second field
        std::size_t pos = line.find(" ");
        // 2020-10-08 22:30:01.737719000 +00:00
        timestamp = time_from_string(line.substr(pos+1));

        // Third and last line is the sequence number
        std::getline(ss, line, '\n');
        pos = line.find(" ");
        // The sequence is the second field
        sequence = std::stol(line.substr(pos+1));
        // This is a change file state.txt file
    } else {
        for (std::string line; std::getline(ss, line, '\n'); ) {
            std::size_t pos = line.find("=");

            // Not a key=value line. So we skip it.
            if (pos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            std::vector<std::string> skipKeys{"txnMaxQueried", "txnActiveList", "txnReadyList", "txnMax"};
            if (key == "sequenceNumber") {
                sequence = std::stol(value);
            } else if (std::count(skipKeys.begin(), skipKeys.end(), key)) {
            } else if (key == "timestamp") {
                pos = value.find('\\', pos + 1);
                std::string tstamp = value.substr(0, pos); // get the date and the hour
                tstamp += value.substr(pos + 1, 3); // get minutes
                pos = value.find('\\', pos + 1);
                tstamp += value.substr(pos + 1, 3); // get seconds
                timestamp = from_iso_extended_string(tstamp);
            } else {
                log_error(_("Invalid Key found: "), key);
                exit(EXIT_FAILURE);
            }
        }
    }

    state.close();
}

// Dump internal data to the terminal, used only for debugging
void
StateFile::dump(void)
{
    std::cerr << "Dumping state.txt file" << std::endl;
    std::cerr << "\tTimestamp: " << timestamp  << std::endl;
    std::cerr << "\tSequence: " << sequence  << std::endl;
    std::cerr << "\tPath: " << path  << std::endl;
    std::cerr << "\tcreated_at: " << created_at  << std::endl;
    std::cerr << "\tclosed_at: " << closed_at << std::endl;
}

// parse a replication file containing changesets
bool
Replication::readChanges(const std::string &file, osmstats::QueryOSMStats &ostats)
{
    changeset::ChangeSetFile changeset;
    std::ifstream stream;
    stream.open(file, std::ifstream::in);
    changeset.readXML(stream);

    // osmstats::QueryOSMStats ostats;
    // ostats.connect();
    // Apply the changes to the database
    for (auto it = std::begin(changeset.changes); it != std::end(changeset.changes); ++it) {
        ostats.applyChange(*it);
    }
   return true;
}

// Add this replication data to the changeset database
bool
Replication::mergeToDB()
{
    return false;
}

std::shared_ptr<std::vector<std::string>> &
Planet::getLinks(GumboNode* node, std::shared_ptr<std::vector<std::string>> &links)
{
    // if (node->type == GUMBO_NODE_TEXT) {
    //     std::string val = std::string(node->v.text.text);
    //     log_debug(_("FIXME: " << "GUMBO_NODE_TEXT " << val);
    // }
    
    if (node->type == GUMBO_NODE_ELEMENT) {
        GumboAttribute* href;
        if (node->v.element.tag == GUMBO_TAG_A &&
            (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
            // All the directories are a 3 digit number, and all the files
            // start with a 3 digit number
            if (href->value[0] >= 48 && href->value[0] <= 57) {
                //if (std::isalnum(href->value[0]) && std::isalnum(href->value[1])) {
                // log_debug(_("FIXME: %1%"), href->value);
                if (std::strlen(href->value) > 0) {
                    links->push_back(href->value);
                }
            }
        }
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            getLinks(static_cast<GumboNode*>(children->data[i]), links);
        }
    }

    return links;
}

// Download a file from planet
std::shared_ptr<std::vector<unsigned char>>
Planet::downloadFile(const std::string &url)
{
    boost::system::error_code ec;
    auto data = std::make_shared<std::vector<unsigned char>>();
    log_debug(_("Downloading: %1%"), url);
    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, url, version};

    req.keep_alive();
    req.set(http::field::host, remote.domain);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persistant
    boost::beast::flat_buffer buffer;

    // Receive the HTTP response
    http::response_parser<http::string_body> parser;
    // read_header(stream, buffer, parser);
    read(stream, buffer, parser);
    if (parser.get().result() == boost::beast::http::status::not_found) {
        log_error(_("Remote file not found: %1%"), url);
        exit(EXIT_FAILURE);
    }

    // Check the magic number of the file
    if (parser.get().body()[0] == 0x1f) {
        log_debug(_("It's gzipped"));
    } else {
        if (parser.get().body()[0] == '<') {
            log_debug(_("It's XML"));
        }
    }

    for (auto body = std::begin(parser.get().body()); body != std::end(parser.get().body()); ++body) {
        data->push_back((unsigned char)*body);
    }

    // Add the last newline back
    data->push_back('\n');
    return data;
}

Planet::~Planet(void)
{
    ioc.reset();                // reset the I/O conhtext
    // stream.shutdown();          // shutdown the socket used by the stream
}

Planet::Planet(void)
{
    // FIXME: for bulk downloads, we might want to strip across
    // all the mirrors. The support minutely diffs
    // pserver = "https://download.openstreetmap.fr";
    // pserver = "https://planet.maps.mail.ru";
    // pserver = "https://planet.openstreetmap.org";
    // connectServer();
};


// Dump internal data to the terminal, used only for debugging
void
Planet::dump(void)
{
    log_debug(_("Dumping Planet data"));
    for (auto it = std::begin(changeset); it != std::end(changeset); ++it) {
        std::cerr << "Changeset at: " << it->first << it->second << std::endl;
    }
    for (auto it = std::begin(minute); it != std::end(minute); ++it) {
        std::cerr << "Minutely at: " << it->first << ": " << it->second << std::endl;
    }
    for (auto it = std::begin(hour); it != std::end(hour); ++it) {
        std::cerr << "Minutely at: " << it->first << ": " << it->second << std::endl;
    }
    for (auto it = std::begin(day); it != std::end(day); ++it) {
        std::cerr << "Daily at: " << it->first << ": " << it->second << std::endl;
    }
}

bool
Planet::connectServer(const std::string &planet)
{
    if (!planet.empty()) {
        remote.domain = planet;
    }

    // Gracefully close the socket
    boost::system::error_code ec;
    ioc.reset();
    ctx.set_verify_mode(ssl::verify_none);
    // Strip off the https part
    std::string tmp;
    auto pos = planet.find(":");
    if (pos != std::string::npos) {
        tmp = planet.substr(pos+3);
    } else {
        tmp = planet;
    }
    auto const results = resolver.resolve(tmp, std::to_string(port));
    boost::asio::connect(stream.next_layer(), results.begin(), results.end(), ec);
    if (ec) {
        log_error(_("stream connect failed %1%"), ec.message());
        return false;
    }
    stream.handshake(ssl::stream_base::client, ec);
    if (ec) {
        log_error(_("stream handshake failed %1%"), ec.message());
        return false;
    }

    return true;
}

// Scan remote directory from planet
std::shared_ptr<std::vector<std::string>>
Planet::scanDirectory(const std::string &dir)
{
    log_debug(_("Scanning remote Directory: %1%"), dir);

    // The io_context is required for all I/O
    boost::asio::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::sslv23_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_none);

    // These objects perform our I/O
    tcp::resolver resolver{ioc};
    ssl::stream<tcp::socket> stream{ioc, ctx};

    // Look up the domain name
    auto const results = resolver.resolve(remote.domain, std::to_string(port));

    // Make the connection on the IP address we get from a lookup
    boost::asio::connect(stream.next_layer(), results.begin(), results.end());

    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);

    auto links =  std::make_shared<std::vector<std::string>>();

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, dir, version };
    req.set(http::field::host, remote.domain);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persistant
    boost::beast::flat_buffer buffer;

    // Receive the HTTP response
    boost::beast::error_code ec;
    http::response_parser<http::string_body> parser;
    // read_header(stream, buffer, parser);
    read(stream, buffer, parser);
    if (parser.get().result() == boost::beast::http::status::not_found) {
        return links;
    }
    GumboOutput* output = gumbo_parse(parser.get().body().c_str());
    getLinks(output->root, links);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    stream.shutdown();
    return links;
}

// Since the data files don't have a consistent time interval, this
// attempts to do a rough calculation of the probably data file,
// and downloads *.state.txt files till the right data file is found.
// Note that this can be slow as it has to download multiple files.
std::shared_ptr<replication::StateFile>
Planet::findData(frequency_t freq, const std::string &path)
{
    auto data = downloadFile(path + ".state.txt");
    if (data->size() == 0) {
        log_error(_("StateFile not found: %1%"), path);
        auto tmp = std::make_shared<replication::StateFile>();
        return tmp;
    } else {
        std::string tmp(reinterpret_cast<const char *>(data->data()));
        auto state = std::make_shared<replication::StateFile>(tmp, true);
        if (state->path[0] = 'h') {
            std::vector<std::string> result;
            boost::split(result, path, boost::is_any_of("/"));
            state->path = path;
        }
        state->dump();
        return state;
    }
}

std::shared_ptr<StateFile>
Planet::findData(frequency_t freq, long sequence)
{
    auto state = std::make_shared<replication::StateFile>();

    return state;
}

std::shared_ptr<StateFile>
Planet::findData(frequency_t freq, ptime tstamp)
{
    ptime now = boost::posix_time::microsec_clock::local_time();
    // start timestamps for the top level minute files
    std::vector<ptime> mstates = {
        time_from_string("2012-09-13 02:06"),
        time_from_string("2014-08-12 22:41"),
        time_from_string("2016-07-09 14:33"),
        time_from_string("2018-06-04 23:24"),
        time_from_string("2020-04-30 23:20"),
        now
    };

    // start timestamps for the top level changeset files
    // There are no state.txt files before 002/010/000, the
    // first sequence is 2009999, 2016-09-08 20:19:01
    std::vector<ptime> cstates = {
        time_from_string("2012-10-28 19:36"),
        time_from_string("2014-10-07 07:58"),
        time_from_string("2016-09-01 20:43"),
        time_from_string("2018-07-29 16:33"), // 2999999
        time_from_string("2020-06-28 23:26"), // 3999999
        now
    };
    // Changeset files don't have an associated state.txt till
    // this first one: .../replication/changesets/002/007/990.state.txt
    underpass::Underpass under;
    under.connect();
    auto state = std::make_shared<replication::StateFile>();
    // boost::local_time::local_time_period lp(states[0]);
    boost::posix_time::time_duration delta1, delta2;
    int minutes1 = 0;
    int minutes2 = 0;
    std::string major;
    std::string minor;
    std::string index;
#if 0
    auto data = downloadFile(newpath + ".state.txt");
    if (data->size() == 0) {
        log_error(_("StateFile not found: %1%"),newpath);
        return state;
    } else {
        std::string tmp(reinterpret_cast<const char *>(data->data()));
        auto state = std::make_shared<replication::StateFile>(tmp, true);
        state->path = newpath;
        under.writeState(*state);
        state->dump();
    }
#endif
    std::vector<ptime> times;
    if (freq == replication::minutely) {
        times = mstates;
    } else if (freq == replication::changeset) {
        times = cstates;
    } else if (freq == replication::hourly) {
        // times = hstates;
    }
    for (int i = times.size(); i >= 0; --i) {
        // std::cerr << to_simple_string(tstamp) << " : "
        //           << to_simple_string(times[i-2]) << " : "
        //           << to_simple_string(times[i-1]) << " : "
        //          );
        if (tstamp >= times[i-1] && tstamp <= times[i]) {
            delta1 = tstamp - times[i-1];
            delta2 = times[i] - tstamp;
            log_debug(_("Hours: %1% : %2%"), delta1.hours(), delta2.hours());
            minutes1 = (delta1.hours() * 60) + delta1.minutes();
            // This is the total time span in the major directory
            minutes2 = (delta2.hours() * 60) + delta2.minutes();
            log_error(_("Minutes: %1% : %2%"), minutes1, minutes2);
            boost::format fmt("%03d");
            fmt % (i-1);
            major += fmt.str() + "/";
            fmt % (minutes1/1000);
            minor += fmt.str() + "/";
            int total = 2;
            fmt % (total);
            index += fmt.str();
            state->path = major + minor + index;
            return state;
        }
    }

#if 0
    std::string path = pserver + datadir + state->path;
    auto data = downloadFile(path + ".state.txt");
    if (data->size() == 0) {
        log_error(_("StateFile not found: " << path);
        return state;
    } else {
        std::string tmp(reinterpret_cast<const char *>(data->data()));
        auto state = std::make_shared<replication::StateFile>(tmp, true);
            state->path = path;
            under.writeState(*state);
            state->dump();
    }
#endif
#if 0
        state.timestamp = tstamp;
        under.writeState(state);
        state.dump();
        return state.path;
    }
#endif

    return state;

#if 0
    auto last = std::make_shared<replication::StateFile>();
    last = under.getLastState(freq);
    if (!last->path.empty()) {
        return state.path;
    }
    // last->dump();
    bool loop = true;
    delta = tstamp - last->timestamp;
    std::vector<std::string> result;
    boost::split(result, last->path, boost::is_any_of("/"));
    int major = std::stoi(result[0]);
    int minor = std::stoi(result[1]);
    int index = std::stoi(result[2]);
    int minutes = (delta.hours() * 60) + delta.minutes();
    int quotient =  minutes / 1000;
    int remainder = minutes % 1000;
    boost::format fmt1("%03d");
    fmt1 % (major);
    std::string newpath = last->path + "/" + fmt1.str();
    boost::format fmt2("%03d");
    int next = (index + remainder)/1000;
    fmt2 % (minor + next + quotient);
    newpath += "/" + fmt2.str();
    boost::format fmt3("%03d");
    next = (index + remainder)%1000;
    fmt3 % (next);
    newpath += "/" + fmt3.str();
    log_debug(_("FOUND: " << newpath);
    while (loop) {
        auto data = downloadFile(newpath + ".state.txt");
        if (data->size() == 0) {
            log_error(_("StateFile not found: " << newpath);
            return std::string();
        } else {
            std::string tmp(reinterpret_cast<const char *>(data->data()));
            auto state = std::make_shared<replication::StateFile>(tmp, true);
            state->path = newpath;
            under.writeState(*state);
            state->dump();
            // see if it's within range
            delta = state->timestamp - tstamp;
            // std::cerr << "DELTA: " << delta.seconds());
            if (delta.seconds() > 0) {
                log_debug(_("StateFile in range: %1%"), newpath);
                return state->path;
            } else {
                    log_error(_("StateFile not in range: %1%"), newpath);
                fmt3 % (++next);
                newpath = newpath.substr(0, newpath.rfind('/')) + "/" + fmt3.str();
                continue;
            }
        }
    }
#endif
}

RemoteURL::RemoteURL(void)
    : major(0), minor(0), index(0), frequency(minutely)
{
}

void
RemoteURL::parse(const std::string &rurl)
{
    if (rurl.empty()) {
        log_error(_("URL is empty!"));
        return;
    }
    std::map<std::string, replication::frequency_t> frequency_tags;
    frequency_tags["minute"] = replication::minutely;
    frequency_tags["hour"] = replication::hourly;
    frequency_tags["day"] = replication::daily;
    frequency_tags["changesets"] = replication::changeset;

    std::vector<std::string> parts;
    boost::split(parts, rurl, boost::is_any_of("/"));
    domain = parts[2];
    datadir = parts[3];
    frequency = frequency_tags[parts[4]];
    subpath = parts[5] + "/" + parts[6] + "/" + parts[7];
    major = std::stoi(parts[5]);
    minor = std::stoi(parts[6]);
    index = std::stoi(parts[7]);
    if (frequency == replication::changeset) {
        filespec = rurl.substr(rurl.find(datadir)) + ".osm.gz";
        url = rurl + ".osm.gz";
    } else {
        filespec = rurl.substr(rurl.find(datadir)) + ".osc.gz";
        url = rurl + ".osc.gz";
    }
    destdir = datadir + "/" + parts[4] + "/" +  parts[5] + "/" + parts[6];
}

void RemoteURL::Increment(void)
{
    boost::format majorfmt("%03d");
    boost::format minorfmt("%03d");
    boost::format indexfmt("%03d");
    std::string newpath;
    if (minor == 999) {
        major++;
        minor = 0;
        index = 0;
    }
    if (index == 999) {
        minor++;
        index = 0;
    } else {
        index++;
    }

    majorfmt % (major);
    minorfmt % (minor);
    indexfmt % (index);

    newpath = majorfmt.str() + "/" + minorfmt.str() + "/" + indexfmt.str();
    // log_debug(_("NEWPATH: " << newpath);
    boost::algorithm::replace_all(url, subpath, newpath);
    boost::algorithm::replace_all(destdir, subpath, newpath);
    boost::algorithm::replace_all(filespec, subpath, newpath);
    boost::algorithm::replace_all(url, subpath, newpath);
    subpath = newpath;
}

RemoteURL &
RemoteURL::operator=(const RemoteURL &inr)
{
    domain = inr.domain;
    datadir = inr.datadir;
    subpath = inr.subpath;
    frequency = inr.frequency;
    major = inr.major;
    minor = inr.minor;
    index = inr.index;
    url = inr.url;
    filespec = inr.filespec;
    destdir = inr.destdir;

    return *this;
}

RemoteURL::RemoteURL(const RemoteURL &inr)
{
    domain = inr.domain;
    datadir = inr.datadir;
    subpath = inr.subpath;
    frequency = inr.frequency;
    major = inr.major;
    minor = inr.minor;
    index = inr.index;
    url = inr.url;
    filespec = inr.filespec;
    destdir = inr.destdir;
}

void
RemoteURL::dump(void)
{
    std::cerr << "URL: " << url << std::endl;
    std::cerr << "\tDomain: " << domain << std::endl;
    std::cerr << "\tDatadir: " << datadir << std::endl;
    std::cerr << "\tSubpath: " << subpath << std::endl;
    std::cerr << "\tURL: " << url << std::endl;
    std::map<frequency_t, std::string> freqs;
    freqs[replication::minutely] = "minute";
    freqs[replication::hourly] = "hour";
    freqs[replication::daily] = "day";
    freqs[replication::changeset] = "changesets";
    std::cerr << "\tFrequency: " << (int)frequency << std::endl;
    std::cerr << "\tMajor: " << major << std::endl;
    std::cerr << "\tMinor: " <<  minor<< std::endl;
    std::cerr << "\tIndex: " << index << std::endl;
    std::cerr << "\tFilespec: " << filespec << std::endl;
    std::cerr << "\tDestdir: " << destdir << std::endl;
}

} // EOF replication namespace

