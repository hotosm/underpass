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

/// \file queryvalidate.hh
/// \brief This file is used to work with the OSM Validation database
///
/// This manages the OSM Validation schema in a postgres database. This
/// includes querying existing data in the database, as well as
/// updating the database.

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
#include "unconfig.h"
#endif

#include <array>
#include <assert.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/date_time.hpp>
#include <boost/locale.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/format.hpp>
#include <boost/math/special_functions/relative_difference.hpp>

using namespace boost::math;
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "osm/osmobjects.hh"
#include "utils/log.hh"
#include "osm/changeset.hh"
#include "validate/queryvalidate.hh"
#include "validate/validate.hh"
#include "data/pq.hh"
using namespace pq;

using namespace logger;

/// \namespace queryvalidate
namespace queryvalidate {

std::map<valerror_t, std::string> status_list = {
    {notags, "notags"},
    {complete, "complete"},
    {incomplete, "incomplete"},
    {badvalue, "badvalue"},
    {correct, "correct"},
    {orphan, "orphan"},
    {overlaping, "overlaping"},
    {duplicate, "duplicate"},
    {badgeom, "badgeom"}
};

std::map<osmobjects::osmtype_t, std::string> objtypes = {
    {osmobjects::empty, "empty"},
    {osmobjects::node, "node"},
    {osmobjects::way, "way"},
    {osmobjects::relation, "relation"}
};

QueryValidate::QueryValidate(void) {}

QueryValidate::QueryValidate(std::shared_ptr<Pq> db) {
    dbconn = db;
}

std::string
QueryValidate::updateValidation(std::shared_ptr<std::vector<long>> removals)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("updateValidation: took %w seconds\n");
#endif
    std::string query = "";
    if (removals->size() > 0) {
        query = "DELETE FROM validation WHERE osm_id IN(";
        for (const auto &osm_id : *removals) {
            query += std::to_string(osm_id) + ",";
        };
        query.erase(query.size() - 1);
        query += ");";
    }
    return query;
}

std::string
QueryValidate::updateValidation(long osm_id, const valerror_t &status, const std::string &source, long version) const
{
    std::string format = "DELETE FROM validation WHERE osm_id = %d and source = '%s' and status = '%s' and version <= %d;";
    boost::format fmt(format);
    fmt % osm_id;
    fmt % source;
    fmt % status_list[status];
    fmt % version;
    std::string query = fmt.str();
    return query;
}

std::string
QueryValidate::applyChange(const ValidateStatus &validation, const valerror_t &status) const
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("applyChange(validation): took %w seconds\n");
#endif
    log_debug("Applying Validation data");

    std::string format;
    std::string query;

    if (validation.values.size() > 0) {
        query = "INSERT INTO validation (osm_id, change_id, user_id, type, status, values, timestamp, location, source, version) VALUES(";
        format = "%d, %d, %g, \'%s\', \'%s\', ARRAY[%s], \'%s\', ST_GeomFromText(\'%s\', 4326), \'%s\', %s) ";
    } else {
        query = "INSERT INTO validation (osm_id, change_id, user_id, type, status, timestamp, location, source, version) VALUES(";
        format = "%d, %d, %g, \'%s\', \'%s\', \'%s\', ST_GeomFromText(\'%s\', 4326), \'%s\', %s) ";
    }
    format += "ON CONFLICT (osm_id, status, source) DO UPDATE SET version = %d,  timestamp = \'%s\' WHERE excluded.version < %d;";
    boost::format fmt(format);
    fmt % validation.osm_id;
    fmt % validation.change_id;
    fmt % validation.user_id;
    fmt % objtypes[validation.objtype];
    std::string valtmp;
    fmt % status_list[status];
    if (validation.values.size() > 0) {
        for (const auto &tag: std::as_const(validation.values)) {
            valtmp += + "'" + dbconn->escapedString(tag) + "',";
        }
        if (!valtmp.empty()) {
            valtmp.pop_back();
        }
        fmt % valtmp;
    }
    fmt % to_simple_string(validation.timestamp);

    std::stringstream ss;
    ss << std::setprecision(12) << boost::geometry::wkt(validation.center);
    std::string center = ss.str();
    fmt % center;

    fmt % validation.source;
    fmt % validation.version;

    // ON CONFLICT
    fmt % validation.version;
    fmt % to_simple_string(validation.timestamp);
    fmt % validation.version;
    query += fmt.str();

    return query;
}

} // namespace queryvalidate

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
