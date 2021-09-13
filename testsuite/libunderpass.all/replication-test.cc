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

    // Test sequence to path
    VERIFY(test_planet.sequenceToPath(1).compare("/000/000/001") == 0,
           "Underpass::sequenceToPath(/000/000/001)")

    VERIFY(test_planet.sequenceToPath(123456789).compare("/123/456/789") == 0,
           "Underpass::sequenceToPath(/123/456/789)")

    VERIFY(test_planet.sequenceToPath(999999999).compare("/999/999/999") == 0,
           "Underpass::sequenceToPath(/999/999/999)")

    // Store fetch results
    std::shared_ptr<StateFile> data;

    // //////////////////////////////////////////////////////////////////////
    // Changeset tests
    //
    const auto initial_changeset_data =
        test_planet.fetchData(replication::changeset, 0);
    VERIFY(initial_changeset_data->isValid() &&
               initial_changeset_data->sequence == 0 &&
               initial_changeset_data->frequency ==
                   Underpass::freq_to_string(replication::changeset) &&
               initial_changeset_data->path.compare("/000/000/000") == 0 &&
               to_iso_extended_string(initial_changeset_data->timestamp)
                       .compare("2012-10-28T19:35:49") == 0 &&
               !underpass.getFirstState(replication::changeset)->isValid(),
           "Underpass::fetchData(changeset, 0)")

    data = test_planet.fetchData(replication::changeset, 0, replication_conn);
    VERIFY(*data.get() == *initial_changeset_data.get() &&
               underpass.getFirstState(replication::changeset)->isValid(),
           "Underpass::fetchData(changeset, 0, replication_conn)")

    // Now retrieve the cached value
    data = test_planet.fetchData(replication::changeset, 0, replication_conn);
    VERIFY(*data.get() == *initial_changeset_data.get() &&
               underpass.getFirstState(replication::changeset)->isValid(),
           "Underpass::fetchData(changeset, 0, replication_conn) - cached")

    // Fetch first, force scan
    data = test_planet.fetchDataFirst(replication::changeset, "", true);
    VERIFY(*data.get() == *initial_changeset_data.get(),
           "Underpass::fetchDataLast(replication::changeset, \"\", force_scan) "
           "- uncached)")

    data = test_planet.fetchDataFirst(replication::changeset);
    VERIFY(*data.get() == *initial_changeset_data.get(),
           "Underpass::fetchDataLast(replication::changeset, \"\") - uncached)")

    data = test_planet.fetchDataFirst(replication::changeset, replication_conn);
    VERIFY(*data.get() == *initial_changeset_data.get(),
           "Underpass::fetchDataLast(replication::changeset, replication_conn) "
           "- uncached)")

    data = test_planet.fetchDataFirst(replication::changeset, replication_conn);
    VERIFY(*data.get() == *initial_changeset_data.get(),
           "Underpass::fetchDataLast(replication::changeset, replication_conn) "
           "- cached)")

    // Fetch last
    // 4704925 was the last sequence when I wrote this test
    data = test_planet.fetchDataLast(replication::changeset);
    VERIFY(data->isValid() && data->sequence > 4621312,
           "Underpass::fetchDataLast(replication::changeset) - uncached)")

    data = test_planet.fetchDataLast(replication::changeset, replication_conn);
    VERIFY(
        data->isValid() && data->sequence > 4621312,
        "Underpass::fetchDataLast(replication::changeset) - replication_conn)")

    data = test_planet.fetchDataLast(replication::changeset, replication_conn);
    VERIFY(data->isValid() && data->sequence > 4621312,
           "Underpass::fetchDataLast(replication::changeset) - "
           "replication_conn) - cached")

    // Path is offset by one!
    data = test_planet.fetchData(replication::changeset, 3939092);
    VERIFY(data->isValid() && data->sequence == 3939092 &&
               data->path == "/003/939/093",
           "Underpass::fetchData(replication::changeset, 3939092)")

    data = test_planet.fetchData(replication::changeset,
                                 time_from_string("2020-05-17 15:45:01"),
                                 replication_conn);
    VERIFY(data->isValid() && data->sequence == 3939092,
           "Underpass::fetchData(replication::changeset, 3939092)")

    // //////////////////////////////////////////////////////////////////////
    // OsmChange tests
    //

    // Test scan
    const auto links{test_planet.scanDirectory("/replication/minute/000/000/")};
    VERIFY(links->size() > 0, "Underpass::scanDirectory()")

    // Try first state
    const auto initial_data = test_planet.fetchData(minutely, 1);
    VERIFY(initial_data->isValid() && initial_data->sequence == 1 &&
               initial_data->frequency == Underpass::freq_to_string(minutely) &&
               initial_data->path.compare("/000/000/001") == 0 &&
               to_iso_extended_string(initial_data->timestamp)
                       .compare("2012-09-12T08:15:45") == 0 &&
               !underpass.getFirstState(minutely)->isValid(),
           "Underpass::fetchData(minutely, 1)")

    // Store for later tests
    const auto initial_timestamp{initial_data->timestamp};

    // Try caching
    data = test_planet.fetchData(minutely, 1, replication_conn);
    VERIFY(*data.get() == *initial_data.get() &&
               underpass.getFirstState(minutely)->isValid(),
           "Underpass::fetchData(minutely, 1, replication_conn)")

    // Now retrieve the cached value
    data = test_planet.fetchData(minutely, 1, replication_conn);
    VERIFY(*data.get() == *initial_data.get() &&
               underpass.getFirstState(minutely)->isValid(),
           "Underpass::fetchData(minutely, 1, replication_conn) - cached")

    // Fetch first, force scan
    data = test_planet.fetchDataFirst(minutely, "", true);
    VERIFY(*data.get() == *initial_data.get(),
           "Underpass::fetchDataLast(minutely, \"\", force_scan) - uncached)")

    data = test_planet.fetchDataFirst(minutely);
    VERIFY(*data.get() == *initial_data.get(),
           "Underpass::fetchDataLast(minutely, \"\") - uncached)")

    data = test_planet.fetchDataFirst(minutely, replication_conn);
    VERIFY(*data.get() == *initial_data.get(),
           "Underpass::fetchDataLast(minutely, replication_conn) - uncached)")

    data = test_planet.fetchDataFirst(minutely, replication_conn);
    VERIFY(*data.get() == *initial_data.get(),
           "Underpass::fetchDataLast(minutely, replication_conn) - cached)")

    // Fetch last
    // 4704925 was the last sequence when I wrote this test
    data = test_planet.fetchDataLast(minutely);
    VERIFY(data->isValid() && data->sequence > 4704925,
           "Underpass::fetchDataLast(minutely) - uncached)")

    data = test_planet.fetchDataLast(minutely, replication_conn);
    VERIFY(data->isValid() && data->sequence > 4704925,
           "Underpass::fetchDataLast(minutely) - replication_conn)")

    data = test_planet.fetchDataLast(minutely, replication_conn);
    VERIFY(data->isValid() && data->sequence > 4704925,
           "Underpass::fetchDataLast(minutely) - replication_conn) - cached")

    // //////////////////////////////////////////////////////////////////////
    // Fetch from timestamp tests
    //

    data = test_planet.fetchDataGreaterThan(minutely, initial_timestamp);
    VERIFY(data->isValid() && data->sequence > 4704925,
           "Underpass::fetchDataGreaterThan(minutely, initial_timestamp)")

    // Now fetch sequence 3 and cache it
    test_planet.fetchData(minutely, 3, replication_conn);
    // Using cache, 3 is returned
    data = test_planet.fetchDataGreaterThan(minutely, initial_timestamp,
                                            replication_conn);
    VERIFY(data->isValid() && data->sequence == 3,
           "Underpass::fetchDataGreaterThan(minutely, initial_timestamp)")

    // Still using the cache, we ask for greater than 3's timestamp but sequence 4 wasn't cached so we expect the last
    data = test_planet.fetchDataGreaterThan(minutely, data->timestamp,
                                            replication_conn);
    VERIFY(data->isValid() && data->sequence > 4704925,
           "Underpass::fetchDataGreaterThan(minutely, initial_timestamp)")

    data = test_planet.fetchDataGreaterThan(minutely, 1);
    VERIFY(data->isValid() && data->sequence == 2,
           "Underpass::fetchDataGreaterThan(minutely, 1)")

    // Out of range
    data = test_planet.fetchDataGreaterThan(
        minutely, time_from_string("2500-12-01 00:00:00"));
    VERIFY(!data->isValid(),
           "Underpass::fetchDataGreaterThan(minutely, 2500-12-01 00:00:00)")

    // Lesser tests at this point we have sequences 1 and 3 in the cache
    // Out of range
    data = test_planet.fetchDataLessThan(minutely, 1);
    VERIFY(!data->isValid(), "Underpass::fetchDataLessThan(minutely, 1)")
    data = test_planet.fetchDataLessThan(minutely, initial_timestamp);
    VERIFY(!data->isValid(),
           "Underpass::fetchDataLessThan(minutely, initial_timestamp)")

    // Get state for initial (uncached)
    data = test_planet.fetchData(minutely, initial_timestamp);
    VERIFY(data->isValid() && data->sequence == 1 &&
               to_iso_extended_string(data->timestamp)
                       .compare("2012-09-12T08:15:45") == 0,
           "Underpass::fetchData(minutely, initial_timestamp)")

    // Get state for initial (cached)
    data = test_planet.fetchData(minutely, initial_timestamp, replication_conn);
    VERIFY(data->isValid() && data->sequence == 1 &&
               to_iso_extended_string(data->timestamp)
                       .compare("2012-09-12T08:15:45") == 0,
           "Underpass::fetchData(minutely, initial_timestamp) - cached")

    // Get the state for a given timestamp
    // https://planet.maps.mail.ru/replication/minute/004/025/013.state.txt
    /*
      #Sun May 17 15:47:04 UTC 2020
      sequenceNumber=4025013
      txnMaxQueried=2784168191
      txnActiveList=
      txnReadyList=
      txnMax=2784168191
      timestamp=2020-05-17T15:47:02Z

      Next ts for 4025014 is: 2020-05-17T15:48:02Z
    */

    data = test_planet.fetchData(minutely,
                                 time_from_string("2020-05-17 15:47:02"));
    VERIFY(data->isValid() && data->sequence == 4025013 &&
               to_iso_extended_string(data->timestamp)
                       .compare("2020-05-17T15:47:02") == 0,
           "Underpass::fetchData(minutely, 2020-05-17 15:47:02) - uncached")

    data = test_planet.fetchData(minutely,
                                 time_from_string("2020-05-17 15:47:02"));
    VERIFY(data->isValid() && data->sequence == 4025013 &&
               to_iso_extended_string(data->timestamp)
                       .compare("2020-05-17T15:47:02") == 0,
           "Underpass::fetchData(minutely, 2020-05-17 15:47:02) - uncached")

    data = test_planet.fetchData(
        minutely, time_from_string("2020-05-17 15:47:02"), replication_conn);
    VERIFY(
        data->isValid() && data->sequence == 4025013 &&
            to_iso_extended_string(data->timestamp)
                    .compare("2020-05-17T15:47:02") == 0,
        "Underpass::fetchData(minutely, 2020-05-17 15:47:02, replication_conn)")

    // Advance one second: should return next sequence
    data = test_planet.fetchData(minutely,
                                 time_from_string("2020-05-17 15:47:03"));
    VERIFY(data->isValid() && data->sequence == 4025014 &&
               to_iso_extended_string(data->timestamp)
                       .compare("2020-05-17T15:48:02") == 0,
           "Underpass::fetchData(minutely, 2020-05-17 15:47:03) - uncached")

    data = test_planet.fetchData(
        minutely, time_from_string("2020-05-17 15:47:03"), replication_conn);
    VERIFY(
        data->isValid() && data->sequence == 4025014 &&
            to_iso_extended_string(data->timestamp)
                    .compare("2020-05-17T15:48:02") == 0,
        "Underpass::fetchData(minutely, 2020-05-17 15:47:03, replication_conn)")

    data = test_planet.fetchData(
        minutely, time_from_string("2020-05-17 15:47:03"), replication_conn);
    VERIFY(data->isValid() && data->sequence == 4025014 &&
               to_iso_extended_string(data->timestamp)
                       .compare("2020-05-17T15:48:02") == 0,
           "Underpass::fetchData(minutely, 2020-05-17 15:47:03, "
           "replication_conn) - cached")

    // Random checks in the middle
    data = test_planet.fetchData(minutely,
                                 time_from_string("2016-05-17 15:47:03"));
    VERIFY(data->isValid() && data->sequence == 1924905,
           "Underpass::fetchData(minutely, 2016-05-17 15:47:03)");

    data = test_planet.fetchData(minutely,
                                 time_from_string("2021-09-07 15:47:03"));
    VERIFY(data->isValid() && data->sequence == 4704890,
           "Underpass::fetchData(minutely, 2021-09-07 15:47:03)");

#if 0
   // Timing tests (uncached)
    {
      TimeIt timer{"initial timestamp " + to_iso_extended_string(initial_timestamp)};
      data = test_planet.fetchData(minutely, initial_timestamp);
      VERIFY(data->isValid() && data->sequence == 1, "Timing test for initial timestamp " + to_iso_extended_string(initial_timestamp));
    }
    {
      TimeIt timer{"2020-05-17 15:47:03"};
      data = test_planet.fetchData(minutely, time_from_string("2020-05-17 15:47:03"));
      VERIFY(data->isValid() && data->sequence == 4025014, "Timing test for 2020-05-17 15:47:03");
    }
    {
      TimeIt timer{"2016-05-17 15:47:03"};
      data = test_planet.fetchData(minutely, time_from_string("2016-05-17 15:47:03"));
      VERIFY(data->isValid() && data->sequence == 1924905, "Timing test for 2016-05-17 15:47:03");
    }
    {
      TimeIt timer{"2021-09-07 15:47:03"};
      data = test_planet.fetchData(minutely, time_from_string("2021-09-07 15:47:03"));
      VERIFY(data->isValid() && data->sequence == 4704890, "Timing test for 2021-09-07 15:47:03");
    }
#endif
}
