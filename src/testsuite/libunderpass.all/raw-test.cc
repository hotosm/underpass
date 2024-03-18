//
// Copyright (c) 2024 Humanitarian OpenStreetMap Team
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
#include "utils/log.hh"
#include "osm/osmobjects.hh"
#include "raw/queryraw.hh"
#include <string.h>
#include "replicator/replication.hh"
#include <boost/geometry.hpp>

using namespace replication;
using namespace logger;
using namespace queryraw;

TestState runtest;
class TestOsmChange : public osmchange::OsmChangeFile {};

class TestPlanet : public Planet {
  public:
    TestPlanet() = default;

    //! Clear the test DB and fill it with with initial test data
    bool init_test_case(const std::string &dbconn)
    {
        source_tree_root = getenv("UNDERPASS_SOURCE_TREE_ROOT")
                               ? getenv("UNDERPASS_SOURCE_TREE_ROOT")
                               : "../";

        const std::string test_replication_db_name{"underpass_test"};
        {
            pqxx::connection conn{dbconn + " dbname=template1"};
            pqxx::nontransaction worker{conn};
            worker.exec0("DROP DATABASE IF EXISTS " + test_replication_db_name);
            worker.exec0("CREATE DATABASE " + test_replication_db_name);
            worker.commit();
        }

        {
            pqxx::connection conn{dbconn + " dbname=" + test_replication_db_name};
            pqxx::nontransaction worker{conn};
            worker.exec0("CREATE EXTENSION postgis");
            worker.exec0("CREATE EXTENSION hstore");

            // Create schema
            const auto schema_path{source_tree_root + "setup/db/underpass.sql"};
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

bool processFile(const std::string &filename, std::shared_ptr<Pq> &db) {
    auto queryraw = std::make_shared<QueryRaw>(db);
    auto osmchanges = std::make_shared<osmchange::OsmChangeFile>();
    std::string destdir_base = DATADIR;
    multipolygon_t poly;
    osmchanges->readChanges(destdir_base + "/testsuite/testdata/raw/" + filename);
    queryraw->buildGeometries(osmchanges, poly);
    osmchanges->areaFilter(poly);
    std::string rawquery;

    for (auto it = std::begin(osmchanges->changes); it != std::end(osmchanges->changes); ++it) {
        osmchange::OsmChange *change = it->get();
        // Nodes
        for (auto nit = std::begin(change->nodes); nit != std::end(change->nodes); ++nit) {
            osmobjects::OsmNode *node = nit->get();
            rawquery += queryraw->applyChange(*node);
        }
        // Ways
        for (auto wit = std::begin(change->ways); wit != std::end(change->ways); ++wit) {
            osmobjects::OsmWay *way = wit->get();
            rawquery += queryraw->applyChange(*way);
        }
        // Relations
        for (auto rit = std::begin(change->relations); rit != std::end(change->relations); ++rit) {
            osmobjects::OsmRelation *relation = rit->get();
            rawquery += queryraw->applyChange(*relation);
        }
    }
    db->query(rawquery);
}

const std::vector<std::string> expectedGeometries = {
    "POLYGON((21.726001473 4.62042952837,21.726086573 4.62042742837,21.726084973 4.62036492836,21.725999873 4.62036702836,21.726001473 4.62042952837))",
    "POLYGON((21.726001473 4.62042952837,21.726086573 4.62042742837,21.726084973 4.62036492836,21.725999873 4.62036702836,21.726001473 4.62042952837))",
    "POLYGON((21.72600148 4.62042953,21.726086573 4.62042742837,21.726084973 4.62036492836,21.725999873 4.62036702836,21.72600148 4.62042953))",
    "MULTIPOLYGON(((21.72600148 4.62042953,21.726086573 4.62042742837,21.726084973 4.62036492836,21.725999873 4.62036702836,21.72600148 4.62042953),(21.7260170728 4.62041343508,21.7260713875 4.62041326798,21.7260708846 4.62037684165,21.7260165699 4.62038035061,21.7260170728 4.62041343508)))",
    "MULTIPOLYGON(((21.72600148 4.62042953,21.726086573 4.62042742837,21.7260807753 4.62037032501,21.725999873 4.62036702836,21.72600148 4.62042953),(21.7260170728 4.62041343508,21.7260713875 4.62041326798,21.7260708846 4.62037684165,21.7260165699 4.62038035061,21.7260170728 4.62041343508)))",
    "MULTILINESTRING((21.726001473 4.62042952837,21.726086573 4.62042742837,21.726084973 4.62036492836),(21.726084973 4.62036492836,21.725999873 4.62036702836,21.726001473 4.62042952837))",
    "MULTIPOLYGON(((-59.8355946 -36.7448782,-59.8353203 -36.7452085,-59.8368532 -36.7480926,-59.8370667 -36.7483355,-59.8408639 -36.7515154,-59.8482593 -36.745907,-59.8486299 -36.7452032,-59.848532 -36.7453269,-59.8483958 -36.7456167,-59.8482593 -36.745907,-59.8486299 -36.7452032,-59.8486213 -36.7451521,-59.8481455 -36.7432981,-59.8478834 -36.7425679,-59.8478326 -36.7423963,-59.8478326 -36.7423963,-59.8477902 -36.7422529,-59.8477764 -36.7422007,-59.8477733 -36.74215,-59.8477773 -36.742095,-59.8477972 -36.7420329,-59.8478247 -36.7419783,-59.8478609 -36.7419236,-59.8479126 -36.7418808,-59.84795 -36.7418607,-59.8480462 -36.7418147,-59.8480692 -36.7417819,-59.8480709 -36.7417548,-59.8480673 -36.7416893,-59.8479835 -36.741163,-59.8479787 -36.7411211,-59.8479562 -36.7410819,-59.847923 -36.7410414,-59.8478722 -36.7410094,-59.8475273 -36.7408127,-59.8474352 -36.7407496,-59.8473715 -36.7406912,-59.8473254 -36.7406329,-59.8472829 -36.7405588,-59.8472494 -36.7404962,-59.8472589 -36.7404251,-59.8472978 -36.7403582,-59.8473424 -36.74032,-59.8473864 -36.7402921,-59.8474375 -36.740274,-59.8474922 -36.740256,-59.8475468 -36.7402465,-59.8476241 -36.7402304,-59.8476784 -36.7402171,-59.8477563 -36.7401919,-59.8478265 -36.7401545,-59.8479128 -36.7400779,-59.8479937 -36.7399885,-59.8480801 -36.7398796,-59.8481504 -36.7397665,-59.848411 -36.7391216,-59.8484023 -36.739028,-59.8483557 -36.7389343,-59.8482783 -36.7388702,-59.84822 -36.7388242,-59.8481308 -36.7388079,-59.8480003 -36.7387583,-59.8478575 -36.7386841,-59.8477933 -36.738618,-59.8477365 -36.7385158,-59.8477106 -36.7384235,-59.8477053 -36.7382963,-59.8477134 -36.7381998,-59.8477433 -36.7381126,-59.8478321 -36.738022,-59.8479105 -36.7379644,-59.8480011 -36.7379216,-59.8482127 -36.7378122,-59.8482877 -36.7377698,-59.8483566 -36.7377126,-59.848393 -36.737632,-59.8484294 -36.7375366,-59.8485761 -36.7372026,-59.848605 -36.7370773,-59.8486246 -36.7369372,-59.8486196 -36.7368366,-59.8485807 -36.7367015,-59.848529 -36.7365836,-59.8484717 -36.7364835,-59.8483887 -36.7363497,-59.8477548 -36.7356502,-59.8477339 -36.7356248,-59.8477339 -36.7356248,-59.8475634 -36.7357007,-59.8474292 -36.7357691,-59.8473073 -36.7358571,-59.8469617 -36.7361243,-59.8447338 -36.737825,-59.8424572 -36.7395354,-59.8423067 -36.7396527,-59.8386641 -36.7424968,-59.838225 -36.7428388,-59.8355946 -36.7448782)))"
};

std::string
getWKT(const polygon_t &polygon) {
    std::stringstream ss;
    ss << std::setprecision(12) << boost::geometry::wkt(polygon);
    return ss.str();
}

std::string
getWKTFromDB(const std::string &table, const long id, std::shared_ptr<Pq> &db) {
    auto result = db->query("SELECT ST_AsText(geom, 4326), refs from relations where osm_id=" + std::to_string(id));
    for (auto r_it = result.begin(); r_it != result.end(); ++r_it) {
        return (*r_it)[0].as<std::string>();
    }
}

int
main(int argc, char *argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("raw-test.log");
    dbglogfile.setVerbosity(3);

    const std::string dbconn{getenv("UNDERPASS_TEST_DB_CONN")
                                    ? getenv("UNDERPASS_TEST_DB_CONN")
                                    : "user=underpass_test host=localhost password=underpass_test"};

    TestPlanet test_planet;
    test_planet.init_test_case(dbconn);
    auto db = std::make_shared<Pq>();

    if (db->connect(dbconn + " dbname=underpass_test")) {
        auto queryraw = std::make_shared<QueryRaw>(db);
        std::map<long, std::shared_ptr<osmobjects::OsmWay>> waycache;

        processFile("raw-case-1.osc", db);
        processFile("raw-case-2.osc", db);

        std::string waysIds = "101874,101875";
        queryraw->getWaysByIds(waysIds, waycache);

        // 4 created Nodes, 1 created Way (same changeset)
        if ( getWKT(waycache.at(101874)->polygon).compare(expectedGeometries[0]) == 0) {
            runtest.pass("4 created Nodes, 1 created Ways (same changeset)");
        } else {
            runtest.fail("4 created Nodes, 1 created Ways (same changeset)");
            return 1;
        }

        // 1 created Way, 4 existing Nodes (different changeset)
        if ( getWKT(waycache.at(101875)->polygon).compare(expectedGeometries[1]) == 0) {
            runtest.pass("1 created Way, 4 existing Nodes (different changesets)");
        } else {
            runtest.fail("1 created Way, 4 existing Nodes (different changesets)");
            return 1;
        }

        // 1 modified node, indirectly modify other existing ways
        processFile("raw-case-3.osc", db);
        waycache.erase(101875);
        queryraw->getWaysByIds(waysIds, waycache);
        if ( getWKT(waycache.at(101875)->polygon).compare(expectedGeometries[2]) == 0) {
            runtest.pass("1 modified Node, indirectly modify other existing Ways (different changesets)");
        } else {
            runtest.fail("1 modified Node, indirectly modify other existing Ways (different changesets)");
            return 1;
        }

        // 1 created Relation referencing 1 created Way and 1 existing Way
        processFile("raw-case-4.osc", db);
        if ( getWKTFromDB("relations", 211766, db).compare(expectedGeometries[3]) == 0) {
            runtest.pass("1 created Relation referencing 1 created Way and 1 existing Way (different changesets)");
        } else {
            runtest.fail("1 created Relation referencing 1 created Way and 1 existing Way (different changesets)");
            return 1;
        }

        // 1 modified Node, indirectly modify other existing Ways and 1 Relation
        processFile("raw-case-5.osc", db);
        if ( getWKTFromDB("relations", 211766, db).compare(expectedGeometries[4]) == 0) {
            runtest.pass("1 modified Node, indirectly modify other existing Ways and 1 Relation (different changesets)");
        } else {
            runtest.fail("1 modified Node, indirectly modify other existing Ways and 1 Relation (different changesets)");
            return 1;
        }

        // 4 created Nodes, 2 created Ways, 1 created Relation with type=multilinestring
        processFile("raw-case-6.osc", db);
        if ( getWKTFromDB("relations", 211776, db).compare(expectedGeometries[5]) == 0) {
            runtest.pass("4 created Nodes, 2 created Ways, 1 created Relation with type=multilinestring (same changeset)");
        } else {
            runtest.fail("4 created Nodes, 2 created Ways, 1 created Relation with type=multilinestring (same changeset)");
            return 1;
        }

        // Complex, 1 polygon relation made of multiple ways
        processFile("raw-case-7.osc", db);
        if ( getWKTFromDB("relations", 17331328, db).compare(expectedGeometries[6]) == 0) {
            runtest.pass("Complex, 1 polygon relation made of multiple ways (same changeset)");
        } else {
            runtest.fail("Complex, 1 polygon relation made of multiple ways (same changeset)");
            return 1;
        }


    }



}