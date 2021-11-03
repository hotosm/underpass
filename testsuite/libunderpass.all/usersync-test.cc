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
#include "log.hh"
#include "galaxy/galaxy.hh"

#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem/path.hpp"
#include <boost/date_time.hpp>

using namespace tmdb;
using namespace galaxy;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::filesystem;

TestState runtest;

class TestOSMStats : public QueryGalaxy
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

        const std::string test_tm_db_name{"taskingmanager_usersync_test"};

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

        // Prepare OSMStats test database
        const std::string test_galaxy_db_name{"galaxy_usersync_test"};

        {
            pqxx::connection conn{dbconn};
            pqxx::nontransaction worker{conn};
            worker.exec0("DROP DATABASE IF EXISTS " + test_galaxy_db_name);
            worker.exec0("CREATE DATABASE " + test_galaxy_db_name);
            worker.commit();
        }

        {
            pqxx::connection conn{dbconn + " dbname=" + test_galaxy_db_name};
            pqxx::nontransaction worker{conn};
            worker.exec0("CREATE EXTENSION postgis");

            // Create schema
            // TODO: get absolute base path
            const path base_path{source_tree_root / "data"};
            const auto schema_path{base_path / "galaxy.sql"};
            std::ifstream schema_definition(schema_path);
            std::string sql((std::istreambuf_iterator<char>(schema_definition)),
                            std::istreambuf_iterator<char>());

            assert(!sql.empty());
            worker.exec0(sql);
        }

        return true;
    };
};

int
main(int argc, char *argv[])
{

    // Test preconditions
    TestOSMStats testosm;

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setLogFilename("");
    dbglogfile.setVerbosity();

    testosm.init_test_case();

    const std::string test_galaxy_db_name{"galaxy_usersync_test"};
    const std::string test_tm_db_name{"taskingmanager_usersync_test"};
    std::string galaxy_conn;
    std::string tm_conn;
    if (getenv("PGHOST") && getenv("PGUSER") && getenv("PGPASSWORD")) {
        galaxy_conn = std::string(getenv("PGUSER")) + ":" +
                        std::string(getenv("PGPASSWORD")) + "@" +
                        std::string(getenv("PGHOST")) + "/" +
                        test_galaxy_db_name;
        tm_conn = std::string(getenv("PGUSER")) + ":" +
                  std::string(getenv("PGPASSWORD")) + "@" +
                  std::string(getenv("PGHOST")) + "/" + test_tm_db_name;
    } else {
        galaxy_conn = test_galaxy_db_name;
        tm_conn = test_tm_db_name;
    }

    // Retrieve users from test DB
    TaskingManager tm;
    assert(tm.connect(tm_conn));

    auto users{tm.getUsers()};
    assert(users.size() == 2);

    // Start the real test

    TestOSMStats testgalaxy;

    if (testgalaxy.connect(galaxy_conn)) {
        runtest.pass("QueryOSMStats::connect()");
    } else {
        runtest.fail("QueryOSMStats::connect()");
        exit(1);
    }

    // Sync
    auto result{testgalaxy.syncUsers(users)};

    if (result.created == 2 && result.updated == 0 && result.deleted == 0) {
        runtest.pass("QueryOSMStats::syncUsers() - create");
    } else {
        runtest.fail("QueryOSMStats::syncUsers() - create");
    }

    // Check that the users were really added
    // TODO: add an API method to retrieve this list?
    const auto get_users = [&]() -> std::vector<TMUser> {
        std::vector<TMUser> users;
        pqxx::nontransaction worker{*testgalaxy.sdb};
        const auto users_result{worker.exec("SELECT * FROM users ORDER BY id")};
        pqxx::result::const_iterator it;

        for (it = users_result.begin(); it != users_result.end(); ++it) {
            TMUser user(it);
            users.push_back(user);
        }
        return users;
    };

    TMUser expectedAlice;
    expectedAlice.id = 21792;
    expectedAlice.username = "alice";
    expectedAlice.name = "Alice";
    expectedAlice.role = TMUser::Role::MAPPER;
    expectedAlice.mapping_level = TMUser::MappingLevel::BEGINNER;
    expectedAlice.tasks_mapped = 9;
    expectedAlice.tasks_validated = 0;
    expectedAlice.tasks_invalidated = 0;
    expectedAlice.projects_mapped = {135};
    expectedAlice.last_validation_date =
        time_from_string("2021-03-12 06:51:12.813");
    expectedAlice.date_registered = time_from_string("2020-09-21 09:44:40.180");
    expectedAlice.gender = TMUser::Gender::FEMALE;

    TMUser expectedBob;
    expectedBob.id = 95488;
    expectedBob.username = "bob";
    expectedBob.name = "";
    expectedBob.role = TMUser::Role::UNSET;
    expectedBob.mapping_level = TMUser::MappingLevel::ADVANCED;
    expectedBob.tasks_mapped = 20;
    expectedBob.tasks_validated = 329;
    expectedBob.tasks_invalidated = 60;
    expectedBob.projects_mapped = {135, 272};
    expectedBob.last_validation_date =
        time_from_string("2020-11-04 02:20:28.209");
    expectedBob.date_registered = time_from_string("2013-01-04 02:01:04.405");
    expectedBob.gender = TMUser::Gender::UNSET;

    auto galaxy_users{get_users()};
    if (galaxy_users.size() == 2 && galaxy_users.at(0) == expectedAlice &&
        galaxy_users.at(1) == expectedBob) {
        runtest.pass("QueryOSMStats::syncUsers() - check create");
    } else {
        runtest.fail("QueryOSMStats::syncUsers() - check create");
    }

    // Test update
    expectedBob.name = "Bob";
    expectedAlice.name = "Alice in Wonderland";
    users.at(0).name = users.at(0).id == 95488 ? "Bob" : "Alice in Wonderland";
    users.at(1).name = users.at(1).id == 95488 ? "Bob" : "Alice in Wonderland";

    result = testgalaxy.syncUsers(users);

    galaxy_users = get_users();
    if (result.updated == 2 && result.created == 0 && result.deleted == 0 &&
        galaxy_users.size() == 2 && galaxy_users.at(0) == expectedAlice &&
        galaxy_users.at(1) == expectedBob) {
        runtest.pass("QueryOSMStats::syncUsers() - update");
    } else {
        runtest.fail("QueryOSMStats::syncUsers() - update");
    }

    // Test delete
    // Remove bob
    users.erase(std::remove_if(users.begin(), users.end(),
                               [](TMUser user) { return user.id == 95488; }));
    result = testgalaxy.syncUsers(users, true);
    galaxy_users = get_users();

    if (result.updated == 1 && result.created == 0 && result.deleted == 1 &&
        galaxy_users.size() == 1 && galaxy_users.at(0) == expectedAlice) {
        runtest.pass("QueryOSMStats::syncUsers() - delete");
    } else {
        runtest.fail("QueryOSMStats::syncUsers() - delete");
    }
}
