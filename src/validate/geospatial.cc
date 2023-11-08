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

#ifndef __GEOSPATIAL_H__
#define __GEOSPATIAL_H__

#include <memory>
#include <string>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include "utils/yaml.hh"
#include "geospatial.hh"
#include "validate.hh"
#include "osm/osmchange.hh"
#include "utils/log.hh"

using namespace logger;

// This plugin checks for geospatial issues
// [*] Bad geometry
// [ ] Overlapping
// [ ] Duplicates
// [ ] Un-connected

namespace geospatial {

LogFile &dbglogfile = LogFile::getDefaultInstance();

Geospatial::Geospatial() {}

// This checks a way. A way should always have some tags. Often a polygon
// with no tags is a building.
std::shared_ptr<ValidateStatus>
Geospatial::checkWay(const osmobjects::OsmWay &way, const std::string &type, yaml::Yaml &tests, std::shared_ptr<ValidateStatus> &status)
{
    if (way.action == osmobjects::remove) {
        return status;
    }

    auto config = tests.get("config");
    bool check_badgeom = config.get_value("badgeom") == "yes";    
    // bool check_overlapping = config.get_value("overlapping") == "yes";
    // bool check_duplicate = config.get_value("duplicate") == "yes";

    if (check_badgeom) {
        auto badgeom_minangle = config.get_value("badgeom_minangle");
        auto badgeom_maxangle = config.get_value("badgeom_maxangle");
        if (badgeom_minangle != "" && badgeom_maxangle != "") {
            if (unsquared(way.linestring, std::stod(badgeom_minangle), std::stod(badgeom_maxangle))) {
                status->status.insert(badgeom);
            }
        } else {
            if (unsquared(way.linestring)) {
                status->status.insert(badgeom);
            }
        }

    }

    // if (check_overlapping) {
        // auto allWays = context.getOverlappingWays();
        // if (overlaps(allWays, way)) {
        //     status->status.insert(overlapping);
        // }
    // }

    // if (check_duplicate) {
        // auto allWays = context.getOverlappingWays();
        // if (overlaps(allWays, way)) {
        //     status->status.insert(overlapping);
        // }
    // }

    return status;
}

bool 
Geospatial::overlaps(const std::list<std::shared_ptr<osmobjects::OsmWay>> &allways, osmobjects::OsmWay &way) {
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("validate::overlaps: took %w seconds\n");
#endif
    if (way.numPoints() <= 1) {
        return false;
    }
    for (auto nit = std::begin(allways); nit != std::end(allways); ++nit) {
        osmobjects::OsmWay *oldway = nit->get();
        if (boost::geometry::overlaps(oldway->polygon, way.polygon)) {
            if (way.getTagValue("layer") == oldway->getTagValue("layer") && way.id != oldway->id) {
                log_error("Building %1% overlaps with %2%", way.id, oldway->id);
                return true;
            }
        }
    }
    return false;
}

bool 
Geospatial::duplicate(const std::list<std::shared_ptr<osmobjects::OsmWay>> &allways, osmobjects::OsmWay &way) {
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("validate::duplicate: took %w seconds\n");
#endif
    if (way.numPoints() <= 1) {
        return false;
    }
    for (auto nit = std::begin(allways); nit != std::end(allways); ++nit) {
        osmobjects::OsmWay *oldway = nit->get();
        std::deque<polygon> output;
        bg::intersection(oldway->polygon, way.polygon, output);
        double iarea = 0;
        for (auto& p : output)
            iarea += bg::area(p);
        double wayarea = bg::area(way.polygon);
        double iareapercent = (iarea * 100) / wayarea;
        if (iareapercent >= 80) {
            if (way.getTagValue("layer") == oldway->getTagValue("layer") && way.id != oldway->id) {
                log_error("Building %1% duplicate %2%", way.id, oldway->id);
                return true;
            }
        }
    }
    return false;
}

bool 
Geospatial::unsquared(
    const linestring_t &way,
    double min_angle,
    double max_angle
) {
    const int num_points =  boost::geometry::num_points(way);
    double last_angle = -1;
    double max_angle_diff = 0;
    bool unsquared = false;
    for(int i = 0; i < num_points - 1; i++) {
        // Three points
        int a,b,c;
        if (i < num_points - 3) {
            a = i;
            b = i + 1;
            c = i + 2;
        } else if (i == num_points - 3) {
            a = i;
            b = i + 1;
            c = 0;
        } else {
            a = i;
            b = 0;
            c = 1;
        }
        double x1 = boost::geometry::get<0>(way[a]);
        double y1 = boost::geometry::get<1>(way[a]);
        double x2 = boost::geometry::get<0>(way[b]);
        double y2 = boost::geometry::get<1>(way[b]);
        double x3 = boost::geometry::get<0>(way[c]);
        double y3 = boost::geometry::get<1>(way[c]);

        double angle = geo::Geo::calculateAngle(x1,y1,x2,y2,x3,y3);
        if (last_angle != -1) {
            double diff = abs(angle - last_angle);
            if (diff > max_angle_diff) {
                max_angle_diff = diff;
            }
        }
        last_angle = angle;

        if (
            (angle > max_angle || angle < min_angle) &&
            (angle < 179 || angle > 181)
        ) {
            unsquared = true;
        }
    }
    if (unsquared && !(num_points > 5 && max_angle_diff < 3)) {
        return true;
    }
    return false;
};

}; // namespace geospatial

#endif // EOF __SEMANTIC_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
