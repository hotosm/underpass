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

#ifndef __HOTOSM_H__
#define __HOTOSM_H__

#include <array>
#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "hotosm.hh"

namespace apidb {

//static QueryDB::base = new  QueryDB();
    
// QueryDB::QueryDB()
QueryDB::QueryDB(void)
{
    // Keywords used for querying objects
    keywords[QueryDB::building] = "building";
    keywords[QueryDB::waterway] = "waterway";
    keywords[QueryDB::highway] = "highway";
    keywords[QueryDB::education] = "education";
    keywords[QueryDB::emergency] = "emergency";
    keywords[QueryDB::financial] = "financial";
    keywords[QueryDB::government] = "government";
    keywords[QueryDB::humanitarian] = "humanitarian";
    keywords[QueryDB::landuse] = "landuse";
    keywords[QueryDB::natural] = "natural";
    keywords[QueryDB::power] = "power";
    keywords[QueryDB::sport] = "sport";
    keywords[QueryDB::transportation] = "transportation";
    keywords[QueryDB::water] = "water";
    keywords[QueryDB::language] = "language";
    keywords[QueryDB::all] = "all";
}

// Connect to the database
bool
QueryDB::connect(std::string &database)
{
    if (database.empty()) {
	database = "pgsnapshot";
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

pqxx::result
QueryDB::query(std::string &select)
{
    pqxx::result result = worker->exec(select);
    return result;
}

QueryDB::~QueryDB(void)
{
    // TODO: close database
    // QueryDB::db.disconnect ();
}

QueryStats::QueryStats(void)
{

}

QueryStats::~QueryStats(void)
{

}

//
// These methods collect statistics
//


// Get the timestamp of the users last update
ptime &
QueryStats::lastUpdate(long userid, ptime &last)
{
    std::string sql = "SELECT tstamp FROM ways WHERE user_id=";
    sql += std::to_string(userid);
    sql += " ORDER BY tstamp DESC LIMIT 1;";

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    std::string timestamp = pqxx::to_string(result[0][0]);

    if (!timestamp.empty()) {
	last = time_from_string(timestamp);
    }

    return last;
}

long
QueryStats::getLength(object obj, long userid, ptime &start, ptime &end)
{
    std::string sql = "SELECT ST_Length(linestring) FROM ways WHERE tags->'";
    sql += keywords[obj] + "' is not NULL";
    sql += " AND user_id=" + std::to_string(userid);
    sql += " ORDER BY tstamp;";
    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    std::cout << "SIZE: " << result.size() <<std::endl;

    if (result.size() == 0) {
	return 0;
    }
    
    long total = 0;
    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
	std::string count = pqxx::to_string(it[0]);
	if (!count.empty()) {
	    total += std::stol(count);
	}
    }

    return total;
}

// Count how many occurances there are for an object.
// object is building, highway, waterway, all
// action is added, modified, deleted, totals
long
QueryStats::getCount(object obj,long userid, action op,
		     ptime &start, ptime &end) const
{    
    std::string sql;
    if (op == QueryStats::changesets) {
	sql = "SELECT COUNT(changeset_id) FROM ways WHERE tags->'";
    } else if (op == QueryStats::totals) {
	sql = "SELECT COUNT(id) FROM ways WHERE tags->'";
    }
    sql += keywords[obj] + "' is not NULL";

    if (userid > 0) {
	sql += " AND user_id=" + std::to_string(userid);
    }
    sql += " AND tstamp>='" + to_simple_string(start) + "'";
	
    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    pqxx::result::const_iterator it;

    long total = std::stol(pqxx::to_string(result[0][0]));
    
    return total;
}

// Tasking Manager Classes
QueryTM::QueryTM(void)
{

}

// Tasking Manager Classes
QueryTM::~QueryTM(void)
{

}

std::shared_ptr<std::vector<long>>
QueryTM::getProjects(long userid)
{
    auto projects =  std::make_shared<std::vector<long>>();

    std::string sql = "SELECT id FROM projects";
    if (userid > 0) {
	sql += " WHERE author_id=" + std::to_string(userid);
    }
    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    std::cout << "SIZE: " << result.size() <<std::endl;
    
    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
	std::string id = pqxx::to_string(it[0]);
	if (!id.empty()) {
	    projects->push_back(std::stol(id));
	}
    }    
    return projects;
}

std::shared_ptr<std::vector<long>>
QueryTM::getUserTasks(long projectid, long userid)
{
    auto tasks =  std::make_shared<std::vector<long>>();

    std::string sql = "SELECT id FROM task_history";
    sql += " WHERE user_id=" + std::to_string(userid);
    sql += " AND project_id='" + std::to_string(projectid) + "';";

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);
    std::cout << "SIZE: " << result.size() <<std::endl;
    
    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
	std::string id = pqxx::to_string(it[0]);
	if (!id.empty()) {
	    tasks->push_back(std::stol(id));
	}
    }
    
    return tasks;
}

int
QueryTM::getTasksMapped(long userid)
{
    std::string sql = "SELECT tasks_mapped FROM users";
    sql += " WHERE id=" + std::to_string(userid);

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);

    int total = std::stoi(pqxx::to_string(result[0][0]));

    return total;
}

int
QueryTM::getTasksValidated(long userid)
{
    std::string sql = "SELECT tasks_validated FROM users";
    sql += " WHERE id=" + std::to_string(userid);

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);

    int total = std::stoi(pqxx::to_string(result[0][0]));

    return total;
}

std::shared_ptr<std::vector<int>>
QueryTM::getUserTeams(long userid)
{
    auto teams =  std::make_shared<std::vector<int>>();

    std::string sql = "SELECT team_id FROM team_members";
    sql += " WHERE user_id=" + std::to_string(userid);

    std::cout << "QUERY: " << sql << std::endl;
    pqxx::result result = worker->exec(sql);

    pqxx::result::const_iterator it;
    for (it = result.begin(); it != result.end(); ++it) {
	std::string id = pqxx::to_string(it[0]);
	if (!id.empty()) {
	    teams->push_back(std::stol(id));
	}
    }

    return teams;
}

// Tasking Manager Classes
QueryTMAPI::QueryTMAPI(void)
{

}
QueryTMAPI::~QueryTMAPI(void)
{

}

//
// BuildOSM methods
//
int
BuildOSM::getWayNodes(long way_id)
{
    // std::string query = "SELECT * FROM XXXX";
    return 3;
}

} // EOF apidb namespace
#endif  // EOF __HOTOSM_H__
// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

