//
// Copyright (c) 2020, Humanitarian OpenStreetMap Team
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

#ifndef __TM_H__
#define __TM_H__

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

} // EOF tmdb namespace
#endif  // EOF __TMM_H__
// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

