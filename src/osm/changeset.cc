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

/// \file changeset.cc
/// \brief The file is used for processing changeset files
///
/// The Changeset file contains the raw data on just the change,
/// and doesn't contain any data of the change except the comment
/// and hashtags used when the change was uploaded to OSM.

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
#include "unconfig.h"
#endif

#include <array>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <pqxx/pqxx>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <zlib.h>
#ifdef LIBXML
#include <libxml++/libxml++.h>
#endif

#include <glibmm/convert.h>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/handler.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/visitor.hpp>

#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>
#include <boost/locale.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;
#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/tokenizer.hpp>
#include <boost/tokenizer.hpp>
#include <boost/timer/timer.hpp>
#include <regex>

#include "osm/changeset.hh"
#include "stats/querystats.hh"

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include "utils/log.hh"
using namespace logger;

/// \namespace changeset
namespace changeset {

bool
ChangeSetFile::readChanges(const std::vector<unsigned char> &buffer)
{

    // parse_memory((const Glib::ustring &)buffer);
}

// Read a changeset file from disk or memory into internal storage
bool
ChangeSetFile::readChanges(const std::string &file)
{
    std::ifstream change;
    int size = 0;
    //    store = false;

    unsigned char *buffer;
    log_debug("Reading changeset file %1% ", file);
    std::string suffix = boost::filesystem::extension(file);
    // It's a gzipped file, common for files downloaded from planet
    std::ifstream ifile(file, std::ios_base::in | std::ios_base::binary);
    if (suffix == ".gz") { // it's a compressed file
                           //    if (file[0] == 0x1f) {
        change.open(file, std::ifstream::in | std::ifstream::binary);
        try {
            boost::iostreams::filtering_streambuf<boost::iostreams::input>inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            inbuf.push(ifile);
            std::istream instream(&inbuf);
            // log_debug(instream.rdbuf());
            readXML(instream);
        } catch (std::exception &e) {
            log_error("opening %1% %2%", file, e.what());
            // return false;
        }
    } else { // it's a text file
        change.open(file, std::ifstream::in);
        readXML(change);
    }

    change.close();
}

void
ChangeSetFile::areaFilter(const multipolygon_t &poly)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("ChangeSetFile::areaFilter: took %w seconds\n");
#endif
    // log_debug("Pre filtering changeset size is %1%", changes.size());
    for (auto it = std::begin(changes); it != std::end(changes); it++) {
        ChangeSet *change = it->get();
        if (poly.empty()) {
            // log_debug("Accepting changeset %1% as in priority area because area information is missing",
            // change->id);
            change->priority = true;
            continue;
        }
        boost::geometry::append(change->bbox, point_t(change->max_lon, change->max_lat));
        boost::geometry::append(change->bbox, point_t(change->max_lon, change->min_lat));
        boost::geometry::append(change->bbox, point_t(change->min_lon, change->min_lat));
        boost::geometry::append(change->bbox, point_t(change->min_lon, change->max_lat));
        boost::geometry::append(change->bbox, point_t(change->max_lon, change->max_lat));
        // point_t pt;
        // boost::geometry::centroid(change->bbox, pt);
        if (!boost::geometry::intersects(change->bbox, poly)) {
            // log_debug("Validating changeset %1% is not in a priority area", change->id);

            change->priority = false;

            changes.erase(it--);
        } else {
            // log_debug("Validating changeset %1% is in a priority area", change->id);
            change->priority = true;
        }
    }
    // log_debug("Post filtering changeset size is %1%",
    // changeset->changes.size());
}

void
ChangeSet::dump(void)
{
    std::cerr << "-------------------------" << std::endl;
    std::cerr << "Change ID: " << id << std::endl;
    std::cerr << "Created At:  " << to_simple_string(created_at) << std::endl;
    std::cerr << "Closed At:   " << to_simple_string(closed_at) << std::endl;
    if (open) {
        std::cerr << "Open change: true" << std::endl;
    } else {
        std::cerr << "Open change: false" << std::endl;
    }
    std::cerr << "User:        " << user << std::endl;
    std::cerr << "User ID:     " << uid << std::endl;
    std::cerr << "Min Lat:     " << min_lat << std::endl;
    std::cerr << "Min Lon:     " << min_lon << std::endl;
    std::cerr << "Max Lat:     " << max_lat << std::endl;
    std::cerr << "Max Lon:     " << max_lon << std::endl;
    std::cerr << "Changes:     " << num_changes << std::endl;
    if (!source.empty()) {
        std::cerr << "Source:      " << source << std::endl;
    }
    // std::cerr << "Comments:    " << comments_count << std::endl;
    for (auto it = std::begin(hashtags); it != std::end(hashtags); ++it) {
        std::cerr << "Hashtags:    " << *it << std::endl;
    }
    if (!comment.empty()) {
        std::cerr << "Comments:    " << comment << std::endl;
    }
    std::cerr << "Editor:      " << editor << std::endl;
}

#ifdef LIBXML
ChangeSet::ChangeSet(const std::deque<xmlpp::SaxParser::Attribute> attributes)
{
    // On non-english numeric locales using decimal separator different than '.'
    // this is necessary to parse double strings with std::stod correctly without
    // loosing precision
    std::setlocale(LC_NUMERIC, "C");

    for (const auto &attr_pair: attributes) {
        try {
            if (attr_pair.name == "id") {
                id = std::stol(attr_pair.value); // change id
            } else if (attr_pair.name == "created_at") {
                created_at =
                    from_iso_extended_string(attr_pair.value.substr(0, 19));
            } else if (attr_pair.name == "closed_at") {
                closed_at =
                    from_iso_extended_string(attr_pair.value.substr(0, 19));
            } else if (attr_pair.name == "open") {
                if (attr_pair.value == "true") {
                    open = true;
                } else {
                    open = false;
                }
            } else if (attr_pair.name == "user") {
                user = attr_pair.value;
            } else if (attr_pair.name == "source") {
                source = attr_pair.value;
            } else if (attr_pair.name == "uid") {
                uid = std::stol(attr_pair.value);
            } else if (attr_pair.name == "lat") {
                min_lat = std::stod(attr_pair.value);
                max_lat = std::stod(attr_pair.value);
            } else if (attr_pair.name == "min_lat") {
                min_lat = std::stod(attr_pair.value);
            } else if (attr_pair.name == "max_lat") {
                max_lat = std::stod(attr_pair.value);
            } else if (attr_pair.name == "lon") {
                min_lon = std::stod(attr_pair.value);
                max_lon = std::stod(attr_pair.value);
            } else if (attr_pair.name == "min_lon") {
                min_lon = std::stod(attr_pair.value);
            } else if (attr_pair.name == "max_lon") {
                max_lon = std::stod(attr_pair.value);
            } else if (attr_pair.name == "num_changes") {
                num_changes = std::stoi(attr_pair.value);
            } else if (attr_pair.name == "changes_count") {
                num_changes = std::stoi(attr_pair.value);
            } else if (attr_pair.name == "comments_count") {
            }
        } catch (const Glib::ConvertError &ex) {
            log_error("ChangeSet::ChangeSet(): Exception caught while "
                        "converting values for std::cout: ",
                      ex.what());
        }
    }
}
#endif // EOF LIBXML

void
ChangeSetFile::dump(void)
{
    std::cerr << "There are " << changes.size() << " changes" << std::endl;
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        // it->dump();
    }
}

// Read an istream of the data and parse the XML
//
bool
ChangeSetFile::readXML(std::istream &xml)
{
#ifdef LIBXML
    // libxml calls on_element_start for each node, using a SAX parser,
    // and works well for large files.
    try {
        set_substitute_entities(true);
        parse_stream(xml);
    } catch (const xmlpp::exception &ex) {
        // FIXME: files downloaded seem to be missing a trailing \n,
        // so produce an error, but we can ignore this as the file is
        // processed correctly.
        log_error("libxml++ exception: %1%", ex.what());
        return false;
    }
#else
    // Boost::parser_tree with RapidXML is faster, but builds a DOM tree
    // so loads the entire file into memory. Most replication files for
    // hourly or minutely changes are small, so this is better for that
    // case.
    boost::property_tree::ptree pt;
    try {
        boost::property_tree::read_xml(xml, pt);
    } catch (exception& boost::property_tree::xml_parser::xml_parser_error) {
        log_error("Error parsing XML");
        return false;
    }

    if (pt.empty()) {
        log_error("ERROR: XML data is empty!");
        return false;
    }

    for (auto value: pt.get_child("osm")) {
        if (value.first == "changeset") {
            changeset::ChangeSet change;
            // Process the tags. These don't exist for every element
            for (auto tag: value.second) {
                if (tag.first == "tag") {
                    std::string key = tag.second.get("<xmlattr>.k", "");
                    std::string val = tag.second.get("<xmlattr>.v", "");
                    change.tags[key] = val;
                }
            }
            // Process the attributes, which do exist in every element
            change.id = value.second.get("<xmlattr>.id", 0);
            change.created_at = value.second.get("<xmlattr>.created_at",
                                 boost::posix_time::second_clock::universal_time());
            change.closed_at = value.second.get("<xmlattr>.closed_at",
                                 boost::posix_time::second_clock::universal_time());
            change.open = value.second.get("<xmlattr>.open", false);
            change.user = value.second.get("<xmlattr>.user", "");
            change.uid = value.second.get("<xmlattr>.uid", 0);
            change.min_lat = value.second.get("<xmlattr>.min_lat", 0.0);
            change.min_lon = value.second.get("<xmlattr>.min_lon", 0.0);
            change.max_lat = value.second.get("<xmlattr>.max_lat", 0.0);
            change.max_lon = value.second.get("<xmlattr>.max_lon", 0.0);
            change.num_changes = value.second.get("<xmlattr>.num_changes", 0);
            change.comments_count = value.second.get("<xmlattr>.comments_count", 0);
            changes.push_back(change);
        }
    }
#endif
    return true;
}

#ifdef LIBXML
void
ChangeSetFile::on_end_element(const Glib::ustring &name)
{
    // log_debug("Element \'%1%\' ending", name);
}

void
ChangeSetFile::on_start_element(const Glib::ustring &name,
                                const AttributeList &attributes)
{
    // log_debug("Element %1%", name);
    if (name == "changeset") {
        auto change = std::make_shared<changeset::ChangeSet>(attributes);
        changes.push_back(change);
        if (change->closed_at != not_a_date_time && (last_closed_at == not_a_date_time || change->closed_at > last_closed_at)) {
            last_closed_at = change->closed_at;
        }
        // changes.back().dump();
    } else if (name == "tag") {
        // We ignore most of the attributes, as they're not used for OSM stats.
        // Processing a tag requires multiple passes through the loop. The
        // two tags to look for are 'k' (keyword) and 'v' (value). So when
        // we see a key we want, we have to wait for the next iteration of
        // the loop to get the value.
        bool hashit = false;
        bool comhit = false;
        bool cbyhit = false;
        double min_lat = 0.0;
        double min_lon = 0.0;
        double max_lat = 0.0;
        double max_lon = 0.0;

        for (const auto &attr_pair: attributes) {
            // std::wcout << "\tPAIR: " << attr_pair.name << " = " << std::endl;
            // attr_pair.value << std::endl;
            if (attr_pair.name == "k" && attr_pair.value == "max_lat") {
                max_lat = std::stod(attr_pair.value);
            } else if (attr_pair.name == "k" && attr_pair.value == "hashtags") {
                hashit = true;
            } else if (attr_pair.name == "k" && attr_pair.value == "comment") {
                comhit = true;
            } else if (attr_pair.name == "k" && attr_pair.value == "created_by") {
                cbyhit = true;
            }

            if (changes.size() == 0) {
                std::cerr << "No changes!" << std::endl;
                auto change = std::make_shared<ChangeSet>();
                changes.push_back(change);
            }

            if (hashit && attr_pair.name == "v") {
                hashit = false;
                std::size_t pos = attr_pair.value.find('#', 0);
                if (pos != std::string::npos) {
                    // Don't allow really short hashtags, they're usually a typo
                    if (attr_pair.value.length() < 3) {
                        continue;
                    }
                    char *token = std::strtok((char *)attr_pair.value.c_str(), "#;");
                    while (token != NULL) {
                        if (token) {
                            changes.back()->addHashtags(token);
                        }
                        token = std::strtok(NULL, "#;");
                    }
                } else {
                    changes.back()->addHashtags(attr_pair.value);
                }
            }
            // Hashtags start with an # of course. The hashtag tag wasn't
            // added till later, so many older hashtags are in the comment
            // field instead.
            if (comhit && attr_pair.name == "v") {
                comhit = false;
                changes.back()->addComment(attr_pair.value);
                std::string value = attr_pair.value;
                // Treat most punctuation (except -, _, +, &) as hashtag delimiters
                // https://github.com/openstreetmap/iD/blob/develop/modules/ui/commit.js
                std::regex subjectRx("(#[^\u2000-\u206F\u2E00-\u2E7F\\s\\'!\"#$%()*,.\\/:;<=>?@\\[\\]^`{|}~]+)", std::regex_constants::icase);
                std::regex_iterator<std::string::iterator> it (value.begin(), value.end(), subjectRx);
                std::regex_iterator<std::string::iterator> end;
                while (it != end) {
                    std::string hashtag = it->str(1).erase(0,1);
                    if (hashtag.size() > 2) {
                        changes.back()->addHashtags(hashtag);
                    }
                    ++it;
                }
            }
            if (cbyhit && attr_pair.name == "v") {
                cbyhit = false;
                changes.back()->addEditor(attr_pair.value);
            }
        }
    }
}
#endif // EOF LIBXML

} // namespace changeset
