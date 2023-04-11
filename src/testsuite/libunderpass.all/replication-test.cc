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

#include <dejagnu.h>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

#include "utils/log.hh"

#include "replicator/replication.hh"
using namespace replication;
using namespace underpass;

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

#define VERIFY(condition, message)                                             \
    if (condition) {                                                           \
        runtest.pass(message);                                                 \
    } else {                                                                   \
        runtest.fail(message);                                                 \
        std::cerr << "Failing at: " << __FILE__ << ':' << __LINE__             \
                  << std::endl;                                                \
        exit(EXIT_FAILURE);                                                    \
    }

struct TimeIt {
    TimeIt(const std::string &message = "")
        : start(std::chrono::high_resolution_clock::now()), message(message)
    {
    }
    ~TimeIt()
    {
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start);
        std::cerr << "Execution time for " << message << " : "
                  << elapsed.count() << " ms" << std::endl;
    }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string message;
};

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::filesystem;
TestState runtest;

class TestPlanet : public Planet {
  public:
    TestPlanet() = default;

    //! Clear the test DB and fill it with with initial test data
    bool init_test_case()
    {
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
    dbglogfile.setWriteDisk(false);
    dbglogfile.setVerbosity(logger::LogFile::LOG_DEBUG);

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

    // Add test for remote URL parse (test for segfault)
    RemoteURL().parse("http://planet.maps.mail.ru/replication/");

    VERIFY(underpass.connect(replication_conn), "Underpass::connect()")

    // Connect to server
    VERIFY(test_planet.connectServer("https://planet.maps.mail.ru"),
           "Connect to server")

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
