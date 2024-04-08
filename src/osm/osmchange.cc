//
// Copyright (c) 2020, 2021, 2023, 2024 Humanitarian OpenStreetMap Team
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

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <algorithm>
#include <pqxx/pqxx>
#include <list>
#include <locale>

#ifdef LIBXML
#include <libxml++/libxml++.h>
#endif
#include <glibmm/convert.h>

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;
using namespace boost::gregorian;
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <ogrsf_frmts.h>
#include <boost/units/systems/si/length.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/timer/timer.hpp>
// #include <boost/multi_index_container.hpp>
// #include <boost/multi_index/member.hpp>
// #include <boost/multi_index/ordered_index.hpp>
// using boost::multi_index_container;
// using namespace boost::multi_index;

#include "validate/validate.hh"
#include "osm/osmobjects.hh"
#include "osm/osmchange.hh"
#include <ogr_geometry.h>

#include "stats/statsconfig.hh"

using namespace osmobjects;

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include "utils/log.hh"
using namespace logger;

namespace osmchange {

/// And OsmChange file contains the data of the actual change. It uses the same
/// syntax as an OSM data file plus the addition of one of the three actions.
/// Nodes, ways, and relations can be created, deleted, or modified.
///
/// <modify>
///     <node id="12345" version="7" timestamp="2020-10-30T20:40:38Z" uid="111111" user="foo" changeset="93310152" lat="50.9176152" lon="-1.3751891"/>
/// </modify>
/// <delete>
///     <node id="23456" version="7" timestamp="2020-10-30T20:40:38Z" uid="22222" user="foo" changeset="93310152" lat="50.9176152" lon="-1.3751891"/>
/// </delete>
/// <create>
///     <node id="34567" version="1" timestamp="2020-10-30T20:15:24Z" uid="3333333" user="bar" changeset="93309184" lat="45.4303763" lon="10.9837526"/>
///</create>

// Read a changeset file from disk or memory into internal storage
bool
OsmChangeFile::readChanges(const std::string &file)
{
    setlocale(LC_ALL, "");
    std::ifstream change;
    int size = 0;
    unsigned char *buffer;
    log_debug("Reading OsmChange file %1%", file);
    std::string suffix = boost::filesystem::extension(file);
    // It's a gzipped file, common for files downloaded from planet
    std::ifstream ifile(file, std::ios_base::in | std::ios_base::binary);
    if (suffix == ".gz") { // it's a compressed file
        change.open(file, std::ifstream::in | std::ifstream::binary);
        try {
            boost::iostreams::filtering_streambuf<boost::iostreams::input>
                inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            inbuf.push(ifile);
            std::istream instream(&inbuf);
            // log_debug(instream.rdbuf());
            readXML(instream);
        } catch (std::exception &e) {
            log_debug("ERROR opening %1% %2%", file, e.what());
            // return false;
        }
    } else { // it's a text file
        change.open(file, std::ifstream::in);
        readXML(change);
    }
#if 0
    for (auto it = std::begin(osmchanges.changes); it != std::end(osmchanges.changes); ++it) {
        //state.created_at = it->created_at;
        //state.closed_at = it->closed_at;
        state.frequency = replication::changeset;
        state.path = file;
        under.writeState(state);
    }
#endif
    // FIXME: return a real value
    return true;
}

// Used for testing
void
OsmChangeFile::buildGeometriesFromNodeCache() {
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            for (auto lit = std::begin(way->refs); lit != std::end(way->refs); ++lit) {
                bg::append(way->linestring, nodecache[*lit]);
            }
            if (way->isClosed()) {
                way->polygon = { {std::begin(way->linestring), std::end(way->linestring)} };
            }
            std::stringstream ss;
            if (way->isClosed()) {
                ss << std::setprecision(12) << bg::wkt(way->polygon);
            } else {
                ss << std::setprecision(12) << bg::wkt(way->linestring);
            }
            waycache.insert(std::pair(way->id, std::make_shared<osmobjects::OsmWay>(*way)));
        }
        // for (auto rit = std::begin(change->relations); rit != std::end(change->relations); ++rit) {
        //     osmobjects::OsmRelation *relation = rit->get();
        //     buildRelationGeometry(*relation);
        // }
    }
}


typedef enum { inner, outer } member_role_t;

struct RelationGeometry {
    member_role_t role ;
    linestring_t linestring;
    polygon_t polygon;
};

void
OsmChangeFile::buildRelationGeometry(osmobjects::OsmRelation &relation) {
    
    std::vector<RelationGeometry> parts;
    linestring_t part;
    linestring_t lastLinestring;
    bool justClosed = false;
    bool first = true;

    std::vector<osmobjects::OsmRelationMember> members;
    
    // Skip members that are not Way
    for (auto mit = relation.members.begin(); mit != relation.members.end(); ++mit) {
        if (mit->type == osmobjects::way) {
            members.push_back(*mit);
        }
    }

    for (auto mit = members.begin(); mit != members.end(); ++mit) {
        // Process Way objects only, not Nodes or other Relations
        if (!waycache.count(mit->ref)) {
            // Way is not available in cache,
            // possibily because Relation is not in the priority area
            // or way was deleted
            return;
        }
        
        auto way = std::make_shared<osmobjects::OsmWay>();

        way = waycache.at(mit->ref);

        if (bg::num_points(way->linestring) > 0 &&
            bg::num_points(way->polygon) == 0)
        {

            // Linestrings

            if (!way->isClosed()) {

                // Reverse the line direction if it's necessary
                if (first && (std::next(mit) != members.end())) {
                    auto nextWay = std::make_shared<osmobjects::OsmWay>();
                    auto nextWayId = std::next(mit)->ref;
                    if (!waycache.count(nextWayId)) {
                        // Way is not available in cache,
                        // possibily because Relation is not in the priority area
                        // or way was deleted
                        return;
                    }
                    nextWay = waycache.at(nextWayId);

                    if ( bg::num_points(nextWay->linestring) > 0 && 
                        bg::num_points(way->linestring) > 0 && (
                        bg::equals(way->linestring.front(), nextWay->linestring.front()) ||
                        bg::equals(way->linestring.front(), nextWay->linestring.back())
                    )) {
                        bg::reverse(way->linestring);
                    }
                } else {
                    if ( bg::num_points(way->linestring) > 0 &&
                         bg::num_points(lastLinestring) > 0 ) {
                        if (bg::equals(way->linestring.back(), lastLinestring.back())) {
                            bg::reverse(way->linestring);
                        }
                    }
                }

                bg::append(part, way->linestring);

                // Check if object is closed
                if (relation.isMultiPolygon() && bg::equals(part.back(), part.front())) {
                    // Convert LineString to Polygon
                    polygon_t polygon;
                    bg::append(polygon.outer(), part);
                    if (mit->role == "inner") {
                        parts.push_back({
                            member_role_t::inner,
                            linestring_t(),
                            { polygon }
                        });
                    } else {
                        parts.push_back({
                            member_role_t::outer,
                            linestring_t(),
                            { polygon }
                        });
                    }
                    part.clear();
                    first = true;
                    justClosed = true;
                    lastLinestring.clear();
                } else if (std::next(mit) != members.end()) {
                    // Check if object is disconnected
                    auto nextWay = std::make_shared<osmobjects::OsmWay>();
                    auto nextWayId = std::next(mit)->ref;
                    if (!waycache.count(nextWayId)) {
                        // Way is not available in cache,
                        // possibily because Relation is not in the priority area
                        // or way was deleted
                        return;
                    }
                    nextWay = waycache.at(nextWayId);
                    if ( (bg::num_points(way->linestring) > 0 && bg::num_points(nextWay->linestring) > 0 &&
                        !bg::equals(way->linestring.back(), nextWay->linestring.front()) &&
                        !bg::equals(way->linestring.back(), nextWay->linestring.back())) ||
                        (bg::num_points(nextWay->linestring) == 0)
                    ) {
                        parts.push_back({
                            member_role_t::outer,
                            { part },
                            polygon_t()
                        });
                        part.clear();
                        first = true;
                        justClosed = true;
                        lastLinestring.clear();
                    }
                }


            } else {
                // Convert LineString to Polygon
                if (mit->role == "inner") {
                    parts.push_back({
                        member_role_t::inner,
                        linestring_t(),
                        { way->polygon }
                    });
                } else {
                    parts.push_back({
                        member_role_t::outer,
                        linestring_t(),
                        { way->polygon }
                    });
                }
            }

            lastLinestring = way->linestring;

        } else {

            // Polygons

            // When Relation is MultiLineString but way's geometry is a Polygon
            if (!relation.isMultiPolygon() && bg::num_points(way->linestring) == 0 &&
                bg::num_points(way->polygon) > 0
            ) {
                // Convert way's Polygon to LineString
                bg::assign_points(way->linestring, way->polygon.outer());
                if (mit->role == "inner") {
                        parts.push_back({
                            member_role_t::inner,
                            { way->linestring },
                            polygon_t()
                        });
                } else {
                        parts.push_back({
                            member_role_t::outer,
                            { way->linestring },
                            polygon_t()
                        });
                }
            } else {
                if (mit->role == "inner") {
                    parts.push_back({
                        member_role_t::inner,
                        linestring_t(),
                        { way->polygon }
                    });
                } else {
                    if (way->polygon.outer().size() > 0) {
                        parts.push_back({
                            member_role_t::outer,
                            linestring_t(),
                            { way->polygon }
                        });
                    } else {
                        parts.push_back({
                            member_role_t::outer,
                            { way->linestring },
                            polygon_t()
                        });
                    }
                }
            }

        }

        if (first && !justClosed) {
            first = false;
        }
        if (justClosed) {
            justClosed = false;
        }

    }

    if (part.size() > 0) {
        parts.push_back({
            member_role_t::outer,
            { part },
            polygon_t()
        });
    }

    std::string geometry = "";
    int i = 0;
    for (auto pit = parts.begin(); pit != parts.end(); ++pit) {
        std::stringstream ss;
        std::string geometry_str;
        ++i;
        if (relation.isMultiPolygon()) {
            if (bg::num_points(pit->polygon.outer()) > 0) {
                ss << std::setprecision(12) << bg::wkt(pit->polygon);
                geometry_str = ss.str();
                // Erase "POLYGON("
                geometry_str.erase(0,8);
                geometry_str.erase(geometry_str.size() - 1);
                if (geometry_str.size() > 0) {
                    geometry += geometry_str + ",";
                }
            }
        } else {
            if (bg::num_points(pit->linestring) > 0) {
                ss << std::setprecision(12) << bg::wkt(pit->linestring);
                geometry_str = ss.str();
                // Erase "LINESTRING("
                geometry_str.erase(0,11);
                geometry_str.erase(geometry_str.size() - 1);
                if (geometry_str.size() > 0) {
                    geometry += "(" + geometry_str + "),";
                }
            }
        }
    }

    // std::cout << "<Relation " << relation.id << ">" << std::endl;
    if (geometry.size() > 0) {
        geometry.erase(geometry.size() - 1);
        if (relation.isMultiPolygon()) {
            // std::cout << "MULTIPOLYGON((" + geometry + "))" << std::endl;
            bg::read_wkt("MULTIPOLYGON((" + geometry + "))", relation.multipolygon);
        } else {
            // std::cout << "MULTILINESTRING(" + geometry + ")" << std::endl;
            bg::read_wkt("MULTILINESTRING(" + geometry + ")", relation.multilinestring);
        }
    }
}

bool
OsmChangeFile::readXML(std::istream &xml)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("OsmChangeFile::readXML: took %w seconds\n");
#endif
    // On non-english numeric locales using decimal separator different than '.'
    // this is necessary to parse lat-lon with std::stod correctly without
    // loosing precision
    // std::setlocale(LC_NUMERIC, "C");
    setlocale(LC_NUMERIC, "C");
    // log_debug("OsmChangeFile::readXML(): " << xml.rdbuf());
    std::ofstream myfile;
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
        // log_error("libxml++ exception: %1%", ex.what());
        // log_debug(xml.rdbuf());
        int return_code = EXIT_FAILURE;
    }
#else
    // Boost::parser_tree with RapidXML is faster, but builds a DOM tree
    // so loads the entire file into memory. Most replication files for
    // hourly or minutely changes are small, so this is better for that
    // case.
    boost::property_tree::ptree pt;
#ifdef USE_TMPFILE
    boost::property_tree::read_xml("tmp.xml", pt);
#else
    try {
        boost::property_tree::read_xml(xml, pt);
    } catch (exception& boost::property_tree::xml_parser::xml_parser_error) {
        log_error("Error parsing XML");
        return false;
    }
#endif

    if (pt.empty()) {
        log_error("ERROR: XML data is empty!");
        return false;
    }

    //    boost::progress_display show_progress( 7000 );

    for (auto value: pt.get_child("osmChange")) {
        OsmChange change;
        action_t action;
        if (value.first == "modify") {
            change.action = modify;
        } else if (value.first == "create") {
            change.action = create;
        } else if (value.first == "remove") { // delete is a reserved word
            change.action = remove;
        }
        for (auto child: value.second) {
            // Process the tags. These don't exist for every element.
            // Both nodes and ways have tags
            for (auto tag: value.second) {
                if (tag.first == "tag") {
                    std::string key = tag.second.get("<xmlattr>.k", "");
                    std::string val = tag.second.get("<xmlattr>.v", "");
                } else if (tag.first == "nd") {
                    long ref = tag.second.get("<xmlattr>.ref", 0);
                    change.addRef(ref);
                }
            }
            // Only nodes have coordinates
            if (child.first == "node") {
                double lat = value.second.get("<xmlattr>.lat", 0.0);
                double lon = value.second.get("<xmlattr>.lon", 0.0);

                auto object = std::make_shared<OsmNode>();
                object->setLatitude(lat);
                object->setLongitude(lon);
                object->setAction(change.action);
                change.nodes.push_back(object);
            } else if (child.first == "way") {
                auto object = std::make_shared<OsmWay>();
                change.ways.push_back(object);
            } else if (child.first == "relation") {
                auto object = std::make_shared<OsmRelation>();
                change.relations.push_back(object);
            }

            // Process the attributes, which do exist in every element
            change.id = value.second.get("<xmlattr>.id", 0);
            change.version = value.second.get("<xmlattr>.version", 0);
            change.timestamp =
                value.second.get("<xmlattr>.timestamp",
                boost::posix_time::second_clock::universal_time());
            change.user = value.second.get("<xmlattr>.user", "");
            change.uid = value.second.get("<xmlattr>.uid", 0);
            //changes.push_back(change);
            // change.dump();
        }
        //        ++show_progress;
    }
#endif
    // FIXME: return a real value
    return false;
}

#ifdef LIBXML
// Called by libxml++ for each element of the XML file
void
OsmChangeFile::on_start_element(const Glib::ustring &name,
                                const AttributeList &attributes)
{

    // If a change is in progress, apply to to that instance
    std::shared_ptr<OsmChange> change;
    // log_debug("NAME: %1%", name);
    // Top level element can be ignored
    if (name == "osmChange") {
        return;
    }
    // There are 3 change states to handle, each one contains possibly multiple
    // nodes and ways.
    if (name == "create") {
        change = std::make_shared<OsmChange>(osmobjects::create);
        changes.push_back(change);
        return;
    } else if (name == "modify") {
        change = std::make_shared<OsmChange>(osmobjects::modify);
        changes.push_back(change);
        return;
    } else if (name == "delete") {
        change = std::make_shared<OsmChange>(osmobjects::remove);
        changes.push_back(change);
        return;
    } else {
        change = changes.back();
    }

    // std::shared_ptr<OsmObject> obj;
    if (name == "node") {
        change->obj.reset();
        change->obj = changes.back()->newNode();
        change->obj->action = changes.back()->action;
    } else if (name == "tag") {
        // A tag element has only has 1 attribute, and numbers are stored as
        // strings
        change->obj->tags[attributes[0].value] = attributes[1].value;
        return;
    } else if (name == "way") {
        change->obj.reset();
        change->obj = changes.back()->newWay();
        change->obj->action = changes.back()->action;
    } else if (name == "relation") {
        change->obj = changes.back()->newRelation();
        change->obj->action = changes.back()->action;
    } else if (name == "member") {
        // Process relation attributes
        long ref = -1;
        osmobjects::osmtype_t type = osmobjects::osmtype_t::empty;
        std::string role;
        for (const auto &a: std::as_const(attributes)) {
            if (a.name == "type") {
                if (a.value == "way") {
                    type = osmobjects::osmtype_t::way;
                } else if (a.value == "node") {
                    type = osmobjects::osmtype_t::node;
                } else if (a.value == "relation") {
                    type = osmobjects::osmtype_t::relation;
                } else {
                    log_debug("Invalid relation type '%1%'!", a.value);
                }
            } else if (a.name == "ref") {
                ref = std::stol(a.value);
            } else if (a.name == "role") {
                role = a.value;
            } else {
                log_debug("Invalid attribute '%1%' in relation member!",
                          a.name);
            }
        }
        if (ref != -1 && type != osmobjects::osmtype_t::empty) {
            changes.back()->addMember(ref, type, role);
        } else {
            log_debug("Invalid relation (ref: %1%, type: %2%, role: %3%",
                      ref, type, role);
        }
    } else if (name == "nd") {
        long ref = std::stol(attributes[0].value);
        changes.back()->addRef(ref);
    }

    // process the attributes
    std::string cache;
    for (const auto &attr_pair: attributes) {
        // Sometimes the data string is unicode
        // std::wcout << "\tPAIR: " << attr_pair.name << " = " << attr_pair.value);
        // tags use a 'k' for the key, and 'v' for the value
        if (attr_pair.name == "ref") {
        } else if (attr_pair.name == "k") {
            cache = attr_pair.value;
            continue;
        } else if (attr_pair.name == "v") {
            if (cache == "timestamp") {
                std::string tmp = attr_pair.value;
                tmp[10] = ' '; // Drop the 'T' in the middle
                tmp.erase(19); // Drop the final 'Z'
                change->obj->timestamp = time_from_string(tmp);
                change->final_entry = change->obj->timestamp;
            } else {
                cache.clear();
            }
        } else if (attr_pair.name == "timestamp") {
            // Clean up the string to something boost can parse
            std::string tmp = attr_pair.value;
            tmp[10] = ' '; // Drop the 'T' in the middle
            tmp.erase(19); // Drop the final 'Z'
            change->obj->timestamp = time_from_string(tmp);
            change->final_entry = change->obj->timestamp;
        } else if (attr_pair.name == "id") {
            change->obj->id = std::stol(attr_pair.value);
        } else if (attr_pair.name == "uid") {
            change->obj->uid = std::stol(attr_pair.value);
        } else if (attr_pair.name == "version") {
            change->obj->version = std::stod(attr_pair.value);
        } else if (attr_pair.name == "user") {
            change->obj->user = attr_pair.value;
        } else if (attr_pair.name == "changeset") {
            change->obj->changeset = std::stol(attr_pair.value);
        } else if (attr_pair.name == "lat") {
            auto lat = reinterpret_cast<OsmNode *>(change->obj.get());
            lat->setLatitude(std::stod(attr_pair.value));
            nodecache[lat->id] = lat->point;
        } else if (attr_pair.name == "lon") {
            auto lon = reinterpret_cast<OsmNode *>(change->obj.get());
            lon->setLongitude(std::stod(attr_pair.value));
            nodecache[lon->id] = lon->point;
        }
    }
}
#endif // EOF LIBXML

void
OsmChange::dump(void)
{
    std::cerr << "------------" << std::endl;
    std::cerr << "Dumping OsmChange()" << std::endl;
    if (action == osmobjects::create) {
        std::cerr << "\tAction: create" << std::endl;
    } else if (action == osmobjects::modify) {
        std::cerr << "\tAction: modify" << std::endl;
    } else if (action == osmobjects::remove) {
        std::cerr << "\tAction: delete" << std::endl;
    } else if (action == osmobjects::none) {
        std::cerr << "\tAction: data element" << std::endl;
    }

    if (nodes.size() > 0) {
        std::cerr << "\tDumping nodes:" << std::endl;
        for (auto it = std::begin(nodes); it != std::end(nodes); ++it) {
            std::shared_ptr<OsmNode> node = *it;
            node->dump();
        }
    }
    if (ways.size() > 0) {
        std::cerr << "\tDumping ways:" << std::endl;
        for (auto it = std::begin(ways); it != std::end(ways); ++it) {
            std::shared_ptr<OsmWay> way = *it;
            way->dump();
        }
    }
    if (relations.size() > 0) {
        for (auto it = std::begin(relations); it != std::end(relations); ++it) {
            // std::cerr << "\tDumping relations: " << it->dump() << std::endl;
            // std::shared_ptr<OsmWay> rel = *it;
            // rel->dump( << std::endl;
        }
    }
    std::cerr << "Final timestamp: " << to_simple_string(final_entry) << std::endl;

}

void
OsmChangeFile::dump(void)
{
    std::cerr << "Dumping OsmChangeFile()" << std::endl;
    std::cerr << "There are " << changes.size() << " changes" << std::endl;
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        OsmChange *change = it->get();
        change->dump();
    }
    if (userstats.size() > 0) {
        for (auto it = std::begin(userstats); it != std::end(userstats); ++it) {
            std::shared_ptr<ChangeStats> stats = it->second;
            stats->dump();
        }
    }
#if 0
    std::cerr << "\tDumping nodecache:" << std::endl;
    for (auto it = std::begin(nodecache); it != std::end(nodecache); ++it) {
        std::cerr << "\t\t: " << it->first << ": " << bg::wkt(it->second) << std::endl;
    }
#endif
}


void
OsmChangeFile::areaFilter(const multipolygon_t &poly)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("OsmChangeFile::areaFilter: took %w seconds\n");
#endif
    std::map<long, bool> priority;
    for (auto it = std::begin(changes); it != std::end(changes); it++) {

        OsmChange *change = it->get();
        bool debug = false;

        // Filter nodes
        for (auto nit = std::begin(change->nodes); nit != std::end(change->nodes); ++nit) {
            OsmNode *node = nit->get();
            if (poly.empty() || bg::within(node->point, poly)) {
                node->priority = true;
                nodecache[node->id] = node->point;
            } else if (!bg::within(node->point, poly)) {
                node->priority = false;
            }
        }

        // Filter ways
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            OsmWay *way = wit->get();
            if (poly.empty()) {
                way->priority = true;
            } else {
                way->priority = false;
                for (auto rit = std::begin(way->refs); rit != std::end(way->refs); ++rit) {
                    if (nodecache.count(*rit) && bg::within(nodecache[*rit], poly)) {
                        way->priority = true;
                        break;
                    }
                }
            }
            if (waycache.count(way->id)) {
                waycache.at(way->id)->priority = way->priority;
            }
            
        }

        // Filter relations
        for (auto rit = std::begin(change->relations); rit != std::end(change->relations); ++rit) {
            OsmRelation *relation = rit->get();
            relation->priority = true;
            if (!poly.empty()) {
                for (auto mit = std::begin(relation->members); mit != std::end(relation->members); ++mit) {
                    if (waycache.count(mit->ref)) {
                        auto way = waycache.at(mit->ref);
                        if (!way->priority) {
                            relation->priority = false;
                            break;
                        }
                    } else {
                        relation->priority = false;
                        break;
                    }
                }
            }
        }
    }
}

std::shared_ptr<std::map<long, std::shared_ptr<ChangeStats>>>
OsmChangeFile::collectStats(const multipolygon_t &poly)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("OsmChangeFile::collectStats: took %w seconds\n");
#endif

    auto mstats =
        std::make_shared<std::map<long, std::shared_ptr<ChangeStats>>>();
        std::shared_ptr<ChangeStats> ostats;

    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        OsmChange *change = it->get();
        // Stats for Nodes
        for (auto it = std::begin(change->nodes); it != std::end(change->nodes); ++it) {
            OsmNode *node = it->get();
            if (!node->priority) {
                continue;
            }
            // Some older nodes in a way wound up with this one tag, which nobody noticed,
            // so ignore it.
            if (node->tags.size() == 1 &&
                node->tags.find("created_at") != node->tags.end()) {
                continue;
            }
            ostats = (*mstats)[node->changeset];
            if (ostats.get() == 0) {
                ostats = std::make_shared<ChangeStats>();
                ostats->changeset = node->changeset;
                ostats->uid = node->uid;
                ostats->username = node->user;
                ostats->closed_at = node->timestamp;
                (*mstats)[node->changeset] = ostats;
            }
            auto hits = scanTags(node->tags, osmchange::node);
            for (auto hit = std::begin(*hits); hit != std::end(*hits); ++hit) {
                if (node->action == osmobjects::create) {
                    ostats->added[*hit]++;
                } else if (node->action == osmobjects::modify) {
                    ostats->modified[*hit]++;
                }
            }
        }

        // Stats for Ways
        for (auto it = std::begin(change->ways); it != std::end(change->ways); ++it) {
            OsmWay *way = it->get();
            if (!way->priority) {
                continue;
            }
            // If there are no tags, assume it's part of a relation
            if (way->tags.size() == 0 && way->action != osmobjects::remove) {
                continue;
            }
            if (way->action == osmobjects::remove) {
                continue;
            }

            // Some older ways in a way wound up with this one tag, which nobody noticed,
            // so ignore it.
            if (way->tags.size() == 1 && way->tags.find("created_at") != way->tags.end()) {
                continue;
            }
            ostats = (*mstats)[way->changeset];
            if (ostats.get() == 0) {
                ostats = std::make_shared<ChangeStats>();
                ostats->changeset = way->changeset;
                ostats->uid = way->uid;
                ostats->username = way->user;
                ostats->closed_at = way->timestamp;
                (*mstats)[way->changeset] = ostats;
            }

            auto hits = scanTags(way->tags, osmchange::way);
            for (auto hit = std::begin(*hits); hit != std::end(*hits); ++hit) {

                if (way->action == osmobjects::create) {
                    ostats->added[*hit]++;
                } else if (way->action == osmobjects::modify) {
                    ostats->modified[*hit]++;
                }

                // Calculate length
                if ( (*hit == "highway" || *hit == "waterway") && way->action == osmobjects::create) {
                    // Get the geometry behind each reference
                    bg::model::linestring<sphere_t> globe;
                    for (auto lit = std::begin(way->refs); lit != std::end(way->refs); ++lit) {
                        double x = nodecache[*lit].get<0>();
                        double y = nodecache[*lit].get<1>();
                        if (x != 0 && y != 0) {
                            globe.push_back(sphere_t(x,y));
                            bg::append(way->linestring, nodecache[*lit]);
                        }
                    }
                    std::string tag;
                    if (*hit == "highway") {
                        tag = "highway_km";
                    }
                    if (*hit == "waterway") {
                        tag = "waterway_km";
                    }
                    double length = bg::length(globe,
                            bg::strategy::distance::haversine<float>(6371.0));
                    // log_debug("LENGTH: %1% %2%", std::to_string(length), way->changeset);
                    ostats->added[tag] += length;
                }
            }
        }

        // Stats for Relations
        for (auto it = std::begin(change->relations); it != std::end(change->relations); ++it) {
            OsmRelation *relation = it->get();
            if (!relation->priority) {
                continue;
            }
            // If there are no tags, ignore it
            if (relation->tags.size() == 0) {
                continue;
            }
            ostats = (*mstats)[relation->changeset];
            if (ostats.get() == 0) {
                ostats = std::make_shared<ChangeStats>();
                ostats->changeset = relation->changeset;
                ostats->uid = relation->uid;
                ostats->username = relation->user;
                ostats->closed_at = relation->timestamp;
                (*mstats)[relation->changeset] = ostats;
            }
            auto hits = scanTags(relation->tags, osmchange::relation);
            for (auto hit = std::begin(*hits); hit != std::end(*hits); ++hit) {
                if (relation->action == osmobjects::create) {
                    ostats->added[*hit]++;
                } else if (relation->action == osmobjects::modify) {
                    ostats->modified[*hit]++;
                }
            }
        }
    }
    return mstats;
}

std::shared_ptr<std::vector<std::string>>
OsmChangeFile::scanTags(std::map<std::string, std::string> tags, osmchange::osmtype_t type)
{
    auto statsconfig = statsconfig::StatsConfig();
    auto hits = std::make_shared<std::vector<std::string>>();
    for (auto it = std::begin(tags); it != std::end(tags); ++it) {
        if (!it->second.empty()) {
            std::string hit = "";
            if (type == node) {
                hit = statsconfig.search(it->first, it->second, node);
            } else if (type == way) {
                hit = statsconfig.search(it->first, it->second, way);
            } else if (type == relation) {
                hit = statsconfig.search(it->first, it->second, relation);
            }
            if (!hit.empty()) {
                hits->push_back(hit);
            }
        }
    }

    return hits;
}

/// Dump internal data to the terminal, only for debugging
void
ChangeStats::dump(void)
{
    std::cerr << "Dumping ChangeStats for: \t " << changeset << std::endl;
    std::cerr << "\tUser ID: \t\t " << uid << std::endl;
    std::cerr << "\tUser Name: \t\t " << username << std::endl;
    std::cerr << "\tAdded features: " << added.size() << std::endl;
    for (auto it = std::begin(added); it != std::end(added); ++it) {
        std::cerr << "\t\t" << it->first << " = " << it->second << std::endl;
    }
    std::cerr << "\tModified features: " << modified.size() << std::endl;
    for (auto it = std::begin(modified); it != std::end(modified); ++it) {
        std::cerr << "\t\t" << it->first << " = " << it->second << std::endl;
    }
    // std::cerr << "\tDeleted features: " << added.size() << std::endl;
    // for (auto it = std::begin(deleted << std::endl; it != std::end(deleted << std::endl; ++it) {
    //     std::cerr << "\t\t" << it->first << " = " << it->second << std::endl;
    // }
};

std::shared_ptr<std::vector<std::shared_ptr<ValidateStatus>>>
OsmChangeFile::validateNodes(const multipolygon_t &poly, std::shared_ptr<Validate> &plugin)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("OsmChangeFile::validateNodes: took %w seconds\n");
#endif
    auto totals =
        std::make_shared<std::vector<std::shared_ptr<ValidateStatus>>>();
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        OsmChange *change = it->get();
        for (auto nit = std::begin(change->nodes);
            nit != std::end(change->nodes); ++nit) {
            OsmNode *node = nit->get();
            if (!node->priority || node->tags.empty() || node->action == osmobjects::remove) {
                continue;
            }
            std::vector<std::string> node_tests = {"building", "natural", "place", "waterway"};
            for (auto test_it = std::begin(node_tests); test_it != std::end(node_tests); ++test_it) {
                if (node->containsKey(*test_it)) {
                    auto status = plugin->checkNode(*node, *test_it);
                    totals->push_back(status);
                }
            }
        }
    }
    return totals;
}

std::shared_ptr<std::vector<std::shared_ptr<ValidateStatus>>>
OsmChangeFile::validateWays(const multipolygon_t &poly, std::shared_ptr<Validate> &plugin)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("OsmChangeFile::validateWays: took %w seconds\n");
#endif
    auto totals = std::make_shared<std::vector<std::shared_ptr<ValidateStatus>>>();
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        OsmChange *change = it->get();
        for (auto nit = std::begin(change->ways); nit != std::end(change->ways); ++nit) {
            OsmWay *way = nit->get();
            if (!way->priority) {
                continue;
            }
            auto status = plugin->checkWay(*way, "building");
            totals->push_back(status);
        }
    }
    return totals;
}

} // namespace osmchange

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
