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
#include <string>
#include "data/pq.hh"

#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>

#include "validate/conflate.hh"
#include "log.hh"

using namespace logger;
using namespace conflate;
using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace boost::filesystem;


TestState runtest;

class Test : public Conflate
{
public:
    /// Clear the test DB and fill it with with initial test data
    bool init(void) {
        logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
        dbglogfile.setVerbosity();
        
        // Create the database
        std::string datadir = DATADIR;
        pq::Pq postgres("postgres");

        // Create schema
        std::string file;
        postgres.query("DROP DATABASE IF EXISTS conflate_test;");
        postgres.query("CREATE DATABASE conflate_test;");
        conf_db.connect("conflate_test");
        conf_db.query("CREATE EXTENSION postgis");
        conf_db.query("CREATE EXTENSION hstore");
#if 0
        file = datadir + "/testsuite/testdata/osm2pgsql_test_schema.sql.gz";
        std::ifstream sql(file, std::ios_base::in | std::ios_base::binary);
        std::ifstream ifschema;
        ifschema.open(file, std::ifstream::in | std::ifstream::binary);
        try {
            boost::iostreams::filtering_streambuf<boost::iostreams::input>inbuf;
            inbuf.push(boost::iostreams::gzip_decompressor());
            inbuf.push(sql);
            std::istream instream(&inbuf);
            std::string sss((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
            conf_db.query(sss + ";");
        } catch (std::exception &e) {
            log_error(_("opening %1% %2%"), file, e.what());
        }
        ifschema.close();
        sql.close();
#endif
        // The test data file is made from an existing postgres database using
        // this command, which will also create the database and the schema.
        // pg_dump --attribute-inserts -d conflate_test > conflate_test_data.sql
        file = datadir + "/testsuite/testdata/conflate_test_data.sql";
        std::ifstream data(file, std::ios_base::in | std::ios_base::binary);
        std::string ssss((std::istreambuf_iterator<char>(data)), std::istreambuf_iterator<char>());
        conf_db.query(ssss + ";");

        return true;
    };   
    
    pqxx::result query(const std::string &query) {
        if (conf_db.isOpen()) {
            std::cerr << query << std::endl;
            conf_db.query(query);
        }
    };

    pq::Pq conf_db;
    std::string source_tree_root;
};


void test_in_existing(conflate::Conflate &conf);
void test_in_new(conflate::Conflate &conf);
void test_out(conflate::Conflate &conf);

int
main(int argc, char *argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("conflate-test.log");
    dbglogfile.setVerbosity(3);

    Test test;
    test.init();
    
    // SRID 3857
    std::string polyin = "MULTIPOLYGON(((2869797.737580292 -2834968.7437994336,2869809.1812239457 -2834936.710575374,2869838.591833413 -2834930.022193661,2869890.7116190027 -2834934.101371171,2869905.73975026 -2834961.222416892,2869878.77816959 -2835001.414082196,2869840.873882975 -2835014.97465877,2869787.9414651026 -2834987.2042768374,2869797.737580292 -2834968.7437994336)))";
    // SRID 4326
    //std::string polyin = "MULTIPOLYGON(((25.779831699999995 -24.667576099827592,25.7799345 -24.667314599827634,25.780198699999996 -24.66725999982764,25.7806669 -24.667293299827644,25.7808019 -24.667514699827606,25.780559700000005 -24.667842799827554,25.7802192 -24.667953499827544,25.7797437 -24.66772679982758,25.779831699999995 -24.667576099827592)))";

    conflate::Conflate conf("conflate_test");

    multipolygon_t boundary;
    boost::geometry::read_wkt(polyin, boundary);
    conf.createView(boundary);

    test_in_existing(conf);
    test_in_new(conf);
    // test_out(conf);
}

void test_in_new(conflate::Conflate &conf)
{
    // SRID 3857
    // ID 102244
    std::string standalone = "POLYGON((2869837.7792011304 -2834972.6024906104,2869843.044613045 -2834972.614740426,2869843.066876943 -2834963.660128418,2869837.8014650284 -2834963.6478786105,2869837.7792011304 -2834972.6024906104))";
    // SRID 3857
    standalone = "POLYGON((2869844.603085916 -2834960.1689336393,2869849.868497831 -2834960.1811834443,2869849.8907617284 -2834951.226578722,2869844.625349814 -2834951.214328924,2869844.603085916 -2834960.1689336393))";
    // SRID 4326
    // standalone = "POLYGON((25.780113599999996 -24.667691499827583,25.7801486 -24.667753999827575,25.7801947 -24.667732699827575,25.780176799999996 -24.667700699827584,25.780205699999996 -24.667687299827584,25.780188699999997 -24.66765679982758,25.780113599999996 -24.667691499827583))";
    polygon_t stand;
    boost::geometry::read_wkt(standalone, stand);
    osmobjects::OsmWay way;
    way.id = 34567;
    way.polygon = stand;
    auto ids = conf.newDuplicate(way);
    for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
	(*it)->dump();
    }
    if (ids->size() == 1) {
        runtest.pass("Conflate::newDuplicate(overlap)");
    } else {     
        runtest.fail("Conflate::newDuplicate(overlap)");
    }

    polygon_t same;
    // Duplicates with 101861, same size, this is 101870
    // SRID 3857
    std::string dupsame = "POLYGON((2869856.9929452413 -2834975.2729504723,2869858.1729318434 -2834982.843339221,2869872.533146156 -2834980.589371952,2869871.742777772 -2834975.554696264,2869874.6927442774 -2834975.0892032185,2869874.2919941107 -2834972.565741167,2869856.9929452413 -2834975.2729504723))";
    // SRID 4326
    // std::string dupsame = "POLYGON((25.780364 -24.66762939982759,25.780374599999995 -24.66769119982758,25.780503599999996 -24.66767279982758,25.7804965 -24.667631699827584,25.780522999999995 -24.667627899827586,25.7805194 -24.667607299827598,25.780364 -24.66762939982759))";
    boost::geometry::read_wkt(dupsame, same);    
    way.id = 456789;
    way.polygon = same;
    ids = conf.newDuplicate(way);
    for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
	(*it)->dump();
    }
    if (ids->size() >= 1) {
        runtest.pass("Conflate::newDuplicate(2 dups)");
    } else {     
        runtest.fail("Conflate::newDuplicate(2 dups)");
    }

    // 1 duplicate in existing data
    // SRID 3857
    standalone = "POLYGON((2869823.4746465636 -2834966.4530848153,2869827.370828741 -2834974.1092179064,2869832.502657267 -2834971.500007319,2869830.5100383814 -2834967.580067366,2869833.7271716655 -2834965.9385928083,2869831.834740322 -2834962.2024013787,2869823.4746465636 -2834966.4530848153))";
    // SRID 4326
    // std::string standalone = "POLYGON((25.7800629 -24.6675573998276,25.780097899999998 -24.66761989982759,25.780143999999996 -24.6675985998276,25.780126099999997 -24.667566599827598,25.780154999999997 -24.667553199827598,25.780137999999997 -24.667522699827607,25.7800629 -24.6675573998276))";
    boost::geometry::read_wkt(standalone, stand);
    way.id = 23456;
    way.polygon = stand;
    ids = conf.newDuplicate(way);
    for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
	(*it)->dump();
    }
    if (ids->size() == 1) {
        runtest.pass("Conflate::newDuplicate(1 dup)");
    } else {     
        runtest.fail("Conflate::newDuplicate(1 dup)");
    }
}

void test_in_existing(conflate::Conflate &conf)
{
    osmobjects::OsmWay way;
    // Overlaps with 101879
    polygon_t over;
    // SRID 3857
    std::string overlap= "POLYGON((2869843.6680021933 -2834980.675120701,2869845.2042111666 -2834989.078500408,2869857.638598288 -2834986.812282393,2869855.857486435 -2834977.0614238507,2869849.601331053 -2834978.200657003,2869850.3471716414 -2834979.4623884424,2869843.6680021933 -2834980.675120701))";
    boost::geometry::read_wkt(overlap, over);

    polygon_t diff;
    // Duplicate, different size with 101831
    // SRID 3857
    std::string dupdiff = "POLYGON((2869853.9316592445 -2834960.377180323,2869854.1765621244 -2834971.1325129042,2869861.156294197 -2834970.961015515,2869862.8706143554 -2834959.813689304,2869853.9316592445 -2834960.377180323))";
    boost::geometry::read_wkt(dupdiff, diff);

    //There should be buildings in the specified area
    auto ids = conf.existingDuplicate();
    if (ids->size() > 0) {
        runtest.pass("Conflate::existingDuplicate()");
    } else {     
        runtest.fail("Conflate::existingDuplicate()");
    }

    // IDs 101879 and 101809 small overlap
    // IDs 101861 and 101870 are duplicates, same size
    // IDs 101831 and 101868 are duplicates, but different size

    way.id = 12345;
}

void test_out(conflate::Conflate &conf)
{
    osmobjects::OsmWay way;
#if 0
    // SRID 3857
    multipolygon_t boundary;
    boost::geometry::read_wk(poly, boundary);
    conf.createView(boundary);
    
    // There should be no buildings in the specified area
    ids = conf.existingDuplicate();
    if (ids->size() == 0) {
        runtest.pass("Conflate::existingDuplicate()");
    } else {     
        runtest.fail("Conflate::existingDuplicate()");
    }
    way.id = 456789;
    way.polygon = stand;
    ids = conf.newDuplicate(way, boundaryout);
    for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
	(*it)->dump();
    }
    if (ids->size() == 0) {
        runtest.pass("Conflate::newDuplicate(out)");
    } else {     
        runtest.fail("Conflate::newDuplicate(out)");
    }
#endif
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
