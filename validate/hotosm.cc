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

#ifndef __HOTOSM_H__
#define __HOTOSM_H__

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

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
    yaml::Yaml yaml;
    yaml.read("/home/rob/projects/HOT/underpass.git/validate/validate/buildings.yaml");
    // yaml.dump();
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
bool
Hotosm::checkPOI(osmobjects::OsmNode *node)
{
    if (node->tags.size() == 0) {
        log_error(_("WARNING: POI %1% has no tags!"), node->id);
        node_errors.push_back(node->id);
        return false;
    }

    return true;
}

// This checks a way. A way should always have some tags. Often a polygon
// with no tags is a building.
bool
Hotosm::checkWay(osmobjects::OsmWay *way)
{
    bool result = true;
    for (auto it = std::begin(way->tags); it != std::end(way->tags); ++it) {
        result = checkTag(it->first, it->second);
        if (!result) {
            return result;
        }
    }

    if (way->numPoints() == 5 && way->isClosed() && way->tags.size() == 0) {
        log_error(_("WARNING: %1% might be a building!"), way->id);
        buildings.push_back(way->id);
        return false;
    }
    
    return true;
}

// Check a tag for typical errors
bool
Hotosm::checkTag(const std::string &key, const std::string &value)
{
    log_trace("Hotosm::checkTag(%1%, %2%)", key, value);
    // Check for an empty value
    if (!key.empty() && value.empty()) {
        log_debug(_("WARNING: empty value for tag \"%1%\""), key);
        return true;
    }
    // Check for a space in the tag key
    if (key.find(' ') != std::string::npos) {
        log_error(_("WARNING: spaces in tag key \"%1%\""), key);
        return false;
    }
    // Check for single quotes in the tag value
    if (value.find('\'') != std::string::npos) {
        log_error(_("WARNING: single quote in tag value \"%1%\""), value);
        return false;
    }
    // Check for single quotes in the tag value
    if (value.find('\"') != std::string::npos) {
        log_error(_("WARNING: double quote in tag value \"%1%\""), value);
        return false;
    }

    return true;
}

};      // EOF hotosm namespace

#endif  // EOF __HOTOSM_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
