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

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
#include "unconfig.h"
#endif

#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1

#include <boost/python.hpp>
#include "validate/hotosm.hh"
#include "validate/validate.hh"
#include "data/osmobjects.hh"
#include "validate/conflate.hh"
#include "galaxy/osmchange.hh"
#include "data/pq.hh"

#include "log.hh"
using namespace logger;

#ifdef USE_PYTHON

using namespace boost::python;
using namespace osmobjects;

std::map<valerror_t, std::string> results = {
    {notags, "notags"},
    {complete, "complete"},
    {incomplete, "incomplete"},
    {badvalue, "badvalue"},
    {correct, "correct"},
    {badgeom, "badgeom"},
    {orphan, "orphan"},
    {overlaping, "overlaping"},
    {duplicate, "duplicate"}
};

ValidateStatus* checkPOI(hotosm::Hotosm& self, const osmobjects::OsmNode &node, const std::string &type) {
    auto _v = self.checkPOI(node, type);
    ValidateStatus* v = new ValidateStatus();
    v->status = _v->status;
    return v;
}

ValidateStatus* checkWay(hotosm::Hotosm& self, const osmobjects::OsmWay &way, const std::string &type) {
    auto _v = self.checkWay(way, type);
    ValidateStatus* v = new ValidateStatus();
    v->status = _v->status;
    return v;
}

std::string dumpJSON(ValidateStatus& self) {
    std::string output = "";
    output += "{\n";
    output += "\t\"osm_id\":" + std::to_string(self.osm_id) + ",\n";
    output += "\t\"user_id\":" + std::to_string(self.user_id) + ",\n";
    output += "\t\"change_id\":" + std::to_string(self.change_id) + ",\n";
    output += "\t\"angle\":" + std::to_string(self.angle) + ",\n";

    if (self.status.size() > 0) {
        output += "\t\"results\": [";
        for (const auto &stat: std::as_const(self.status)) {
            output += "\"" + results[stat] + "\",";
        }
        output.erase(output.size() - 1);
        output += "]";
    }

    if (self.values.size() > 0) {
        output += ",\n\t\"values\": [";
        for (auto it = std::begin(self.values); it != std::end(self.values); ++it ) {
            output += "\"" + *it + "\",";
        }
        output.erase(output.size() - 1);
        output += "]\n";
    }
    output += "}\n";
    return output;
}

std::string checkOsmChange(hotosm::Hotosm& self, const std::string &xml, const std::string &check) {
    auto result = self.checkOsmChange(xml, check);
    std::string output = "[ ";
    for (auto it = std::begin(result); it != std::end(result); ++it) {
        if ((*it).status.size() > 0) {
            output += dumpJSON(*it) + ",";
        }
    }
    output.erase(output.size() - 1);
    output += " ]";
    return output;
}

BOOST_PYTHON_MODULE(underpass)
{

    using namespace conflate;
    class_<OsmNode>("OsmNode")
        .def("setLatitude", &OsmNode::setLatitude)
        .def("setLongitude", &OsmNode::setLongitude)
        .def("setPoint", &OsmNode::setPoint)
        .def("addTag", &OsmNode::addTag)
        .def("dump", &OsmNode::dump);
    class_<OsmWay>("OsmWay")
        .def("isClosed", &OsmWay::isClosed)
        .def("numPoints", &OsmWay::numPoints)
        .def("getLength", &OsmWay::getLength)
        .def("addRef", &OsmWay::addRef)
        .def("addTag", &OsmWay::addTag)
        .def("dump", &OsmWay::dump);
    class_<OsmRelation>("OsmRelation")
        .def("addMember", &OsmRelation::addMember)
        .def("dump", &OsmRelation::dump);

    // 
    using namespace hotosm;
    class_<Hotosm, boost::noncopyable>("Validate")
        .def("checkTag", &Hotosm::checkTag)
        .def("checkWay", &checkWay, boost::python::return_value_policy<boost::python::manage_new_object>())
        .def("checkPOI", &checkPOI, boost::python::return_value_policy<boost::python::manage_new_object>())
        .def("overlaps", &Hotosm::overlaps)
        .def("checkOsmChange", &checkOsmChange);

    class_<ValidateStatus, boost::noncopyable>("ValidateStatus")
        .def("hasStatus", &ValidateStatus::hasStatus)
        .def("dump", &dumpJSON);

    using namespace osmchange;
    class_<OsmChangeFile, boost::noncopyable>("OsmChangeFile")
        .def("readChanges", &OsmChangeFile::readChanges)
        .def("dump", &OsmChangeFile::dump);

#if 1
    class_<Conflate>("Conflate")
        .def("connect", &Conflate::connect)
        .def("newDuplicatePolygon", &Conflate::newDuplicatePolygon)
        .def("existingDuplicatePolygon", &Conflate::existingDuplicatePolygon)
        .def("newDuplicateLineString", &Conflate::newDuplicateLineString)
        .def("existingDuplicateLineString", &Conflate::existingDuplicateLineString);
#endif
}
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
