//
// Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
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

#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <valarray>
#include <locale>

#include <boost/dll/alias.hpp>
#include <boost/function.hpp>
#include <boost/config.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include "utils/yaml.hh"
#include "semantic.hh"
#include "validate.hh"
#include "osm/osmchange.hh"
#include "utils/log.hh"

using namespace logger;

// This plugin checks for issues with tags
// [*] Required tags
// [*] Bad values
// [*] Empty value
// [*] No tags

namespace semantic {

LogFile &dbglogfile = LogFile::getDefaultInstance();

Semantic::Semantic(void) {}

bool Semantic::isValidTag(const std::string &key, const std::string &value, yaml::Node tags) {
    if (tags.contains_value(key, value) ||
        (tags.get(key).children.size() == 0 && tags.contains_key(key))) {
            return true;
    }
    log_debug("Bad tag: %1%=%2%", key, value);
    return false;
}

bool Semantic::isRequiredTag(const std::string &key, yaml::Node required_tags) {
    if (required_tags.children.size() > 0 && required_tags.contains_key(key)) {
        log_debug("Required tag: %1%", key);
        return true;
    }
    return false;
}

// Check a POI for tags. A node that is part of a way shouldn't have any
// tags, this is to check actual POIs, like a school.
std::shared_ptr<ValidateStatus>
Semantic::checkPOI(const osmobjects::OsmNode &node, const std::string &type)
{
    auto status = std::make_shared<ValidateStatus>(node);
    status->timestamp = boost::posix_time::microsec_clock::universal_time();
    status->uid = node.uid;

    if (yamls.size() == 0) {
        log_error("No config files!");
        return status;
    }
    if (node.tags.size() == 0) {
        status->status.insert(notags);
        return status;
    }
    if (node.action == osmobjects::remove) {
        return status;
    }

    // Configuration for validation
    yaml::Yaml tests = yamls[type];
    // List of valid tags to be validated
    auto tags = tests.get("tags");

    // Not using required_tags disables writing features flagged for not being tag complete
    // from being written to the database thus reducing the size of the results.
    auto required_tags = tests.get("required_tags");

    std::string key;
    int tagexists = 0;
    status->center = node.point;

    for (auto vit = std::begin(node.tags); vit != std::end(node.tags); ++vit) {
        if (!isValidTag(vit->first, vit->second, tags)) {
            status->status.insert(badvalue);
            status->values.insert(vit->first + "=" +  vit->second);
        }
        if (isRequiredTag(vit->first, required_tags)) {
            tagexists++;
        }
        checkTag(vit->first, vit->second);
    }

    if (tagexists == required_tags.children.size()) {
        status->status.insert(complete);
    } else {
        status->status.insert(incomplete);
    }
    if (status->status.size() == 0) {
        status->status.insert(correct);
    }
    return status;
}

// This checks a way. A way should always have some tags. Often a polygon
// with no tags is a building.
std::shared_ptr<ValidateStatus>
Semantic::checkWay(const osmobjects::OsmWay &way, const std::string &type)
{
    // On non-english numeric locales using decimal separator different than '.'
    // this is necessary to parse double strings with std::stod correctly
    // without loosing precision
    // std::setlocale(LC_NUMERIC, "C");
    setlocale(LC_NUMERIC, "C");

    auto status = std::make_shared<ValidateStatus>(way);
    status->timestamp = boost::posix_time::microsec_clock::universal_time();
    status->uid = way.uid;

    if (yamls.size() == 0) {
        log_error("No config files!");
        return status;
    }
    if (way.tags.size() == 0) {
        status->status.insert(notags);
        return status;
    }
    if (way.action == osmobjects::remove) {
        return status;
    }

    yaml::Yaml tests = yamls[type];

    std::string key;
    // List of valid tags to be validated
    auto tags = tests.get("tags");

    // These values are in the config section of the YAML file
    auto config = tests.get("config");

    // Not using required_tags disables writing features flagged for not being tag complete
    // from being written to the database thus reducing the size of the results.
    auto required_tags = tests.get("required_tags");

    // This enables/disables writing features flagged for not having values
    // in range as defined in the YAML config file. This prevents those
    // from being written to the database to reduce the size of the results.

    int tagexists = 0;
    for (auto vit = std::begin(way.tags); vit != std::end(way.tags); ++vit) {
            if (!isValidTag(vit->first, vit->second, tags)) {
                status->status.insert(badvalue);
                status->values.insert(vit->first + "=" +  vit->second);
            }
            if (isRequiredTag(vit->first, required_tags)) {
                tagexists++;
            }
            checkTag(vit->first, vit->second);
    }

    if (tagexists == required_tags.children.size()) {
        status->status.insert(complete);
    } else {
        status->status.insert(incomplete);
    }

    return status;
}

// Check a tag for typical errors
std::shared_ptr<ValidateStatus>
Semantic::checkTag(const std::string &key, const std::string &value)
{
    auto status = std::make_shared<ValidateStatus>();
    // log_trace("Semantic::checkTag(%1%, %2%)", key, value);
    // Check for an empty value
    if (!key.empty() && value.empty()) {
        log_debug("WARNING: empty value for tag \"%1%\"", key);
        status->status.insert(badvalue);
    }
    // Check for a space in the tag key
    if (key.find(' ') != std::string::npos) {
        log_error("WARNING: spaces in tag key \"%1%\"", key);
        status->status.insert(badvalue);
    }
    // Check for single quotes in the tag value
    if (value.find('\'') != std::string::npos) {
        log_error("WARNING: single quote in tag value \"%1%\"", value);
        status->status.insert(badvalue);
    }
    // Check for single quotes in the tag value
    if (value.find('\"') != std::string::npos) {
        log_error("WARNING: double quote in tag value \"%1%\"", value);
        status->status.insert(badvalue);
    }
    return status;
}

// Check a OSM Change for typical errors
std::vector<ValidateStatus>
Semantic::checkOsmChange(const std::string &xml, const std::string &check) {
    osmchange::OsmChangeFile ocf;
    std::stringstream _xml(xml);
    ocf.readXML(_xml);
    std::vector<ValidateStatus> result;
    for (auto it = std::begin(ocf.changes); it != std::end(ocf.changes); ++it) {
        osmchange::OsmChange *change = it->get();
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            if (!way->containsKey(check)) {
                continue;
            }
            auto status = checkWay(*way, check);
            result.push_back(*status);
        }
        for (auto nit = std::begin(change->nodes); nit != std::end(change->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
            if (!node->containsKey(check)) {
                continue;
            }
            auto status = checkPOI(*node, check);
            result.push_back(*status);
        }
    }
    return result;
}

}; // namespace semantic

#endif // EOF __SEMANTIC_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
