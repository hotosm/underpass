//
// Copyright (c) 2020, 2021, 2022 Humanitarian OpenStreetMap Team
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

#include "pqxx/nontransaction"
#include <array>
#include <assert.h>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
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
using namespace logger;

std::once_flag prepare_user_statement_flag;

/// \namespace queryvalidate
namespace queryvalidate {

static std::mutex pqxx_mutex;

QueryValidate::QueryValidate(void) {}

QueryValidate::QueryValidate(const std::string &dburl) { connect(dburl); };

bool
QueryValidate::updateValidation(std::shared_ptr<std::vector<long>> removals)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("updateValidation: took %w seconds\n");
#endif
    if (removals->size() > 0) {
        std::string query = "DELETE FROM validation WHERE osm_id IN(";
        for (const auto &osm_id : *removals) {
            query += std::to_string(osm_id) + ",";
        };
        query.erase(query.size() - 1);
        query += ");";
        std::scoped_lock write_lock{pqxx_mutex};
        pqxx::work worker(*sdb);
        pqxx::result result = worker.exec(query);
        worker.commit();
    }
    return true;
}

bool
QueryValidate::applyChange(const ValidateStatus &validation) const
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("applyChange(validation): took %w seconds\n");
#endif
    log_debug("Applying Validation data");
    // validation.dump();

    if (validation.angle == 0 && validation.status.size() == 0) {
        return true;
    }
    std::map<osmobjects::osmtype_t, std::string> objtypes = {
        {osmobjects::empty, "empty"},
        {osmobjects::node, "node"},
        {osmobjects::way, "way"},
        {osmobjects::relation, "relation"}};
    std::map<valerror_t, std::string> status = {
        {notags, "notags"},
        {complete, "complete"},
        {incomplete, "incomplete"},
        {badvalue, "badvalue"},
        {correct, "correct"},
        {orphan, "orphan"},
        {overlaping, "overlaping"},
        {duplicate, "duplicate"},
        {badgeom, "badgeom"}};
    std::string format;
    std::string query;
    if (validation.values.size() > 0) {
        query = "INSERT INTO validation (osm_id, change_id, angle, user_id, type, status, values, timestamp, location, source) VALUES(";
        format = "%d, %d, %g, %d, \'%s\', ARRAY[%s]::status[], ARRAY[%s], \'%s\', ST_GeomFromText(\'%s\', 4326), \'%s\'";
    } else {
        query = "INSERT INTO validation (osm_id, change_id, angle, user_id, type, status, timestamp, location) VALUES(";
        format = "%d, %d, %g, %d, \'%s\', ARRAY[%s]::status[], \'%s\', ST_GeomFromText(\'%s\', 4326)";
    }
    boost::format fmt(format);
    fmt % validation.osm_id;
    fmt % validation.change_id;
    if (isnan(validation.angle)) {
        fmt % 0.0;
    } else {
        fmt % validation.angle;
    }
    fmt % validation.user_id;
    fmt % objtypes[validation.objtype];
    std::string stattmp, valtmp;
    for (const auto &_stat: std::as_const(validation.status)) {
        stattmp += " \'" + status[_stat] + "\',";
    }
    if (!stattmp.empty()) {
        stattmp.pop_back();
    }
    fmt % stattmp;
    if (validation.values.size() > 0) {
        for (const auto &tag: std::as_const(validation.values)) {
            valtmp += " \'" + fixString(tag) + "\',";
        }
        if (!valtmp.empty()) {
            valtmp.pop_back();
        }
        fmt % valtmp;
    }
    fmt % to_simple_string(validation.timestamp);
    // Postgres wants the order of lat,lon reversed from how they
    // are stored in the WKT.
    fmt % boost::geometry::wkt(validation.center);
    if (validation.values.size() > 0) {
        fmt % validation.source;
    }
    query += fmt.str();
    query += ") ON CONFLICT (osm_id) DO UPDATE ";
    query += " SET status = ARRAY[" + stattmp + " ]::status[]";
    if (validation.values.size() > 0) {
        query += ", values = ARRAY[" + valtmp + " ], source = \'" + validation.source + "\'";
    }
//    log_debug("QUERY: %1%", query);

    std::scoped_lock write_lock{pqxx_mutex};
    pqxx::work worker(*sdb);
    pqxx::result result = worker.exec(query);
    worker.commit();

    return true;
}

std::string
QueryValidate::fixString(std::string text) const
{
    std::string newstr;
    int i = 0;
    while (i < text.size()) {
        if (text[i] == '\'') {
            newstr += "&apos;";
        } else if (text[i] == '\"') {
            newstr += "&quot;";
        } else if (text[i] == ')') {
            newstr += "&#41;";
        } else if (text[i] == '(') {
            newstr += "&#40;";
        } else if (text[i] == '\\') {
            // drop this character
        } else {
            newstr += text[i];
        }
        i++;
    }
    return sdb->esc(boost::locale::conv::to_utf<char>(newstr, "Latin1"));
}

} // namespace queryvalidate

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
