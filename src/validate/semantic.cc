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

#include <boost/function.hpp>
#include <boost/config.hpp>
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

Semantic::Semantic() {}

// Check a tag for typical errors

void
Semantic::checkTag(const std::string &key, const std::string &value, std::shared_ptr<ValidateStatus> &status)
{
    // log_trace("Semantic::checkTag(%1%, %2%)", key, value);
    // Check for an empty value
    if (!key.empty() && value.empty()) {
        log_debug("WARNING: empty value for tag \"%1%\"", key);
        status->status.insert(badvalue);
        status->values.insert(key + "=" + value);
    }
    // Check for a space in the tag key
    if (key.find(' ') != std::string::npos) {
        log_error("WARNING: spaces in tag key \"%1%\"", key);
        status->status.insert(badvalue);
        status->values.insert(key + "=" + value);
    }
    // Check for single quotes in the tag value
    if (key.find('\'') != std::string::npos) {
        log_error("WARNING: single quote in tag key \"%1%\"", value);
        status->status.insert(badvalue);
        status->values.insert(key + "=" + value);
    }
    // Check for double quotes in the tag key
    if (key.find('\"') != std::string::npos) {
        log_error("WARNING: double quote in tag key \"%1%\"", value);
        status->status.insert(badvalue);
        status->values.insert(key + "=" + value);
    }
    // Check for a underscore at the beginning of the tag key
    if(key.at(0) == '_') {
        log_error("WARNING: underscore at the beginning of the tag key \"%1%\"", key);
        status->status.insert(badvalue);
        status->values.insert(key + "=" + value);
    }
}

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
Semantic::checkNode(const osmobjects::OsmNode &node, const std::string &type, yaml::Yaml &tests, std::shared_ptr<ValidateStatus> &status)
{

    auto config = tests.get("config");
    bool check_badvalue = config.get_value("badvalue") == "yes";
    bool check_incomplete = config.get_value("incomplete") == "yes";

    if (node.tags.size() == 0) {
        status->status.insert(notags);
        return status;
    }
    if (node.action == osmobjects::remove) {
        return status;
    }

    // List of valid tags to be validated
    auto tags = tests.get("tags");

    // Not using required_tags disables writing features flagged for not being tag complete
    // from being written to the database thus reducing the size of the results.
    auto required_tags = tests.get("required_tags");

    std::string key;
    int tagexists = 0;
    status->center = node.point;

    if (node.tags.count(type)) {
        for (auto vit = std::begin(node.tags); vit != std::end(node.tags); ++vit) {
            if (check_badvalue) {
                if (!isValidTag(vit->first, vit->second, tags)) {
                    status->status.insert(badvalue);
                    status->values.insert(vit->first + "=" +  vit->second);
                }
            }
            if (check_incomplete) {
                if (isRequiredTag(vit->first, required_tags)) {
                    tagexists++;
                }
            }
            checkTag(vit->first, vit->second, status);
        }

        if (check_incomplete) {
            if (tagexists != required_tags.children.size()) {
                status->status.insert(incomplete);
            }
        }
    }
    return status;
}

// This checks a way. A way should always have some tags. Often a polygon
// with no tags is a building.
std::shared_ptr<ValidateStatus>
Semantic::checkWay(const osmobjects::OsmWay &way, const std::string &type, yaml::Yaml &tests, std::shared_ptr<ValidateStatus> &status)
{
    if (way.action == osmobjects::remove) {
        return status;
    }

    // These values are in the config section of the YAML file
    auto config = tests.get("config");
    bool check_badvalue = config.get_value("badvalue") == "yes";
    bool check_incomplete = config.get_value("incomplete") == "yes";

    // List of required tags to be validated
    yaml::Node required_tags;
    if (check_incomplete) {
        required_tags = tests.get("required_tags");
    } 

    // List of valid tags to be validated
    yaml::Node tags;
    if (check_badvalue) {
        tags = tests.get("tags");
    }

    if (check_badvalue && way.tags.size() == 0) {
        status->status.insert(notags);
        return status;
    }

    int tagexists = 0;
    if (way.tags.count(type)) {
        for (auto vit = std::begin(way.tags); vit != std::end(way.tags); ++vit) {
            if (check_badvalue) {
                if (tags.children.size() > 0 && !isValidTag(vit->first, vit->second, tags)) {
                    status->status.insert(badvalue);
                    status->values.insert(vit->first + "=" +  vit->second);
                }
                checkTag(vit->first, vit->second, status);
            }
            if (check_incomplete) {
                if (isRequiredTag(vit->first, required_tags)) {
                    tagexists++;
                }
            }
        }

        if (check_incomplete && tagexists != required_tags.children.size()) {
            status->status.insert(incomplete);
        }
    }
    return status;
}

// This checks a relation.
std::shared_ptr<ValidateStatus>
Semantic::checkRelation(const osmobjects::OsmRelation &relation, const std::string &type, yaml::Yaml &tests, std::shared_ptr<ValidateStatus> &status)
{
    if (relation.action == osmobjects::remove) {
        return status;
    }

    // These values are in the config section of the YAML file
    auto config = tests.get("config");
    bool check_badvalue = config.get_value("badvalue") == "yes";
    bool check_incomplete = config.get_value("incomplete") == "yes";

    // List of required tags to be validated
    yaml::Node required_tags;
    if (check_incomplete) {
        required_tags = tests.get("required_tags");
    } 

    // List of valid tags to be validated
    yaml::Node tags;
    if (check_badvalue) {
     tags = tests.get("tags");
    }

    if (check_badvalue && relation.tags.size() == 0) {
        status->status.insert(notags);
        return status;
    }

    int tagexists = 0;
    if (relation.tags.count(type)) {
        for (auto vit = std::begin(relation.tags); vit != std::end(relation.tags); ++vit) {
            if (check_badvalue) {
                if (tags.children.size() > 0 && !isValidTag(vit->first, vit->second, tags)) {
                    status->status.insert(badvalue);
                    status->values.insert(vit->first + "=" +  vit->second);
                }
                checkTag(vit->first, vit->second, status);
            }
            if (check_incomplete) {
                if (isRequiredTag(vit->first, required_tags)) {
                    tagexists++;
                }
            }
        }

        if (check_incomplete && tagexists != required_tags.children.size()) {
            status->status.insert(incomplete);
        }
    }
    return status;
}

}; // namespace semantic

#endif // EOF __SEMANTIC_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
