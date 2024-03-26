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
    "MULTIPOLYGON(((-59.8355946 -36.7448782,-59.8353203 -36.7452085,-59.8368532 -36.7480926,-59.8370667 -36.7483355,-59.8408639 -36.7515154,-59.8482593 -36.745907,-59.8486299 -36.7452032,-59.848532 -36.7453269,-59.8483958 -36.7456167,-59.8482593 -36.745907,-59.8486299 -36.7452032,-59.8486213 -36.7451521,-59.8481455 -36.7432981,-59.8478834 -36.7425679,-59.8478326 -36.7423963,-59.8478326 -36.7423963,-59.8477902 -36.7422529,-59.8477764 -36.7422007,-59.8477733 -36.74215,-59.8477773 -36.742095,-59.8477972 -36.7420329,-59.8478247 -36.7419783,-59.8478609 -36.7419236,-59.8479126 -36.7418808,-59.84795 -36.7418607,-59.8480462 -36.7418147,-59.8480692 -36.7417819,-59.8480709 -36.7417548,-59.8480673 -36.7416893,-59.8479835 -36.741163,-59.8479787 -36.7411211,-59.8479562 -36.7410819,-59.847923 -36.7410414,-59.8478722 -36.7410094,-59.8475273 -36.7408127,-59.8474352 -36.7407496,-59.8473715 -36.7406912,-59.8473254 -36.7406329,-59.8472829 -36.7405588,-59.8472494 -36.7404962,-59.8472589 -36.7404251,-59.8472978 -36.7403582,-59.8473424 -36.74032,-59.8473864 -36.7402921,-59.8474375 -36.740274,-59.8474922 -36.740256,-59.8475468 -36.7402465,-59.8476241 -36.7402304,-59.8476784 -36.7402171,-59.8477563 -36.7401919,-59.8478265 -36.7401545,-59.8479128 -36.7400779,-59.8479937 -36.7399885,-59.8480801 -36.7398796,-59.8481504 -36.7397665,-59.848411 -36.7391216,-59.8484023 -36.739028,-59.8483557 -36.7389343,-59.8482783 -36.7388702,-59.84822 -36.7388242,-59.8481308 -36.7388079,-59.8480003 -36.7387583,-59.8478575 -36.7386841,-59.8477933 -36.738618,-59.8477365 -36.7385158,-59.8477106 -36.7384235,-59.8477053 -36.7382963,-59.8477134 -36.7381998,-59.8477433 -36.7381126,-59.8478321 -36.738022,-59.8479105 -36.7379644,-59.8480011 -36.7379216,-59.8482127 -36.7378122,-59.8482877 -36.7377698,-59.8483566 -36.7377126,-59.848393 -36.737632,-59.8484294 -36.7375366,-59.8485761 -36.7372026,-59.848605 -36.7370773,-59.8486246 -36.7369372,-59.8486196 -36.7368366,-59.8485807 -36.7367015,-59.848529 -36.7365836,-59.8484717 -36.7364835,-59.8483887 -36.7363497,-59.8477548 -36.7356502,-59.8477339 -36.7356248,-59.8477339 -36.7356248,-59.8475634 -36.7357007,-59.8474292 -36.7357691,-59.8473073 -36.7358571,-59.8469617 -36.7361243,-59.8447338 -36.737825,-59.8424572 -36.7395354,-59.8423067 -36.7396527,-59.8386641 -36.7424968,-59.838225 -36.7428388,-59.8355946 -36.7448782)))",
    "MULTIPOLYGON(((-69.0344971 -33.6875005,-69.0354574 -33.6880228,-69.0356076 -33.6879112,-69.0360421 -33.6881656,-69.0362352 -33.6883218,-69.0369111 -33.6886075,-69.0375173 -33.6871078,-69.0367931 -33.686581,-69.0366161 -33.6866525,-69.0361923 -33.6865766,-69.0364122 -33.6860945,-69.0368253 -33.68634,-69.0370399 -33.6864739,-69.037512 -33.6861168,-69.0374959 -33.6859115,-69.0376568 -33.6857687,-69.037866 -33.6856838,-69.037351 -33.6853401,-69.0371311 -33.6842242,-69.0365088 -33.68376,-69.0362889 -33.6832065,-69.036144 -33.6823673,-69.0358865 -33.6818227,-69.0358973 -33.6817468,-69.03557 -33.6815816,-69.0359187 -33.6810459,-69.0351462 -33.6804031,-69.0349263 -33.6798541,-69.0345454 -33.67968,-69.0342611 -33.6794791,-69.0337515 -33.6794836,-69.0332151 -33.6793095,-69.0331185 -33.6788586,-69.0329737 -33.6786354,-69.0327162 -33.6783541,-69.0325767 -33.6782024,-69.0321851 -33.6780238,-69.0315521 -33.6776756,-69.0312892 -33.6774881,-69.0311068 -33.6773453,-69.0308118 -33.6771131,-69.0305275 -33.6769747,-69.0303397 -33.6769033,-69.0300447 -33.676948,-69.0298086 -33.6769256,-69.0288216 -33.6784256,-69.0287519 -33.6783742,-69.0296692 -33.6768676,-69.0292937 -33.6765819,-69.0289557 -33.6765596,-69.0286982 -33.6766355,-69.0284568 -33.6765685,-69.0282691 -33.6768363,-69.0279794 -33.6769122,-69.0277594 -33.6767917,-69.0276629 -33.6770015,-69.0274376 -33.6768988,-69.0273035 -33.6768899,-69.0270782 -33.6770328,-69.0268743 -33.6769301,-69.0265229 -33.6775261,-69.0258417 -33.6786086,-69.0233284 -33.6774703,-69.0232372 -33.6775238,-69.0232211 -33.6781176,-69.0231353 -33.6783408,-69.0233445 -33.6788229,-69.0233767 -33.6791979,-69.0232158 -33.6794925,-69.0233231 -33.6796264,-69.022733 -33.6803049,-69.0227705 -33.6806933,-69.0227598 -33.6811084,-69.0223092 -33.6817245,-69.0238702 -33.6824744,-69.0240419 -33.6822735,-69.0255761 -33.6830234,-69.0254581 -33.6832243,-69.0291756 -33.6849295,-69.0293527 -33.6846617,-69.0311283 -33.6854607,-69.0308976 -33.6858222,-69.0337139 -33.6871256,-69.0343523 -33.6862061,-69.0349907 -33.6865409,-69.0343738 -33.6874604,-69.0344971 -33.6875005),(-69.0305328 -33.683019,-69.0311444 -33.6821173,-69.0274215 -33.6803451,-69.0263647 -33.6819119,-69.0305328 -33.683019)))",
    "MULTIPOLYGON(((-70.3434661 -38.5210725,-70.3434387 -38.5210955,-70.3434261 -38.5211299,-70.3434276 -38.5211716,-70.3434429 -38.5212067,-70.3434724 -38.5212373,-70.3435173 -38.5212595,-70.3435951 -38.5212788,-70.3436942 -38.5212955,-70.3438252 -38.52131479999999,-70.3439597 -38.521337,-70.3441048 -38.521359100000005,-70.3442463 -38.52138410000001,-70.3443808 -38.5214145,-70.3445082 -38.5214533,-70.3446177 -38.521495599999994,-70.3447356 -38.52154330000001,-70.3448535 -38.52158469999999,-70.3449795 -38.5216101,-70.3451015 -38.521622900000004,-70.3452113 -38.52164189999999,-70.3453088 -38.5216674,-70.3454192 -38.521701,-70.3455168 -38.5217296,-70.3456184 -38.5217582,-70.3457445 -38.5217805,-70.3458786 -38.52179,-70.3460209 -38.52179,-70.3471381 -38.5217576,-70.3472051 -38.52175339999999,-70.3472561 -38.52174300000001,-70.3473044 -38.52172410000001,-70.3473446 -38.521701,-70.3473795 -38.52166949999999,-70.3474116 -38.5216422,-70.3474492 -38.521612800000014,-70.3474921 -38.5215835,-70.3475323 -38.5215541,-70.3476101 -38.52149739999999,-70.3475739 -38.52162409999999,-70.3475456 -38.521693299999995,-70.3475137 -38.52176259999999,-70.3474677 -38.5218235,-70.3474253 -38.5218674,-70.3473686 -38.52190379999999,-70.3473085 -38.5219342,-70.3472413 -38.521954,-70.347174 -38.5219675,-70.3472817 -38.5219659,-70.3474005 -38.5219536,-70.3475173 -38.52192869999999,-70.3476447 -38.5218955,-70.3477721 -38.52184559999999,-70.3479455 -38.52176259999999,-70.3479066 -38.52183180000001,-70.3478641 -38.5218789,-70.3478075 -38.52192869999999,-70.3477355 -38.5219766,-70.3479018 -38.5219262,-70.3480467 -38.5218675,-70.3481915 -38.521791900000004,-70.3483042 -38.52172060000001,-70.3484135 -38.521645,-70.3486227 -38.5215149,-70.3488534 -38.52138899999999,-70.3491002 -38.5212799,-70.3493684 -38.5211918,-70.3496098 -38.5211162,-70.3498834 -38.5210491,-70.3501301 -38.5210029,-70.3503447 -38.5209693,-70.3505593 -38.520944100000015,-70.3507792 -38.5209204,-70.3507792 -38.5209204,-70.3505605 -38.52082279999999,-70.350236 -38.5206906,-70.3498659 -38.5205962,-70.3495735 -38.5205458,-70.3492637 -38.5204975,-70.3491282 -38.52049329999999,-70.3490075 -38.520505899999996,-70.3488064 -38.5205469,-70.3488064 -38.5205469,-70.3485167 -38.5206098,-70.3478837 -38.5207179,-70.3473875 -38.5207808,-70.3468376 -38.5208333,-70.3461456 -38.5208816,-70.3455448 -38.520934,-70.3449628 -38.5209949,-70.3443834 -38.52103060000001,-70.343796 -38.52105360000001,-70.3434661 -38.5210725)),((-70.3393449 -38.5210096,-70.3397998 -38.5209924,-70.3400091 -38.5209944,-70.3401816 -38.521013700000005,-70.3403726 -38.5210619,-70.340539 -38.52112459999999,-70.3406807 -38.5211969,-70.3408225 -38.5212789,-70.3409704 -38.5213898,-70.3410954 -38.521514500000016,-70.341237 -38.5216474,-70.3414069 -38.5217803,-70.3415626 -38.52188550000001,-70.3417466 -38.52196860000001,-70.3419731 -38.5220406,-70.3421855 -38.522096,-70.3423908 -38.5221458,-70.3426173 -38.5222012,-70.3428155 -38.522262099999985,-70.3430137 -38.52233410000001,-70.3432119 -38.5224338,-70.3433534 -38.5225058,-70.343495 -38.5225889,-70.3436649 -38.522660899999984,-70.3438772 -38.5226885,-70.3440683 -38.522682999999994,-70.3442524 -38.522660899999984,-70.3449444 -38.522532899999995,-70.3450571 -38.5224952,-70.3451644 -38.522428000000005,-70.3452341 -38.5223483,-70.3452663 -38.5222769,-70.3452717 -38.52220139999999,-70.3452448 -38.5221342,-70.3451966 -38.5220797,-70.3451 -38.5220335,-70.3449444 -38.52198729999999,-70.3447513 -38.52195369999999,-70.3445624 -38.52194130000001,-70.343915 -38.52190679999999,-70.3437005 -38.5218859,-70.3434859 -38.5218565,-70.3432337 -38.5217935,-70.3429923 -38.521717999999986,-70.3427402 -38.5216215,-70.3425364 -38.5215333,-70.3423486 -38.5214368,-70.3421877 -38.521336100000006,-70.3420589 -38.5212353,-70.341948 -38.5211271,-70.341948 -38.5211271,-70.3415966 -38.5210893,-70.3412157 -38.521032700000006,-70.3409931 -38.5209875,-70.3408737 -38.5209414,-70.3406967 -38.5208417,-70.3405653 -38.5207452,-70.3404205 -38.5206245,-70.3402944 -38.520521699999996,-70.3401375 -38.52040730000002,-70.3399175 -38.520231,-70.3397392 -38.5200925,-70.3396024 -38.5200075,-70.3394187 -38.519918299999986,-70.3392309 -38.519868,-70.3388715 -38.5197882,-70.3384933 -38.5197012,-70.3380534 -38.5196015,-70.3377664 -38.519549000000005,-70.3372943 -38.519500699999995,-70.3368531 -38.5194703,-70.3365098 -38.5194577,-70.3361759 -38.5194399,-70.3359184 -38.5194567,-70.3359184 -38.5194567,-70.3362585 -38.519638999999984,-70.3365107 -38.5197779,-70.3367348 -38.51993860000001,-70.3369777 -38.520114,-70.3372112 -38.5202456,-70.3376501 -38.5204356,-70.3381078 -38.520632899999995,-70.3384721 -38.5208009,-70.3387149 -38.520874,-70.3390231 -38.52093250000001,-70.3393449 -38.5210096)))"
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

        // processFile("raw-case-1.osc", db);
        // processFile("raw-case-2.osc", db);

        // std::string waysIds = "101874,101875";
        // queryraw->getWaysByIds(waysIds, waycache);

        // // 4 created Nodes, 1 created Way (same changeset)
        // if ( getWKT(waycache.at(101874)->polygon).compare(expectedGeometries[0]) == 0) {
        //     runtest.pass("4 created Nodes, 1 created Ways (same changeset)");
        // } else {
        //     runtest.fail("4 created Nodes, 1 created Ways (same changeset)");
        //     return 1;
        // }

        // // 1 created Way, 4 existing Nodes (different changeset)
        // if ( getWKT(waycache.at(101875)->polygon).compare(expectedGeometries[1]) == 0) {
        //     runtest.pass("1 created Way, 4 existing Nodes (different changesets)");
        // } else {
        //     runtest.fail("1 created Way, 4 existing Nodes (different changesets)");
        //     return 1;
        // }

        // // 1 modified node, indirectly modify other existing ways
        // processFile("raw-case-3.osc", db);
        // waycache.erase(101875);
        // queryraw->getWaysByIds(waysIds, waycache);
        // if ( getWKT(waycache.at(101875)->polygon).compare(expectedGeometries[2]) == 0) {
        //     runtest.pass("1 modified Node, indirectly modify other existing Ways (different changesets)");
        // } else {
        //     runtest.fail("1 modified Node, indirectly modify other existing Ways (different changesets)");
        //     return 1;
        // }

        // // 1 created Relation referencing 1 created Way and 1 existing Way
        // processFile("raw-case-4.osc", db);
        // if ( getWKTFromDB("relations", 211766, db).compare(expectedGeometries[3]) == 0) {
        //     runtest.pass("1 created Relation referencing 1 created Way and 1 existing Way (different changesets)");
        // } else {
        //     runtest.fail("1 created Relation referencing 1 created Way and 1 existing Way (different changesets)");
        //     return 1;
        // }

        // // 1 modified Node, indirectly modify other existing Ways and 1 Relation
        // processFile("raw-case-5.osc", db);
        // if ( getWKTFromDB("relations", 211766, db).compare(expectedGeometries[4]) == 0) {
        //     runtest.pass("1 modified Node, indirectly modify other existing Ways and 1 Relation (different changesets)");
        // } else {
        //     runtest.fail("1 modified Node, indirectly modify other existing Ways and 1 Relation (different changesets)");
        //     return 1;
        // }

        // // 4 created Nodes, 2 created Ways, 1 created Relation with type=multilinestring
        // processFile("raw-case-6.osc", db);
        // if ( getWKTFromDB("relations", 211776, db).compare(expectedGeometries[5]) == 0) {
        //     runtest.pass("4 created Nodes, 2 created Ways, 1 created Relation with type=multilinestring (same changeset)");
        // } else {
        //     runtest.fail("4 created Nodes, 2 created Ways, 1 created Relation with type=multilinestring (same changeset)");
        //     return 1;
        // }

        // // Complex, 1 polygon relation made of multiple ways
        // processFile("raw-case-7.osc", db);
        // if ( getWKTFromDB("relations", 17331328, db).compare(expectedGeometries[6]) == 0) {
        //     runtest.pass("Complex, 1 polygon relation made of multiple ways (same changeset)");
        // } else {
        //     runtest.fail("Complex, 1 polygon relation made of multiple ways (same changeset)");
        //     return 1;
        // }

        // 
        // processFile("raw-case-8.osc", db);
        // if ( getWKTFromDB("relations", 16191459, db).compare(expectedGeometries[7]) == 0) {
        //     runtest.pass("Complex, 1 polygon relation made of multiple ways (same changeset)");
        // } else {
        //     runtest.fail("Complex, 1 polygon relation made of multiple ways (same changeset)");
        //     return 1;
        // }

        processFile("raw-case-9.osc", db);
        if ( getWKTFromDB("relations", 16193116, db).compare(expectedGeometries[8]) == 0) {
            runtest.pass("Complex, 2 polygon relation made of multiple ways (same changeset)");
        } else {
            runtest.fail("Complex, 2 polygon relation made of multiple ways (same changeset)");
            return 1;
        }


    }



}