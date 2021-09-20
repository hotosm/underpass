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

#include <array>
#include <boost/format.hpp>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "hottm.hh"
#include "log.hh"

using namespace logger;

namespace tmdb {

bool
TaskingManager::connect(const std::string &dburl)
{

    if (dburl.empty()) {
        log_error(_(" need to specify URL connection string!"));
    }

    std::string dbuser;
    std::string dbpass;
    std::string dbhost;
    std::string dbname = "dbname=";
    std::size_t apos = dburl.find('@');

    if (apos != std::string::npos) {
        dbuser = "user=";
        std::size_t cpos = dburl.find(':');

        if (cpos != std::string::npos) {
            dbuser += dburl.substr(0, cpos);
            dbpass = "password=";
            dbpass += dburl.substr(cpos + 1, apos - cpos - 1);
        } else {
            dbuser += dburl.substr(0, apos);
        }
    }

    std::vector<std::string> result;

    if (apos != std::string::npos) {
        boost::split(result, dburl.substr(apos + 1), boost::is_any_of("/"));
    } else {
        boost::split(result, dburl, boost::is_any_of("/"));
    }

    if (result.size() == 1) {
        dbname += result[0];
        dbhost = "host=localhost";
    } else if (result.size() == 2) {
        if (result[0] != "localhost") {
            dbhost = "host=";
            dbhost += result[0];
        }

        dbname += result[1];
    }

    std::string args = dbhost + " " + dbname + " " + dbuser + " " + dbpass;
    // log_debug(args);

    try {
        db = std::make_unique<pqxx::connection>(args);

        if (db->is_open()) {
            log_debug(_("Opened database connection to %1%"), dburl);
            return true;
        } else {
            return false;
        }
    } catch (const std::exception &e) {
        log_error(_("Couldn't open database connection to %1% %2%"), dburl,
                  e.what());
        return false;
    }
}

std::vector<TMTeam>
TaskingManager::getTeams(TaskingManagerIdType teamid)
{
    std::vector<TMTeam> teams;

    std::string sql = "SELECT id,organisation_id,name FROM teams";

    if (teamid > 0) {
        sql += " WHERE id=" + std::to_string(teamid);
    }

    std::cout << "QUERY: " << sql << std::endl;

    auto worker{getWorker()};
    if (!worker) {
        log_error(_("NULL worker in getTeams()"));
    } else {
        pqxx::result result = worker->exec(sql);

        pqxx::result::const_iterator it;

        for (it = result.begin(); it != result.end(); ++it) {
            TMTeam team(it);
            teams.push_back(team);
        }
    }

    return teams;
}

std::vector<long>
TaskingManager::getTeamMembers(TaskingManagerIdType teamid, bool active)
{
    std::vector<long> members;

    std::string sql = "SELECT user_id FROM team_members WHERE team_id=";
    sql += std::to_string(teamid);

    if (active) {
        sql += " AND active='t'";
    }

    auto worker{getWorker()};
    if (!worker) {
        log_error(_("NULL worker in getTeamMembers()"));
    } else {
        pqxx::result result = worker->exec(sql);
        // pqxx::array_parser parser = result[0][0].as_array();
        pqxx::result::const_iterator rit;

        for (rit = result.begin(); rit != result.end(); ++rit) {
            // members->push_back(std::stol(rit));
            long foo = rit[0].as(long(0));
            members.push_back(foo);
        }
    }

    return members;
}

std::vector<TMUser>
TaskingManager::getUsers(TaskingManagerIdType userId)
{
    std::vector<TMUser> users;

    // Extract data from TM DB
    // FIXME: missing fields in TM schema:
    // - home
    // - osm_registration
    std::string sql{R"sql(
                      SELECT
                        u.id,
                        u.name,
                        u.username,
                        u.date_registered,
                        u.last_validation_date,
                        u.tasks_mapped,
                        u.tasks_validated,
                        u.tasks_invalidated,
                        u.gender,
                        u.role,
                        u.mapping_level,
                        u.projects_mapped
                      FROM users u
                      ORDER BY u.id
                      )sql"};

    if (userId > 0) {
        sql += " WHERE u.id = " + std::to_string(userId);
    }

    auto worker{getWorker()};
    if (!worker) {
        log_error(_("NULL worker in getUsers()"));
    } else {
        pqxx::result result = worker->exec(sql);
        pqxx::result::const_iterator it;

        for (it = result.begin(); it != result.end(); ++it) {
            TMUser user(it);
            users.push_back(user);
        }
    }

    return users;
};

std::vector<TMProject>
TaskingManager::getProjects(TaskingManagerIdType projectid)
{
    std::vector<TMProject> projects;

    std::string sql = "SELECT "
                      "id,status,created,priority,author_id,mapper_level,total_"
                      "tasks,tasks_mapped,tasks_validated FROM projects";

    if (projectid > 0) {
        sql += " WHERE id=" + std::to_string(projectid);
    }

    auto worker{getWorker()};
    if (!worker) {
        log_error(_("NULL worker in getProjects()"));
    } else {
        std::cout << "QUERY: " << sql << std::endl;
        pqxx::result result = worker->exec(sql);
        std::cout << "SIZE: " << result.size() << std::endl;

        pqxx::result::const_iterator it;

        for (it = result.begin(); it != result.end(); ++it) {
            TMProject project(it);
            projects.push_back(project);
        }
    }

    return projects;
}

std::unique_ptr<pqxx::work>
TaskingManager::getWorker() const
{
    if (!db) {
        return nullptr;
    }
    return std::make_unique<pqxx::work>(*db);
}

std::vector<long>
TaskingManager::getProjectTeams(long projectid)
{
    std::vector<TaskingManagerIdType> teams;

    std::string sql = "SELECT team_id FROM project_teams WHERE project_id=";
    sql += std::to_string(projectid);

    auto worker{getWorker()};
    if (!worker) {
        log_error(_("NULL worker in getProjectTeams()"));
    } else {
        pqxx::result result = worker->exec(sql);
        // pqxx::array_parser parser = result[0][0].as_array();
        pqxx::result::const_iterator rit;

        for (rit = result.begin(); rit != result.end(); ++rit) {
            const TaskingManagerIdType foo = rit[0].as(TaskingManagerIdType(0));
            teams.push_back(foo);
        }
    }

    return teams;
}

} // namespace tmdb
// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
