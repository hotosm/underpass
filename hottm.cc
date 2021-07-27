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
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <pqxx/pqxx>

#include "hottm.hh"

namespace tmdb {

bool
TaskingManager::connect(std::string &database)
{
    if (database.empty()) {
	database = "tmsnap";
    }
    
    try {
	std::string args = "dbname = " + database;
	db = new pqxx::connection(args);
	if (db->is_open()) {
	    worker = new pqxx::work(*db);
	    return true;
	} else {
	    return false;
	}
    } catch (const std::exception &e) {
	std::cerr << e.what() << std::endl;
	return false;
   }
}

std::shared_ptr<std::vector<TMTeam>>
TaskingManager::getTeams(long teamid)
{
    auto teams =  std::make_shared<std::vector<TMTeam>>();

    std::string sql = "SELECT id,organisation_id,name FROM teams";
    if (teamid > 0) {
	sql += " WHERE id=" + std::to_string(teamid);
    }

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);

    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
        TMTeam team(it);
	teams->push_back(team);
    }

    
    return teams;
}

std::shared_ptr<std::vector<long>>
TaskingManager::getTeamMembers(long teamid, bool active)
{
    auto members =  std::make_shared<std::vector<long>>();

    std::string sql = "SELECT user_id FROM team_members WHERE team_id=";
    sql +=  std::to_string(teamid);
    if (active) {
        sql += " AND active='t'";
    }

    pqxx::result result = worker->exec(sql);
    // pqxx::array_parser parser = result[0][0].as_array();
    pqxx::result::const_iterator rit;
    for (rit = result.begin(); rit != result.end(); ++rit) {
        //members->push_back(std::stol(rit));
        long foo = rit[0].as(long(0));
        members->push_back(foo);
    }

    return members;
}

std::shared_ptr<std::vector<TMUser>>
TaskingManager::getUsers(long userid)
{
    auto users =  std::make_shared<std::vector<TMUser>>();

    std::string sql = "SELECT id,username,role,mapping_level,tasks_mapped,tasks_validated, tasks_invalidated,name,date_registered,last_validation_date FROM users";
    if (userid > 0) {
	sql += " WHERE id=" + std::to_string(userid);
    }

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    std::cout << "SIZE: " << result.size() <<std::endl;

    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
        TMUser user(it);
	users->push_back(user);
    }
    
    return users;
};

std::shared_ptr<std::vector<TMProject>>
TaskingManager::getProjects(long projectid)
{
    auto projects =  std::make_shared<std::vector<TMProject>>();

    std::string sql = "SELECT id,status,created,priority,author_id,mapper_level,total_tasks,tasks_mapped,tasks_validated FROM projects";
    if (projectid > 0) {
        sql += " WHERE id=" + std::to_string(projectid);
    }

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    std::cout << "SIZE: " << result.size() <<std::endl;
    
    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
        TMProject project(it);
	projects->push_back(project);
    }
    
    return projects;
};

std::shared_ptr<std::vector<long>>
TaskingManager::getProjectTeams(long projectid)
{
    auto teams =  std::make_shared<std::vector<long>>();

    std::string sql = "SELECT team_id FROM project_teams WHERE project_id=";
    sql +=  std::to_string(projectid);

    pqxx::result result = worker->exec(sql);
    // pqxx::array_parser parser = result[0][0].as_array();
    pqxx::result::const_iterator rit;
    for (rit = result.begin(); rit != result.end(); ++rit) {
        long foo = rit[0].as(long(0));
        teams->push_back(foo);
    }

    return teams;
}

} // EOF tmdb namespace
// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

