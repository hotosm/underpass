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

#include "osmstats/replication.hh"
using namespace replication;
#include "data/underpass.hh"
using namespace underpass;

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::filesystem;
TestState runtest;

class TestPlanet : public Planet
{
  public:
    TestPlanet() = default;

    //! Clear the test DB and fill it with with initial test data
    bool init_test_case()
    {

        logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
        dbglogfile.setVerbosity();

        const std::string dbconn{getenv("UNDERPASS_TEST_DB_CONN")
                                     ? getenv("UNDERPASS_TEST_DB_CONN")
                                     : ""};
        source_tree_root = getenv("UNDERPASS_SOURCE_TREE_ROOT")
                               ? getenv("UNDERPASS_SOURCE_TREE_ROOT")
                               : ".";

        const std::string test_replication_db_name{"replication_planet_test"};

        {
            pqxx::connection conn{dbconn};
            pqxx::nontransaction worker{conn};
            worker.exec0("DROP DATABASE IF EXISTS " + test_replication_db_name);
            worker.exec0("CREATE DATABASE " + test_replication_db_name);
            worker.commit();
        }

        {
            pqxx::connection conn{dbconn +
                                  " dbname=" + test_replication_db_name};
            pqxx::nontransaction worker{conn};
            worker.exec0("CREATE EXTENSION postgis");
            worker.exec0("CREATE EXTENSION hstore");

            // Create schema
            const auto schema_path{source_tree_root / "data" / "underpass.sql"};
            std::ifstream schema_definition(schema_path);
            std::string sql((std::istreambuf_iterator<char>(schema_definition)),
                            std::istreambuf_iterator<char>());

            assert(!sql.empty());
            worker.exec0(sql);
        }

        return true;
    };

    std::string source_tree_root;
};

int
main(int argc, char *argv[])
{

    // Test preconditions

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setLogFilename("");
    dbglogfile.setVerbosity();

    TestPlanet test_planet;
    Underpass underpass;

    test_planet.init_test_case();

    const std::string test_replication_db_name{"replication_planet_test"};
    std::string replication_conn;
    if (getenv("PGHOST") && getenv("PGUSER") && getenv("PGPASSWORD")) {
        replication_conn = std::string(getenv("PGUSER")) + ":" +
                           std::string(getenv("PGPASSWORD")) + "@" +
                           std::string(getenv("PGHOST")) + "/" +
                           test_replication_db_name;
    } else {
        replication_conn = test_replication_db_name;
    }

    if (underpass.connect(replication_conn)) {
        runtest.pass("Underpass::connect()");
    } else {
        runtest.fail("Underpass::connect()");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    assert(test_planet.connectServer("https://planet.maps.mail.ru"));

    // Test sequence to path
    if (test_planet.sequenceToPath(1).compare("/000/000/001") == 0) {
        runtest.pass("Underpass::sequenceToPath(/000/000/001)");
    } else {
        runtest.fail("Underpass::sequenceToPath(/000/000/001)");
        exit(EXIT_FAILURE);
    }

    if (test_planet.sequenceToPath(123456789).compare("/123/456/789") == 0) {
        runtest.pass("Underpass::sequenceToPath(/123/456/789)");
    } else {
        runtest.fail("Underpass::sequenceToPath(/123/456/789)");
        exit(EXIT_FAILURE);
    }

    if (test_planet.sequenceToPath(999999999).compare("/999/999/999") == 0) {
        runtest.pass("Underpass::sequenceToPath(/999/999/999)");
    } else {
        runtest.fail("Underpass::sequenceToPath(/999/999/999)");
        exit(EXIT_FAILURE);
    }

    // Test scan
    const auto links{test_planet.scanDirectory("/replication/minute/000/000/")};
    if (links->size() > 0) {
        runtest.pass("Underpass::scanDirectory()");
    } else {
        runtest.fail("Underpass::scanDirectory()");
        exit(EXIT_FAILURE);
    }

    // Try first state
    auto data{test_planet.fetchData(replication::minutely, 1)};
    //std::cout << to_iso_extended_string(data->timestamp) << std::endl;
    if (data->isValid() && data->sequence == 1 &&
        to_iso_extended_string(data->timestamp)
                .compare("2012-09-12T08:15:45") == 0 &&
        !underpass.getFirstState(replication::minutely)->isValid()) {
        runtest.pass("Underpass::fetchData(minutely, 1)");
    } else {
        runtest.fail("Underpass::fetchData(minutely, 1)");
        exit(EXIT_FAILURE);
    }

    // Try caching
    data = test_planet.fetchData(replication::minutely, 1, replication_conn);
    //std::cout << to_iso_extended_string(data->timestamp) << std::endl;
    if (data->isValid() && data->sequence == 1 &&
        to_iso_extended_string(data->timestamp)
                .compare("2012-09-12T08:15:45") == 0 &&
        underpass.getFirstState(replication::minutely)->isValid()) {
        runtest.pass("Underpass::fetchData(minutely, 1, replication_conn)");
    } else {
        runtest.fail("Underpass::fetchData(minutely, 1, replication_conn)");
        exit(EXIT_FAILURE);
    }

    // Now retrieve the cached value
    data = test_planet.fetchData(replication::minutely, 1, replication_conn);
    //std::cout << to_iso_extended_string(data->timestamp) << std::endl;
    if (data->isValid() && data->sequence == 1 &&
        to_iso_extended_string(data->timestamp)
                .compare("2012-09-12T08:15:45") == 0 &&
        underpass.getFirstState(replication::minutely)->isValid()) {
        runtest.pass(
            "Underpass::fetchData(minutely, 1, replication_conn - cached)");
    } else {
        runtest.fail(
            "Underpass::fetchData(minutely, 1, replication_conn - cached)");
        exit(EXIT_FAILURE);
    }
}
