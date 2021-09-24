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

#include "data/osm2pgsql.hh"
#include "hottm.hh"
#include "log.hh"

#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

using namespace osm2pgsql;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::filesystem;

TestState runtest;

#define VERIFY(condition, message)                                                                                                 \
    if (condition) {                                                                                                               \
        runtest.pass(message);                                                                                                     \
    } else {                                                                                                                       \
        runtest.fail(message);                                                                                                     \
        std::cerr << "Failing at: " << __FILE__ << ':' << __LINE__ << std::endl;                                                   \
        exit(EXIT_FAILURE);                                                                                                        \
    }

#define COMPARE(first, second, message)                                                                                            \
    if (first == second) {                                                                                                         \
        runtest.pass(message);                                                                                                     \
    } else {                                                                                                                       \
        runtest.fail(message);                                                                                                     \
        std::cerr << "Failing at: " << __FILE__ << ':' << __LINE__ << std::endl;                                                   \
        std::cerr << "Values are not equal: " << first << " != " << second << std::endl;                                           \
        exit(EXIT_FAILURE);                                                                                                        \
    }

class TestOsm2Pgsql : public Osm2Pgsql {
  public:
    TestOsm2Pgsql() = default;

    //! Clear the test DB and fill it with with initial test data
    bool init_test_case()
    {

        logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
        dbglogfile.setVerbosity();

        const std::string dbconn{getenv("UNDERPASS_TEST_DB_CONN") ? getenv("UNDERPASS_TEST_DB_CONN") : ""};
        source_tree_root = getenv("UNDERPASS_SOURCE_TREE_ROOT") ? getenv("UNDERPASS_SOURCE_TREE_ROOT") : SRCDIR;

        const std::string test_osm2pgsql_db_name{"osm2pgsql_test"};

        {
            pqxx::connection conn{dbconn};
            pqxx::nontransaction worker{conn};
            worker.exec0("DROP DATABASE IF EXISTS " + test_osm2pgsql_db_name);
            worker.exec0("CREATE DATABASE " + test_osm2pgsql_db_name);
            worker.commit();
        }

        {
            pqxx::connection conn{dbconn + " dbname=" + test_osm2pgsql_db_name};
            pqxx::nontransaction worker{conn};
            worker.exec0("CREATE EXTENSION postgis");
            worker.exec0("CREATE EXTENSION hstore");

            // Create schema
            const path base_path{source_tree_root / "testsuite"};
            const auto schema_path{base_path / "testdata" / "pgsql_test_schema.sql"};
            std::ifstream schema_definition(schema_path);
            std::string sql((std::istreambuf_iterator<char>(schema_definition)), std::istreambuf_iterator<char>());

            assert(!sql.empty());
            worker.exec0(sql);

            // Load a minimal data set for testing
            const auto data_path{base_path / "testdata" / "pgsql_test_data.sql.gz"};
            std::ifstream data_definition(data_path, std::ios_base::in | std::ios_base::binary);
            boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            inbuf.push(data_definition);
            std::istream instream(&inbuf);
            std::string data_sql((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
            assert(!data_sql.empty());
            worker.exec0(data_sql);
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

    TestOsm2Pgsql testosm2pgsql;

    // Test that default constructed object have a schema set
    assert(testosm2pgsql.getSchema().compare(testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME) == 0);

    testosm2pgsql.init_test_case();

    const std::string test_osm2pgsql_db_name{"osm2pgsql_test"};
    std::string osm2pgsql_conn;
    if (getenv("PGHOST") && getenv("PGUSER") && getenv("PGPASSWORD")) {
        osm2pgsql_conn = std::string(getenv("PGUSER")) + ":" + std::string(getenv("PGPASSWORD")) + "@" +
                         std::string(getenv("PGHOST")) + "/" + test_osm2pgsql_db_name;
    } else {
        osm2pgsql_conn = test_osm2pgsql_db_name;
    }

    VERIFY(testosm2pgsql.connect(osm2pgsql_conn), "Osm2Pgsql::connect()");

    const auto last_update{testosm2pgsql.getLastUpdate()};

    VERIFY(!last_update.is_not_a_date_time() && to_iso_extended_string(last_update) == "2021-08-05T23:38:28",
           "Osm2Pgsql::getLastUpdate()");

    // Read OSC from file
    std::ifstream ifs(testosm2pgsql.source_tree_root / "testsuite" / "testdata" / "test_change.osc");
    const std::string xml{std::istreambuf_iterator<char>{ifs}, {}};

    // Test update from osm changes
    std::shared_ptr<OsmChangeFile> osm_changes = std::make_shared<OsmChangeFile>();
    std::stringstream xml_stream{xml};
    osm_changes->readXML(xml_stream);

    auto results{testosm2pgsql.query("SELECT ST_Contains(ST_MakeLine(way), ST_SetSRID(ST_MakePoint(2.4060, 49.590), 4326)) FROM " +
                                     testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_roads WHERE osm_id = 1")};

    VERIFY(results.at(0)["st_contains"].as(std::string()) == "t",
           "OsmChangeFile::readXML(xml_stream) - verify initial road 1 geometry");

    // Test that semaphore is in points
    results = testosm2pgsql.query("SELECT osm_id FROM " + testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME +
                                  ".planet_osm_point WHERE osm_id = 10");

    COMPARE(results.size(), 1, "OsmChangeFile::readXML(xml_stream) - verify traffic light 10 is in points");

    // Read the osm change and process it
    if (!testosm2pgsql.updateDatabase(osm_changes)) {
        //if (!testosm2pgsql.updateDatabase(xml)) {
        runtest.fail("Osm2Pgsql::updateDatabase()");
        exit(EXIT_FAILURE);
    } else {
        // Check for changes!
        results = testosm2pgsql.query(
            "SELECT osm_id, name, ST_X(way) AS x, shop, tags -> 'crazy=>tag' as crazy, \"addr:housename\" FROM " +
            testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_point ORDER BY name");

        COMPARE(results.at(0)["name"].as(std::string()), "Some interesting point new name",
                "Osm2Pgsql::updateDatabase() - retrieve 0");

        COMPARE(results.at(1)["name"].as(std::string()), "Some other interesting point (44)",
                "Osm2Pgsql::updateDatabase() - retrieve 1");

        COMPARE(results.at(1)["shop"].as(std::string()), "department_store", "Osm2Pgsql::updateDatabase() - retrieve shop 44");

        COMPARE(results.at(1)["osm_id"].as(int()), 44, "Osm2Pgsql::updateDatabase() - retrieve shop id 44");

        COMPARE(results.at(1)["addr:housename"].as(std::string()), "My house name at N°5",
                "Osm2Pgsql::updateDatabase() - retrieve 1 addr:housename");

        COMPARE(results.at(1)["crazy"].as(std::string()), "crazy'\"=>values", "Osm2Pgsql::updateDatabase() - retrieve 1 tags");

        COMPARE(results.at(1)["osm_id"].as(int()), 44, "Osm2Pgsql::updateDatabase() - retrieve osm_id 44");

        COMPARE(results.at(0)["x"].as(double()), 3.12, "Osm2Pgsql::updateDatabase() - retrieve X 0");

        COMPARE(results.at(1)["x"].as(double()), 3.0, "Osm2Pgsql::updateDatabase() - retrieve X 1");

        results =
            testosm2pgsql.query("SELECT * FROM " + testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_nodes WHERE id = 11");

        COMPARE(results.size(), 0, "Osm2Pgsql::updateDatabase() - verify node 11 is gone from points");

        // Verify way 2 which has been altered
        results =
            testosm2pgsql.query("SELECT * FROM " + testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_ways WHERE id = 2");

        // verify node 11 is gone
        COMPARE(results.at(0)["nodes"].as(std::string()), "{1,4,2,5,1}",
                "Osm2Pgsql::updateDatabase() - verify changed way 2 nodes");

        results = testosm2pgsql.query("SELECT ST_Contains(ST_MakeLine(way), ST_SetSRID(ST_MakePoint(2.4060, 49.590), 4326)) FROM " +
                                      testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_roads WHERE osm_id = 1");

        VERIFY(results.at(0)["st_contains"].as(std::string()) == "f",
               "OsmChangeFile::updateDatabase() - verify final road 1 geometry");

        // Test that semaphore was removed from points
        results = testosm2pgsql.query("SELECT osm_id FROM " + testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME +
                                      ".planet_osm_point WHERE osm_id = 10");

        COMPARE(results.size(), 0, "OsmChangeFile::updateDatabase() - verify traffic light 10 is gone");

        // Check individual polygons from ways
        results = testosm2pgsql.query("SELECT ST_AsText(way), \"natural\", tags FROM " +
                                      testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_polygon WHERE osm_id = 201");

        COMPARE(results.at(0)["st_astext"].as(std::string()), "POLYGON((1 50,1 50,1 49,1 49,1 50))",
                "Osm2Pgsql::updateDatabase() - verify new polygon geom");

        COMPARE(results.at(0)["natural"].as(std::string()), "wood", "Osm2Pgsql::updateDatabase() - verify new polygon natural");

        // Check multipolygons from relations
        results =
            testosm2pgsql.query("SELECT ST_AsText(way), \"natural\", tags FROM " + testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME +
                                ".planet_osm_polygon WHERE \"natural\" = 'grassland'");

        // verify grassland
        COMPARE(results.at(0)["st_astext"].as(std::string()),
                "POLYGON((1.7 49.5,1.9 49.5,1.9 50,1.7 50,1.7 49.5),(1.75 49.6,1.75 49.9,1.85 49.9,1.85 49.6,1.75 49.6))",
                "Osm2Pgsql::updateDatabase() - verify new multipolygon geom");

        // verify grassland relation
        results =
            testosm2pgsql.query("SELECT * FROM " + testosm2pgsql.OSM2PGSQL_DEFAULT_SCHEMA_NAME + ".planet_osm_rels WHERE id = 6");

        COMPARE(results.at(0)["rel_off"].as(int()), 2, "Osm2Pgsql::updateDatabase() - verify new multipolygon rel 6 rel_off");
        COMPARE(results.at(0)["parts"].as(std::string()), "{201,202}",
                "Osm2Pgsql::updateDatabase() - verify new multipolygon rel 6 parts");
        COMPARE(results.at(0)["members"].as(std::string()), "{w201,outer,w202,inner}",
                "Osm2Pgsql::updateDatabase() - verify new multipolygon rel 6 parts");
    }
}
