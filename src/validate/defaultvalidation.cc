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

#ifndef __DEFAULTVALIDATION_H__
#define __DEFAULTVALIDATION_H__

#include <memory>
#include "utils/yaml.hh"
#include "defaultvalidation.hh"
#include "geospatial.hh"
#include "semantic.hh"
#include "validate.hh"
#include "osm/osmchange.hh"
#include "utils/log.hh"

using namespace logger;

namespace defaultvalidation {

LogFile &dbglogfile = LogFile::getDefaultInstance();

DefaultValidation::DefaultValidation(void) {}

// Check a POI for tags. A node that is part of a way shouldn't have any
// tags, this is to check actual POIs, like a school.
std::shared_ptr<ValidateStatus>
DefaultValidation::checkNode(const osmobjects::OsmNode &node, const std::string &type)
{
    auto status = std::make_shared<ValidateStatus>(node);
    status->timestamp = boost::posix_time::microsec_clock::universal_time();
    status->uid = node.uid;
    if (yamls.size() == 0) {
        log_error("No config files!");
        return status;
    }
    yaml::Yaml tests = yamls[type];
    semantic::Semantic::checkNode(node, type, tests, status);

    return status;
}

// This checks a way. A way should always have some tags. Often a polygon
// with no tags is a building.
std::shared_ptr<ValidateStatus>
DefaultValidation::checkWay(const osmobjects::OsmWay &way, const std::string &type)
{
    auto status = std::make_shared<ValidateStatus>(way);
    status->timestamp = boost::posix_time::microsec_clock::universal_time();
    status->uid = way.uid;
    if (yamls.size() == 0) {
        log_error("No config files!");
        return status;
    }
    yaml::Yaml tests = yamls[type];
    semantic::Semantic::checkWay(way, type, tests, status);
    geospatial::Geospatial::checkWay(way, type, tests, status);
    if (way.linestring.size() > 2) {
        boost::geometry::centroid(way.linestring, status->center);
    }
    status->source = type;
    return status;
}

}; // namespace defaultvalidation

#endif // EOF __DEFAULTVALIDATION_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
