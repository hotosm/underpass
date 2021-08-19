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

#ifndef __HOTOSM_H__
#define __HOTOSM_H__

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <iterator>

#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/config.hpp>

#include "data/yaml.hh"
#include "hotosm.hh"
#include "validate.hh"
#include "osmstats/osmchange.hh"
#include "log.hh"
#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>

using namespace logger;

namespace hotosm {

LogFile& dbglogfile = LogFile::getDefaultInstance();

// FIXME: things to look for
// Villages, hamlets, neigborhoods, towns, or cities added without a name

Hotosm::Hotosm(void)
{
}

#if 0
Hotosm::Hotosm(std::vector<std::shared_ptr<osmchange::OsmChange>> &changes)
{
    for (auto it = std::begin(changes); it != std::end(changes); ++it) {
        osmchange::OsmChange *change = it->get();
        change->dump();
        if (change->action == osmobjects::create) {
            for (auto it = std::begin(change->nodes); it != std::end(change->nodes); ++it) {
                osmobjects::OsmNode *node = it->get();
                if (node->tags.size() > 0) {
                    log_debug(_("Validating New Node ID %1% has tags"), node->id);
                    checkPOI(node);
                } else {
                    continue;
                }
            // for (auto it = std::begin(change->ways); it != std::end(change->ways); ++it) {
            //     OsmWay *way = it->get();
            //     if (way->tags.size() == 0) {
            //         log_error(_("Validating New Way ID %1% has no tags"), way->id);
            //         checkWay(way);
            //     } else {
            //         continue;
            //     }
            // }
            }       
        }
    }
}
#endif

// Check a POI for tags. A node that is part of a way shouldn't have any
// tags, this is to check actual POIs, like a school.
std::shared_ptr<ValidateStatus>
Hotosm::checkPOI(const osmobjects::OsmNode &node, const std::string &type)
{
    auto status = std::make_shared<ValidateStatus>(node);
    status->status.clear();
    if (yamls.size() == 0) {
	log_error(_("No config files!"));
	return status;
    }
    if (node.tags.size() == 0) {
	status->status.push_back(notags);
	return status;
    }

    yaml::Yaml tests = yamls[type];
    int count = 0;
    std::string key;
    int keyexists = 0;
    int valexists = 0;

    for (auto vit = std::begin(node.tags); vit != std::end(node.tags); ++vit) {
	if (tests.containsKey(vit->first)) {
	    // std::cerr << "Matched key " << vit->first << "!" << std::endl;
	    keyexists++;
	} else {
	    status->status.push_back(incomplete);
	}
	if (tests.containsValue(vit->first, vit->second)) {
	    // std::cerr << "Matched value: " << vit->second << "\t" << "!" << std::endl;
	    valexists++;
	    status->status.push_back(correct);
	} else {
	    status->status.push_back(badvalue);
	}
    }
    // std::cerr << keyexists << " : " << valexists << " : " << tests.config.size() << std::endl;

    if (keyexists == tests.config.size() && valexists == tests.config.size()) {
	status->status.clear();
	status->status.push_back(complete);
    } else {
	status->status.push_back(incomplete);
    }
    return status;
}

// This checks a way. A way should always have some tags. Often a polygon
// with no tags is a building.
std::shared_ptr<ValidateStatus>
Hotosm::checkWay(const osmobjects::OsmWay &way, const std::string &type)
{
    auto status = std::make_shared<ValidateStatus>(way);

    for (auto it = std::begin(way.tags); it != std::end(way.tags); ++it) {
	// FIXME: temporarily disabled
        // result = checkTag(it->first, it->second);
        //if (!result) {
        //    return status;
	// }
    }

    if (way.refs.size() == 5 && way.tags.size() == 0) {
    // if (way.numPoints() == 5 && way.isClosed() && way.tags.size() == 0) {
	log_error(_("WARNING: %1% might be a building!"), way.id);
	buildings.push_back(way.id);
	return status;
    }
    
    return status;
}

// Check a tag for typical errors
std::shared_ptr<ValidateStatus>
Hotosm::checkTag(const std::string &key, const std::string &value)
{
    auto status = std::make_shared<ValidateStatus>();

    log_trace("Hotosm::checkTag(%1%, %2%)", key, value);
    // Check for an empty value
    if (!key.empty() && value.empty()) {
        log_debug(_("WARNING: empty value for tag \"%1%\""), key);
        return status;
    }
    // Check for a space in the tag key
    if (key.find(' ') != std::string::npos) {
        log_error(_("WARNING: spaces in tag key \"%1%\""), key);
        return status;
    }
    // Check for single quotes in the tag value
    if (value.find('\'') != std::string::npos) {
        log_error(_("WARNING: single quote in tag value \"%1%\""), value);
        return status;
    }
    // Check for single quotes in the tag value
    if (value.find('\"') != std::string::npos) {
        log_error(_("WARNING: double quote in tag value \"%1%\""), value);
        return status;
    }

    return status;
}

};      // EOF hotosm namespace

#endif  // EOF __HOTOSM_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
