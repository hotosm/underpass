//
// Copyright (c) 2020, 2001 Humanitarian OpenStreetMap Team
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
#include <array>
#include <memory>
#include <iostream>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <pqxx/pqxx>
#ifdef LIBXML
# include <libxml++/libxml++.h>
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
#include <boost/progress.hpp>
// #include <boost/multi_index_container.hpp>
// #include <boost/multi_index/member.hpp>
// #include <boost/multi_index/ordered_index.hpp>
// using boost::multi_index_container;
// using namespace boost::multi_index;

#include "hotosm.hh"
#include "timer.hh"
#include "osmstats/osmchange.hh"
#include "data/osmobjects.hh"
#include <ogr_geometry.h>

using namespace osmobjects;

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include "log.hh"
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
    log_debug(_("Reading OsmChange file %1%"), file);
    std::string suffix = boost::filesystem::extension(file);
    // It's a gzipped file, common for files downloaded from planet
    std::ifstream ifile(file, std::ios_base::in | std::ios_base::binary);
    if (suffix == ".gz") {  // it's a compressed file
        change.open(file,  std::ifstream::in |  std::ifstream::binary);
        try {
            boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            inbuf.push(ifile);
            std::istream instream(&inbuf);
            // log_debug(_(instream.rdbuf();
            readXML(instream);
        } catch (std::exception& e) {
            log_debug(_("ERROR opening %1% %2%"),file, e.what());
            // return false;
        }
    } else {                // it's a text file
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

bool
OsmChangeFile::readXML(std::istream &xml)
{
    // log_debug(_("OsmChangeFile::readXML(): " << xml.rdbuf();
    std::ofstream myfile;
#ifdef LIBXML
    // libxml calls on_element_start for each node, using a SAX parser,
    // and works well for large files.
    Timer timer;
    try {
        set_substitute_entities(true);
        timer.startTimer();
        parse_stream(xml);
        timer.endTimer("libxml++");
    }
    catch(const xmlpp::exception& ex) {
        log_error(_("libxml++ exception: %1%"), ex.what());
        // log_debug(xml.rdbuf());
        int return_code = EXIT_FAILURE;
    }
#else
    // Boost::parser_tree with RapidXML is faster, but builds a DOM tree
    // so loads the entire file into memory. Most replication files for
    // hourly or minutely changes are small, so this is better for that
    // case.
    boost::property_tree::ptree pt;
    Timer timer;
    timer.startTimer();
#ifdef USE_TMPFILE
    boost::property_tree::read_xml("tmp.xml", pt);
#else
    boost::property_tree::read_xml(xml, pt);
#endif

    timer.endTimer("parse_tree");

    if (pt.empty()) {
        log_error(_("ERROR: XML data is empty!"));
        // return false;
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
                    // static_cast<OsmNode *>(object)->addTag(key, val);
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
g               change.relations.push_back(object);
           }
           
           // Process the attributes, which do exist in every element
           change.id = value.second.get("<xmlattr>.id", 0);
           change.version = value.second.get("<xmlattr>.version", 0);
           change.timestamp = value.second.get("<xmlattr>.timestamp",
                                  boost::posix_time::second_clock::local_time());
           change.user = value.second.get("<xmlattr>.user", "");
           change.uid = value.second.get("<xmlattr>.uid", 0);
           //changes.push_back(change);
           change.dump();
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
OsmChangeFile::on_start_element(const Glib::ustring& name,
                                const AttributeList& attributes)
{
    // If a change is in progress, apply to to that instance
    std::shared_ptr<OsmChange> change;
    // log_debug(_("NAME: %1%"), name));
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
        change->obj  = changes.back()->newRelation();
        change->obj->action = changes.back()->action;
    } else if (name == "member") {
        // It's a member of a relation
    } else if (name == "nd") {
        changes.back()->addRef(std::stol(attributes[0].value));
    }

    // process the attributes
    std::string cache;
    for (const auto& attr_pair : attributes) {
        // Sometimes the data string is unicode
        // std::wcout << "\tPAIR: " << attr_pair.name << " = " << attr_pair.value);
        // tags use a 'k' for the key, and 'v' for the value
        if (attr_pair.name == "ref") {
            //static_cast<OsmWay *>(object)->refs.push_back(std::stol(attr_pair.value));
        } else if (attr_pair.name == "k") {
            cache = attr_pair.value;
            continue;
        } else if (attr_pair.name == "v") {
            if (cache == "timestamp") {
                std::string tmp = attr_pair.value;
                tmp[10] = ' ';      // Drop the 'T' in the middle
                tmp.erase(19);      // Drop the final 'Z'
                // change->setTimestamp(tmp);
                change->obj->timestamp = time_from_string(tmp);
            } else {
                // obj->tags[cache] = attr_pair.value;
                cache.clear();
            }
         } else if (attr_pair.name == "timestamp") {
            // Clean up the string to something boost can parse
            std::string tmp = attr_pair.value;
            tmp[10] = ' ';      // Drop the 'T' in the middle
            tmp.erase(19);      // Drop the final 'Z'
            change->obj->timestamp = time_from_string(tmp);
            // change->setTimestamp(tmp);
        } else if (attr_pair.name == "id") {
            // change->setChangeID(std::stol(attr_pair.value));
            change->obj->id = std::stol(attr_pair.value);
        } else if (attr_pair.name == "uid") {
            // change->setUID(std::stol(attr_pair.value));
            change->obj->uid = std::stol(attr_pair.value);
        } else if (attr_pair.name == "version") {
            // change->setVersion(std::stod(attr_pair.value));
            change->obj->version = std::stod(attr_pair.value);
        } else if (attr_pair.name == "user") {
            // change->setUser(attr_pair.value);
            change->obj->user = attr_pair.value;
        } else if (attr_pair.name == "changeset") {
            // change->setChangeID(std::stol(attr_pair.value));
            change->obj->change_id = std::stol(attr_pair.value);
        } else if (attr_pair.name == "lat") {
            // static_cast<OsmNode>(*obj).setLatitude(std::stod(attr_pair.value));
            OsmNode *foo = reinterpret_cast<OsmNode *>(change->obj.get());
            foo->setLatitude(std::stod(attr_pair.value));
        } else if (attr_pair.name == "lon") {
            // obj->setLongitude(std::stod(attr_pair.value));
            OsmNode *foo = reinterpret_cast<OsmNode *>(change->obj.get());
            foo->setLongitude(std::stod(attr_pair.value));
        }
        // obj->dump();
    }
}
#endif  // EOF LIBXML

void
OsmChange::dump(void)
{
    std::cerr << "------------" << std::endl;
    std::cerr << "Dumping OsmChange()" << std::endl;
    if (action == osmobjects::create) {
        std::cerr << "\tAction: create" << std::endl;
    } else if(action == osmobjects::modify) {
        std::cerr << "\tAction: modify" << std::endl;
    } else if(action == osmobjects::remove) {
        std::cerr << "\tAction: delete" << std::endl;
    } else if(action == osmobjects::none) {
        std::cerr << "\tAction: data element" << std::endl;
    }

    if (nodes.size() > 0) {
        std::cerr << "\tDumping nodes:"  << std::endl;
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
}

std::shared_ptr<std::map<long, std::shared_ptr<ChangeStats>>>
OsmChangeFile::collectStats(const multipolygon_t &poly)
{
    // FIXME: stuff to extract for MERL
    // names added to villages, neigborhood, or citys
    // facilities added (aggregated by schools, clinics, water points, bridges,
    // airports and administrative boundaries)
    auto mstats = std::make_shared<std::map<long, std::shared_ptr<ChangeStats>>>();
    std::shared_ptr<ChangeStats> ostats;

    log_debug(_("Collecting Statistics for: %1%"), changes.size());

    std::map<long, point_t> nodecache;
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
	nodecache.clear();
        OsmChange *change = it->get();
        // change->dump();
        for (auto it = std::begin(change->nodes); it != std::end(change->nodes); ++it) {
            OsmNode *node = it->get();
	    auto wkt = boost::geometry::wkt(node->point);
	    nodecache[node->id] = node->point;
	    // Filter data by polygon
	    if (!boost::geometry::within(node->point, poly)) {
		log_debug(_("Changeset with node %1% is not in a priority area"), node->change_id);
		continue;
	    } else {
		log_debug(_("Changeset with node %1% is in a priority area"), node->change_id);
	    }
	    // Some older nodes in a way wound up with this one tag, which nobody noticed,
	    // so ignore it.
	    if (node->tags.size() == 1 && node->tags.find("created_at") != node->tags.end()) {
		continue;
	    }
	    // node->dump();
	    ostats = (*mstats)[node->change_id];
	    if (ostats.get() == 0) {
		ostats = std::make_shared<ChangeStats>();
		ostats->change_id = node->id;
		ostats->user_id = node->uid;
		ostats->username = node->user;
		(*mstats)[node->change_id] = ostats;
	    }
	    auto hits = scanTags(node->tags);
	    for (auto hit = std::begin(*hits); hit != std::end(*hits); ++hit) {
		// log_debug(_("FIXME node: " << *hit << " : " << (int)node->action);
		if (node->action == osmobjects::create) {
		    ostats->added[*hit]++;
		} else if (node->action == osmobjects::modify){
		    ostats->modified[*hit]++;
		}
	    }
	}
        for (auto it = std::begin(change->ways); it != std::end(change->ways); ++it) {
            OsmWay *way = it->get();
	    // If there are no tags, assume it's part of a relation
            if (way->tags.size() == 0) {
		// continue;
	    }
	    // Filter data by polygon
	    // Some older ways in a way wound up with this one tag, which nobody noticed,
	    // so ignore it.
	    if (way->tags.size() == 1 && way->tags.find("created_at") != way->tags.end()) {
		continue;
	    }
	    ostats = (*mstats)[way->change_id];
	    if (ostats.get() == 0) {
		ostats = std::make_shared<ChangeStats>();
		ostats->change_id = way->id;
		ostats->user_id = way->uid;
		ostats->username = way->user;
		(*mstats)[way->change_id] = ostats;
	    }
	    auto hits = scanTags(way->tags);
	    for (auto hit = std::begin(*hits); hit != std::end(*hits); ++hit) {
		// log_debug("FIXME way: ", *hit, (int)way->action);
		way->dump();
		if (*hit == "highway" || *hit == "waterway") {
		    // Get the geometry behind each reference
		    boost::geometry::model::linestring<sphere_t> line;
		    for (auto lit = std::begin(way->refs); lit != std::end(way->refs); ++lit) {
			line.push_back(sphere_t(nodecache[*lit].get<0>(),
                                                nodecache[*lit].get<1>()));
		    }
		    // Get the middle point of the linestring on the sphere
		    long ref = way->refs[way->refs.size()/2];
		    point_t pt(nodecache[ref].get<0>(), nodecache[ref].get<1>());
		    if (!boost::geometry::within(pt, poly)) {
			log_debug(_("Changeset with way %1% is not in a priority area"), way->change_id);
			continue;
		    } else {
			log_debug(_("Changeset with way %1% is in a priority area"), way->change_id);
		    }
		    std::string tag;
		    if (*hit == "highway" && way->action == osmobjects::create) {
			tag = "highway_km_added";
		    } else if (*hit == "highway" && way->action == osmobjects::modify) {
			tag = "highway_km_modified";
		    }
		    if (*hit == "waterway" && way->action == osmobjects::create) {
			tag = "waterway_km_added";
		    } else if (*hit == "highway" && way->action == osmobjects::modify) {
			tag = "waterway_km_modified";
		    }
		    double length = boost::geometry::length(line,
                        boost::geometry::strategy::distance::haversine<float>(6371.0)) * 1000;
		    // log_debug("LENGTH: %1%", std::to_string(length));
		    if (way->action == osmobjects::create) {
			ostats->added[tag] += length;
			ostats->added[*hit]++;
		    } else if (way->action == osmobjects::modify){
			ostats->modified[tag] += length;
			ostats->modified[*hit]++;
		    }
		}
	    }
	}
    }
    return mstats;
}

std::shared_ptr<std::vector<std::string>>
OsmChangeFile::scanTags(std::map<std::string, std::string> tags)
{
    auto hits = std::make_shared<std::vector<std::string>>();

    // These are values for the place tag
    std::vector<std::string> places = {
	"village",
	"hamlet",
	"neigborhood",
	"city",
	"town"
    };
    // These are values for the amenities tag
    std::vector<std::string> amenities = {
	"hospital",
	"school",
	"clinic",
	"kindergarten",
	"drinking_water",
	"health_facility",
	"health_center",
	"healthcare"
    };
    std::vector<std::string> buildings = {
	"hospital",
	"hut",
	"school",
	"healthcare"
	"clinic",
	"kindergarten",
	"health center",
	"health centre",
	"latrine",
	"latrines",
	"toilet",
	"toilets"
    };

    // These are values for the highway tag
    std::vector<std::string> highways = {
	"highway",
	"tertiary",
	"secondary",
	"unclassified",
	"track",
	"residential",
	"path",
	"bridge",
	"waterway"
    };

    std::vector<std::string> schools = {
	"primary",
	"secondary",
	"kindergarten"
    };
    // Some older nodes in a way wound up with this one tag, which nobody noticed,
    // so ignore it.
    if (tags.size() == 1 && tags.find("created_at") != tags.end()) {
	return hits;
    }
    std::map<std::string, bool> cache;
    for (auto it = std::begin(tags); it != std::end(tags); ++it) {
	// Look for nodes tagged
	if (it->first == "building" || it->first == "amenity" || it->first == "place" || it->first == "school" ||  it->first == "healthcare") {
	    auto match = std::find(amenities.begin(), amenities.end(), boost::algorithm::to_lower_copy(it->second));
	    if (match != amenities.end()) {
		if (!cache[it->second]) {
		    log_error(_("\tmatched amenity value: %1%"), it->second);
		    hits->push_back(boost::algorithm::to_lower_copy(it->second));
		    cache[it->second] = true;
		    // An amenity is a building, but the building tag may not be set
		    hits->push_back("building");
		} else {
		    continue;
		}
	    }
	    match = std::find(places.begin(), places.end(), boost::algorithm::to_lower_copy(it->second));
	    if (match != places.end()) {
		if (!cache[it->second]) {
		    log_error(_("\tmatched place value: %1%"), it->second);
		    hits->push_back(boost::algorithm::to_lower_copy(it->second));
		    hits->push_back("place");
		    cache[it->second] = true;
		}
	    }

	    // Add anything with the building tag set
	    if (it->first == "building") {
		hits->push_back("building");
		match = std::find(buildings.begin(), buildings.end(), boost::algorithm::to_lower_copy(it->second));
		if (match != buildings.end()) {
		    if (!cache[it->second]) {
			log_error(_("\tmatched building value: %1%"), it->second);
			hits->push_back(boost::algorithm::to_lower_copy(it->second));
			cache[it->second] = true;
		    }
		}
	    }
	    match = std::find(schools.begin(), schools.end(), boost::algorithm::to_lower_copy(it->second));
	    if (match != schools.end()) {
		if (!cache[it->second]) {
		    log_error(_("\tmatched school value: %1%"), it->second);
		    // Add the type of school
		    hits->push_back(boost::algorithm::to_lower_copy(it->second));
		    // Add a generic school accumulator
		    hits->push_back("school");
		    cache[it->second] = true;
		}
	    }
	}
	if (it->first == "highway") {
	    // hits->push_back("highway");
	    auto match = std::find(highways.begin(), highways.end(), boost::algorithm::to_lower_copy(it->second));
	    if (match != highways.end()) {
		if (!cache[it->second]) {
		    log_debug(_("\tmatched highway value: %1%"), it->second);
		    hits->push_back(it->first);
		    cache[it->second] = true;
		}
	    }
	}
	if (it->first == "waterway") {
	    if (!cache[it->second]) {
		log_debug(_("\tmatched waterway value: %1%"), it->second);
		hits->push_back(it->first);
		cache[it->second] = true;
	    }
	}
    }

    return hits;
}

/// Dump internal data to the terminal, only for debugging
void
ChangeStats::dump(void)
{
    std::cerr << "Dumping ChangeStats for: \t " << change_id << std::endl;
    std::cerr << "\tUser ID: \t\t " << user_id << std::endl;
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
    // 	std::cerr << "\t\t" << it->first << " = " << it->second << std::endl;
    // }
};

} // EOF namespace osmchange

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
