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

#ifndef __VALIDATE_HH__
#define __VALIDATE_HH__

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
# include "unconfig.h"
#endif

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <unordered_set>

#include <boost/config.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "data/osmobjects.hh"
#include "data/yaml.hh"
#include "log.hh"

using namespace logger;

/// \file validate.hh
/// \brief This class tries to validate the OSM objects
///
/// This class analyzes an OSM object to look for errors. This may
/// include lack of tags on a POI node or a way. This is not an
/// exhaustive test, mostly just a fast sanity-check.

// JOSM validator
//   Crossing ways
//   Duplicate Ways
//   Duplicate nodes
//   Duplicate relations
//   Duplicated way nodes
//   Orphan nodes
//   No square building corners

// OSMInspector
//   Empty tag key
//   Unknown highway type
//   Single node way
//   Interescting ways

// OSMose
//   Overlapping buildings
//   orphan nodes
//   Duplicate geomtry
//   Highway not connected
//   Missing tags
//   Duplicate object
//   
//

typedef enum {notags, complete, incomplete, badvalue, correct } valerror_t;

class ValidateStatus {
public:
    ValidateStatus(void) {};
    ValidateStatus(const osmobjects::OsmNode &node) {
	osm_id = node.id;
	user_id = node.uid;
	change_id = node.change_id;
	objtype = osmobjects::node;
	timestamp = node.timestamp;
    }
    ValidateStatus(const osmobjects::OsmWay &way) {
	osm_id = way.id;
	user_id = way.uid;
	change_id = way.change_id;
	objtype = osmobjects::way;
	timestamp = way.timestamp;
    }
    //valerror_t operator[](int index){ return status[index]; };
    bool hasStatus(const valerror_t &val) {
	auto match = std::find(status.begin(), status.end(), val);
	if (match != status.end()) {
	    return true;
	}
	return false;
    }
    std::unordered_set<valerror_t> status;
    osmobjects::osmtype_t objtype;
    long osm_id = 0;
    long user_id = 0;
    long change_id = 0;
    ptime timestamp;
    point_t center;

    void dump(void) {
	std::cerr << "Dumping Validation Statistics" << std::endl;
	std::cerr << "\tOSM ID: " << osm_id << std::endl;
	std::cerr << "\tUser ID: " << user_id << std::endl;
	std::cerr << "\tChange ID: " << change_id << std::endl;

	std::map<valerror_t, std::string> results;
	results[notags] = "No tags";
	results[complete] = "Tags are complete";
	results[incomplete] = "Tags are incomplete";
	results[badvalue] = "Bad tag value";
	results[correct] = "Correct tag value";
	for (auto it = std::begin(status); it != std::end(status); ++it) {
	    std::cerr << "\tResult: " << results[*it] << std::endl;
	}
    }
};

class BOOST_SYMBOL_VISIBLE Validate
{
public:
    Validate(void) {
	std::string dir = SRCDIR;
	if (!boost::filesystem::exists(dir)) {
	    dir = PKGLIBDIR;
	    if (!boost::filesystem::exists(dir)) {
		log_error(_("No validation config files in %1%!"), dir);
	    }
	}
	for (auto& file : std::filesystem::recursive_directory_iterator(dir)) {
	    std::filesystem::path config = file.path();
	    if (config.extension() == ".yaml") {
		log_debug("Loading: %s", config.stem());
		yaml::Yaml yaml;
		yaml.read(config.string());
		if (!config.stem().empty()) {
		    yamls[config.stem()] = yaml;
		}
	    }
	}
#if 0
	Validate(const std::string &filespec) {
	    yaml::Yaml yaml;
	    yaml.read(filespec);
	    if (!config.stem().empty()) {
		yamls[config.stem()] = yaml;
	    }
	}
#endif
    };
    virtual ~Validate(void) {};
    // Validate(std::vector<std::shared_ptr<osmchange::OsmChange>> &changes) {};

    /// Check a POI for tags. A node that is part of a way shouldn't have any
    /// tags, this is to check actual POIs, like a school.
    virtual std::shared_ptr<ValidateStatus> checkPOI(const osmobjects::OsmNode &node, const std::string &type) = 0;

    /// This checks a way. A way should always have some tags. Often a polygon
    /// is a building
    virtual std::shared_ptr<ValidateStatus> checkWay(const osmobjects::OsmWay &way, const std::string &type) = 0;
    std::shared_ptr<ValidateStatus> checkTags (std::map<std::string, std::string> tags) {
        bool result;
        for (auto it = std::begin(tags); it != std::end(tags); ++it) {
	    // FIXME: temporarily disabled
            // result = checkTag(it->first, it->second);
        }
        // return result;
    };
    void dump(void) {
	for (auto it = std::begin(yamls); it != std::end(yamls); ++it) {
	    it->second.dump();
	}
    }
    virtual std::shared_ptr<ValidateStatus> checkTag(const std::string &key, const std::string &value) = 0;
    yaml::Yaml &operator[](const std::string &key) { return yamls[key]; };
  protected:
    std::map<std::string, yaml::Yaml> yamls;
};

#endif  // EOF __VALIDATE_HH__

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
