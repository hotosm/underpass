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

#include <dejagnu.h>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "hottm.hh"

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/path.hpp"
#include <boost/date_time.hpp>

using namespace tmdb;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::filesystem;

TestState runtest;

class TestTM : public TaskingManager
{
  public:
    //! Clear the test DB and fill it with with initial test data
    bool init_test_case()
    {

        const std::string dbconn{getenv("UNDERPASS_TEST_DB_CONN")
                                     ? getenv("UNDERPASS_TEST_DB_CONN")
                                     : ""};
        const std::string source_tree_root{
            getenv("UNDERPASS_SOURCE_TREE_ROOT")
                ? getenv("UNDERPASS_SOURCE_TREE_ROOT")
                : "."};

        const std::string test_tm_db_name{"taskingmanager_tm_test"};

        {
            pqxx::connection conn{dbconn};
            pqxx::nontransaction worker{conn};
            worker.exec0("DROP DATABASE IF EXISTS " + test_tm_db_name);
            worker.exec0("CREATE DATABASE " + test_tm_db_name);
            worker.commit();
        }

        {
            pqxx::connection conn{dbconn + " dbname=" + test_tm_db_name};
            pqxx::nontransaction worker{conn};
            worker.exec0("CREATE EXTENSION postgis");

            // Create schema
            const path base_path{source_tree_root / "testsuite"};
            const auto schema_path{base_path / "testdata" /
                                   "taskingmanager_schema.sql"};
            std::ifstream schema_definition(schema_path);
            std::string sql((std::istreambuf_iterator<char>(schema_definition)),
                            std::istreambuf_iterator<char>());

            assert(!sql.empty());
            worker.exec0(sql);

            // Load a minimal data set for testing
            const auto data_path{base_path / "testdata" /
                                 "taskingmanager_test_data.sql"};
            std::ifstream data_definition(data_path);
            std::string data_sql(
                (std::istreambuf_iterator<char>(data_definition)),
                std::istreambuf_iterator<char>());

            assert(!data_sql.empty());
            worker.exec0(data_sql);
        }

        std::string tm_conn;
        if (getenv("PGHOST") && getenv("PGUSER") && getenv("PGPASSWORD")) {
            tm_conn = std::string(getenv("PGUSER")) + ":" +
                      std::string(getenv("PGPASSWORD")) + "@" +
                      std::string(getenv("PGHOST")) + "/" + test_tm_db_name;
        } else {
            tm_conn = test_tm_db_name;
        }

        // Connect
        assert(connect(tm_conn));

        return true;
    };
};

int
main(int argc, char *argv[])
{
    TestTM testtm;

    testtm.init_test_case();

    std::vector<TaskingManagerIdType> retl;
    std::vector<TMUser> retu;
    std::vector<TMTeam> rett;
    std::vector<TMProject> retp;

    // Test TM db readers
    // TODO: check actual values
    retu = testtm.getUsers();

    if (retu.size() > 0 && retu.at(0).id > 0 && !retu.at(0).username.empty() &&
        retu.at(0).tasks_mapped >= 0) {
        runtest.pass("taskingManager::getUsers()");
    } else {
        runtest.fail("taskingManager::getUsers()");
    }

    rett = testtm.getTeams();

    if (rett.size() > 0 && rett.at(0).teamid > 0 && rett.at(0).orgid > 0 &&
        !rett.at(0).name.empty()) {
        runtest.pass("taskingManager::getTeams()");
    } else {
        runtest.fail("taskingManager::getTeams()");
    }

    retl = testtm.getTeamMembers(1, true);

    if (retl.size() >= 0) {
        runtest.pass("taskingManager::getTeamMembers()");
    } else {
        runtest.fail("taskingManager::getTeamMembers()");
    }

    retp = testtm.getProjects();

    if (retp.size() > 0) {
        runtest.pass("taskingManager::getProjects()");
    } else {
        runtest.fail("taskingManager::getProjects()");
    }

    retl = testtm.getProjectTeams(1);

    if (retl.size() >= 0) {
        runtest.pass("taskingManager::getProjectTeams()");
    } else {
        runtest.fail("taskingManager::getProjectTeams()");
    }
}
