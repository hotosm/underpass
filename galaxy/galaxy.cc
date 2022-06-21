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

/// \file galaxy.hh
/// \brief This file is used to work with the OSM Stats database
///
/// This manages the OSM Stats schema in a postgres database. This
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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/math/special_functions/relative_difference.hpp>

using namespace boost::math;
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "data/osmobjects.hh"
#include "log.hh"
#include "galaxy/changeset.hh"
#include "galaxy/galaxy.hh"
#include "validate/validate.hh"
using namespace logger;

std::once_flag prepare_user_statement_flag;

/// \namespace galaxy
namespace galaxy {

static std::mutex pqxx_mutex;

QueryGalaxy::QueryGalaxy(void) {}

QueryGalaxy::QueryGalaxy(const std::string &dburl) { connect(dburl); };

bool
QueryGalaxy::applyChange(const osmchange::ChangeStats &change) const
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("applyChange(statistics): took %w seconds\n");
#endif
    // std::cout << "Applying OsmChange data" << std::endl;

#if 0
    if (hasHashtag(change.change_id)) {
        std::cout << "Has hashtag for id: " << change.change_id << std::endl;
    } else {
        std::cerr << "No hashtag for id: " << change.change_id << std::endl;
    }
#endif
    std::string ahstore;

    if (change.added.size() > 0) {
        ahstore = "HSTORE(ARRAY[";

        for (const auto &added: std::as_const(change.added)) {
            if (added.first.empty()) {
                return true;
            }

            if (added.second > 0) {
                ahstore += " ARRAY[\'" + added.first + "\',\'" +
                           std::to_string(added.second) + "\'],";
            }
        }

        ahstore.erase(ahstore.size() - 1);
        ahstore += "])";
    } else {
        ahstore.clear();
    }

    if (change.modified.size() > 0) {
        ahstore = "HSTORE(ARRAY[";

        for (const auto &modified: std::as_const(change.modified)) {
            if (modified.first.empty()) {
                return true;
            }

            if (modified.second > 0) {
                ahstore += " ARRAY[\'" + modified.first + "\',\'" +
                           std::to_string(modified.second) + "\'],";
            }
        }

        ahstore.erase(ahstore.size() - 1);
        ahstore += "])";
    }

    // Some of the data field in the changset come from a different file,
    // which may not be downloaded yet.
    ptime now = boost::posix_time::microsec_clock::universal_time();
    std::string aquery;

    if (ahstore.size() > 0) {
        if (change.added.size() > 0) {
            aquery = "INSERT INTO changesets (id, user_id, closed_at, "
                     "updated_at, added)";
        } else if (change.modified.size() > 0) {
            aquery =
                "INSERT INTO changesets (id, user_id, closed_at, updated_at, "
                "modified)";
        }
    } else {
        aquery = "INSERT INTO changesets (id, user_id, updated_at)";
    }

    aquery += " VALUES(" + std::to_string(change.change_id) + ", ";
    aquery += std::to_string(change.user_id) + ", ";
    aquery += "\'" + to_simple_string(change.closed_at) + "\', ";
    aquery += "\'" + to_simple_string(now) + "\', ";

    if (ahstore.size() > 0) {
        if (change.added.size() > 0) {
            aquery += ahstore +
                      ") ON CONFLICT (id) DO UPDATE SET added = " + ahstore +
                      ",";
        } else {
            aquery += ahstore +
                      ") ON CONFLICT (id) DO UPDATE SET modified = " + ahstore +
                      ",";
        }
    } else {
        aquery.erase(aquery.size() - 2);
        aquery += ") ON CONFLICT (id) DO UPDATE SET";
    }

    aquery += " closed_at = \'" + to_simple_string(change.closed_at) + "\',";
    aquery += " updated_at = \'" + to_simple_string(now) + "\'";
    // aquery += " WHERE changesets.id=" + std::to_string(change.change_id); //Fix me : we are not sure whether the changefile comes first or changeset comes first in planet , we are dependent on planet and planet doesnot always provide changefile and changeset file parallely sometimes there is delay , hence decided to store it for now , when we will be removing changeset from area filter those rows coming from changefile should be removed

    // _debug(_("QUERY stats: %1%"), aquery);
    // Serialize changes writing
    {
        std::scoped_lock write_lock{pqxx_mutex};
        pqxx::work worker(*sdb);
        pqxx::result result = worker.exec(aquery);
        worker.commit();
    }

    return true;
}

bool
QueryGalaxy::applyChange(const changeset::ChangeSet &change) const
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("applyChange(changeset): took %w seconds\n");
#endif
    // log_debug(_("Applying ChangeSet data"));
    // change.dump();

    // Some old changefiles have no user information
    std::string query = "INSERT INTO users VALUES(";
    query += std::to_string(change.uid) + ",\'" + fixString(change.user);
    query += "\') ON CONFLICT DO NOTHING;";
    // log_debug(_("QUERY: %1%"), query);

    // Serialize changes writing
    {
	std::scoped_lock write_lock{pqxx_mutex};
        pqxx::work worker(*sdb);
        pqxx::result result = worker.exec(query);
        worker.commit();
    }

    // Add changeset data
    // road_km_added | road_km_modified | waterway_km_added |
    // waterway_km_modified | roads_added | roads_modified | waterways_added |
    // waterways_modified | buildings_added | buildings_modified | pois_added |
    // pois_modified | updated_at the updated_at timestamp is set after the
    // change data has been processed

    // galaxy=# UPDATE raw_changesets SET road_km_added = (SELECT
    // road_km_added
    // + 10.0 FROM raw_changesets WHERE road_km_added>0 AND user_id=649260 LIMIT
    // 1) WHERE user_id=649260;

    query = "INSERT INTO changesets (id, editor, user_id, created_at, closed_at, updated_at";

    if (change.hashtags.size() > 0) {
        query += ", hashtags ";
    }

    if (!change.source.empty()) {
        query += ", source ";
    }

    query += ", bbox) VALUES(";
    query += std::to_string(change.id) + ",\'" + fixString(change.editor) + "\',\'";

    query += std::to_string(change.uid) + "\',\'";
    query += to_simple_string(change.created_at) + "\', \'";
    if (change.closed_at != not_a_date_time) {
	query += to_simple_string(change.closed_at) + "\', \'";
    } else {
	query += to_simple_string(change.created_at) + "\', \'";
    }
    query += to_simple_string(boost::posix_time::second_clock::universal_time()) + "\'";

    // if (!change.open) {
    //     query += ",\'" + to_simple_string(change.closed_at) + "\'";
    // }
    // Hashtags are only used in mapping campaigns using Tasking Manager
    if (change.hashtags.size() > 0) {
        query += ", \'{ ";

        for (const auto &hashtag: std::as_const(change.hashtags)) {
            auto ht{hashtag};
            query += "\"" + fixString(ht) + "\"" + ", ";
        }

        // drop the last comma
        query.erase(query.size() - 2);
        query += " }\'";
    }

    // The source field is not always present
    if (!change.source.empty()) {
        query += ",\'" + fixString(change.source) += "\'";
    }

    // Store the current values as they may get changed to expand very short
    // lines or POIs so they have a bounding box big enough for postgis to use.
    double min_lat = change.min_lat;
    double max_lat = change.max_lat;
    double min_lon = change.min_lon;
    double max_lon = change.max_lon;
    // FIXME: There are bugs in the bounding box coordinates in some of the
    // older changeset files, but for now ignore these.
  
    const double fudge{0.0001};

    // a changeset with a single node in it doesn't draw a line
    if (change.max_lon < 0 && change.min_lat < 0) {
        // log_error(_("WARNING: single point! %1%"), change.id);
        min_lat = change.min_lat + (fudge / 2);
        max_lat = change.max_lat + (fudge / 2);
        min_lon = change.min_lon - (fudge / 2);
        max_lon = change.max_lon - (fudge / 2);
        // return false;
    }

    if (max_lon == min_lon || max_lat == min_lat) {
        // log_error(_("WARNING: not a line! %1%"), change.id);
        min_lat = change.min_lat + (fudge / 2);
        max_lat = change.max_lat + (fudge / 2);
        min_lon = change.min_lon - (fudge / 2);
        max_lon = change.max_lon - (fudge / 2);
        // return false;
    }

    if (max_lon < 0 && min_lat < 0) {
        // log_error(_("WARNING: single point! "), change.id);
        min_lat = change.min_lat + (fudge / 2);
        max_lat = change.max_lat + (fudge / 2);
        min_lon = change.min_lon - (fudge / 2);
        max_lon = change.max_lon - (fudge / 2);
        // return false;
    }

    // if (change.num_changes == 0) {
    //     log_debug(_("WARNING: no changes!"), change.id);
    //     return false;
    // }

    // Add the bounding box of the changeset here next
    // long_high,lat_high,
    // long_low,lat_high,
    // long_low,lat_low,
    // long_high,lat_low,
    // long_high,lat_high
    std::string bbox;
    bbox += ", ST_MULTI(ST_GeomFromEWKT(\'SRID=4326;POLYGON((";
    // Upper left
    bbox += std::to_string(max_lon) + "  ";
    bbox += std::to_string(max_lat) + ",";
    // Upper right
    bbox += std::to_string(min_lon) + "  ";
    bbox += std::to_string(max_lat) + ",";
    // Lower right
    bbox += std::to_string(min_lon) + "  ";
    bbox += std::to_string(min_lat) + ",";
    // Lower left
    bbox += std::to_string(max_lon) + "  ";
    bbox += std::to_string(min_lat) + ",";
    // Close the polygon
    bbox += std::to_string(max_lon) + "  ";
    bbox += std::to_string(max_lat) + ")";

    query += bbox;

    query += ")\')";
    // query += ")) ON CONFLICT DO NOTHING;";
    query += ")) ON CONFLICT (id) DO UPDATE SET editor=\'" +  fixString(change.editor);
    query += "\', created_at=\'" + to_simple_string(change.created_at);
    // if (!change.open) {
    // 	query += "\', closed_at=\'" + to_simple_string(change.closed_at);
    // }
    query += "\', bbox=" + bbox.substr(2) + ")'))";
    // log_debug(_("QUERY: %1%"), query);

    // Serialize changes writing
    {
	std::scoped_lock write_lock{pqxx_mutex};
        pqxx::work worker(*sdb);
        pqxx::result result = worker.exec(query);
        worker.commit();
    }

    return true;
}

bool
QueryGalaxy::updateValidation(long id)
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("updateValidation: took %w seconds\n");
#endif
    std::string query = "DELETE FROM validation WHERE osm_id=";
    query += std::to_string(id);
    std::scoped_lock write_lock{pqxx_mutex};
    pqxx::work worker(*sdb);
    pqxx::result result = worker.exec(query);
    worker.commit();

    return true;
}

bool
QueryGalaxy::applyChange(const ValidateStatus &validation) const
{
#ifdef TIMING_DEBUG_X
    boost::timer::auto_cpu_timer timer("applyChange(validation): took %w seconds\n");
#endif
    log_debug(_("Applying Validation data"));
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
	query = "INSERT INTO validation (osm_id, change_id, angle, user_id, type, status, values, timestamp, location) VALUES(";
	format = "%d, %d, %g, %d, \'%s\', ARRAY[%s]::status[], ARRAY[%s], \'%s\', ST_GeomFromText(\'%s\', 4326)";
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
    query += fmt.str();
    query += ") ON CONFLICT (osm_id) DO UPDATE ";
    query += " SET status = ARRAY[" + stattmp + " ]::status[]";
    if (validation.values.size() > 0) {
	query += ", values = ARRAY[" + valtmp + " ]";
    }
//    log_debug(_("QUERY: %1%"), query);

    std::scoped_lock write_lock{pqxx_mutex};
    pqxx::work worker(*sdb);
    pqxx::result result = worker.exec(query);
    worker.commit();

    return true;
}

// Get the timestamp of the last update in the database
ptime
QueryGalaxy::getLastUpdate(void)
{
    std::string query = "SELECT MAX(created_at) FROM changesets;";
    // log_debug(_("QUERY: %1%"), query);
    pqxx::work worker(*sdb);
    pqxx::result result = worker.exec(query);
    worker.commit();

    ptime last;

    if (result[0][0].size() > 0) {
        last = time_from_string(result[0][0].c_str());
        return last;
    }

    return not_a_date_time;
}

QueryGalaxy::SyncResult
QueryGalaxy::syncUsers(const std::vector<TMUser> &users, bool purge)
{
    // Preconditions:
    assert(sdb);

    std::vector<TaskingManagerIdType> currentIds;

    pqxx::work worker(*sdb);
    const auto result{worker.exec("SELECT id FROM users")};

    for (const auto &row: std::as_const(result)) {
        currentIds.push_back(row.at(0).as(TaskingManagerIdType(0)));
    }

    log_debug(_("Galaxy users count: %1%"), currentIds.size());

    SyncResult syncResult;
    std::vector<TaskingManagerIdType> updatedIds;

    bool hasError{false};

    // Prepare statements for insert and update
    std::call_once(prepare_user_statement_flag, [this]() {
        const std::string insertSql{R"sql(
                          INSERT INTO users (
                            id,
                            username,
                            name,
                            date_registered,
                            last_validation_date,
                            tasks_mapped,
                            tasks_validated,
                            tasks_invalidated,
                            projects_mapped,
                            mapping_level,
                            gender,
                            access
                          )
                          VALUES ( $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12 )
                        )sql"};
        const std::string updateSql{R"sql(
              UPDATE users SET
                  username = $2,
                  name = $3,
                  date_registered = $4,
                  last_validation_date = $5,
                  tasks_mapped = $6,
                  tasks_validated = $7,
                  tasks_invalidated = $8,
                  projects_mapped = $9,
                  mapping_level = $10,
                  gender = $11,
                  access = $12
              WHERE id = $1
            )sql"};

        sdb->prepare("insert_galaxy_user", insertSql);
        sdb->prepare("update_galaxy_user", updateSql);
    });

    for (const auto &user: std::as_const(users)) {
        if (hasError) {
            break;
        }
        const int mapping_level{static_cast<int>(user.mapping_level)};
        const int gender{static_cast<int>(user.gender)};
        const int role{static_cast<int>(user.access)};
        std::string projects_mapped{"{"};

        for (const auto &elem: std::as_const(user.projects_mapped)) {
            projects_mapped += std::to_string(elem);

            if (elem != user.projects_mapped.back()) {
                projects_mapped += ",";
            }
        }

        projects_mapped += "}";

        // If the id exists it is an update
        if (std::find(currentIds.begin(), currentIds.end(), user.id) !=
            currentIds.end()) {
            try {

                // clang-format off
                const auto result{
                    worker.exec_prepared0("update_galaxy_user",
                       user.id,
                       user.username,
                       user.name,
                       to_iso_string(user.date_registered),
                       to_iso_string(user.last_validation_date),
                       user.tasks_mapped,
                       user.tasks_validated,
                       user.tasks_invalidated,
                       projects_mapped,
                       mapping_level,
                       gender,
                       role
                )};
                // clang-format on
                if (result.affected_rows() == 1) {
                    syncResult.updated++;
                    updatedIds.push_back(user.id);
                    if (syncResult.updated % 1000 == 0) {
                        // log_debug(_("Users updated: %1%"),
                        // syncResult.updated);
                    }
                }
            } catch (std::exception const &e) {
                log_error(_("Couldn't update user record: %1%"), e.what());
                hasError = true;
            }
        } else {
            try {
                // clang-format off
                const auto result{
                    worker.exec_prepared0("insert_galaxy_user",
                       user.id,
                       user.username,
                       user.name,
                       to_iso_string(user.date_registered),
                       to_iso_string(user.last_validation_date),
                       user.tasks_mapped,
                       user.tasks_validated,
                       user.tasks_invalidated,
                       projects_mapped,
                       mapping_level,
                       gender,
                       role
                )};
                // clang-format on
                if (result.affected_rows() == 1) {
                    syncResult.created++;
                    if (syncResult.created % 1000 == 0) {
                        // log_debug(_("Users created: %1%"),
                        // syncResult.created);
                    }
                }
            } catch (std::exception const &e) {
                log_error(_("Couldn't create user record: %1%"), e.what());
                hasError = true;
            }
        }
    }

    if (purge && !hasError) {
        std::vector<TaskingManagerIdType> deletedIds;
        std::sort(currentIds.begin(), currentIds.end());
        std::sort(updatedIds.begin(), updatedIds.end());
        std::set_difference(currentIds.begin(), currentIds.end(),
                            updatedIds.begin(), updatedIds.end(),
                            std::inserter(deletedIds, deletedIds.begin()));
        if (!deletedIds.empty()) {
            std::string sql{"DELETE FROM users WHERE id IN ("};
            for (auto it = deletedIds.cbegin(); it != deletedIds.cend(); ++it) {
                if (it != deletedIds.cbegin()) {
                    sql += ",";
                }
                sql += std::to_string(*it);
            }
            sql += ")";
            try {
                const auto result{worker.exec0(sql)};
                // log_debug(_("Users deleted %1%"), deletedIds.size());
                syncResult.deleted = deletedIds.size();
            } catch (std::exception const &e) {
                log_error(_("Couldn't delete user records: %1%"), e.what());
                hasError = true;
            }
        }
    }

    if (!hasError) {
        try {
            worker.commit();
        } catch (std::exception const &e) {
            log_error(_("Couldn't commit user record changes: %1%"), e.what());
            syncResult.clear();
        }
    } else {
        syncResult.clear();
    }

    return syncResult;
};

std::string
QueryGalaxy::fixString(std::string text) const
{
    return boost::locale::conv::to_utf<char>(sdb->esc(text), "Latin1");
}

} // namespace galaxy

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
