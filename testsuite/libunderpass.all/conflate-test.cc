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
    
    // SRID 4326
    std::string polyin = "MULTIPOLYGON(((25.779831699999995 -24.667576099827592,25.7799345 -24.667314599827634,25.780198699999996 -24.66725999982764,25.7806669 -24.667293299827644,25.7808019 -24.667514699827606,25.780559700000005 -24.667842799827554,25.7802192 -24.667953499827544,25.7797437 -24.66772679982758,25.779831699999995 -24.667576099827592)))";

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
    // ID 102244
    // std::string standalone = "POLYGON((25.7802155 -24.6675598998276,25.7802628 -24.667559999827603,25.780262999999998 -24.667486899827605,25.780215700000003 -24.66748679982761,25.7802155 -24.6675598998276))";
    std::string standalone = "POLYGON((25.7802049 -24.667585399827594,25.7802522 -24.667585499827595,25.7802524 -24.6675123998276,25.7802051 -24.667512299827603,25.7802049 -24.667585399827594))";
    polygon_t stand;
    boost::geometry::read_wkt(standalone, stand);
    osmobjects::OsmWay way;
    way.id = 34567;
    way.polygon = stand;
    auto ids = conf.newDuplicatePolygon(way);
    // for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
    // 	(*it)->dump();
    // }
    if (ids->size() == 3 && ids->at(0)->hasStatus(overlaping)) {
        runtest.pass("Conflate::newDuplicate(overlap)");
    } else {     
        runtest.fail("Conflate::newDuplicate(overlap)");
    }

    polygon_t same;
    // Duplicates with 101861, same size, this is 101870
    // SRID 4326
    std::string dupsame = "POLYGON((25.780364 -24.66762939982759,25.780374599999995 -24.66769119982758,25.780503599999996 -24.66767279982758,25.7804965 -24.667631699827584,25.780522999999995 -24.667627899827586,25.7805194 -24.667607299827598,25.780364 -24.66762939982759))";
    boost::geometry::read_wkt(dupsame, same);    
    way.id = 456789;
    way.polygon = same;
    ids = conf.newDuplicatePolygon(way);
    // for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
    // 	(*it)->dump();
    // }
    if (ids->size() >= 1 && ids->at(0)->hasStatus(duplicate)) {
        runtest.pass("Conflate::newDuplicate(2 dups)");
    } else {     
        runtest.fail("Conflate::newDuplicate(2 dups)");
    }

    // 1 duplicate in existing data
    // SRID 4326
    standalone = "POLYGON((25.7800629 -24.6675573998276,25.780097899999998 -24.66761989982759,25.780143999999996 -24.6675985998276,25.780126099999997 -24.667566599827598,25.780154999999997 -24.667553199827598,25.780137999999997 -24.667522699827607,25.7800629 -24.6675573998276))";
    boost::geometry::read_wkt(standalone, stand);
    way.id = 23456;
    way.polygon = stand;
    ids = conf.newDuplicatePolygon(way);
    // for (auto it = std::begin(*ids); it != std::end(*ids); ++it) {
    // 	(*it)->dump();
    // }
    if (ids->size() == 1 && ids->at(0)->hasStatus(duplicate)) {
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
    std::string overlap = "POLYGON((25.7802443 -24.6676734998276,25.7802581 -24.6677420998276,25.7803698 -24.6677235998276,25.7803538 -24.6676439998276,25.7802976 -24.6676532998276,25.7803043 -24.6676635998276,25.7802443 -24.6676734998276))";
    boost::geometry::read_wkt(overlap, over);

    polygon_t diff;
    // Duplicate, different size with 101831
    std::string dupdiff = "POLYGON((25.7803365 -24.6675077998276,25.7803387 -24.6675955998276,25.7804014 -24.6675941998276,25.7804168 -24.6675031998276,25.7803365 -24.6675077998276))";
    boost::geometry::read_wkt(dupdiff, diff);

    //There should be buildings in the specified area
    auto ids = conf.existingDuplicatePolygon();
    if (ids->size() > 0  && ids->at(0)->hasStatus(duplicate)){
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
    ids = conf.existingDuplicatePolygon();
    if (ids->size() == 0) {
        runtest.pass("Conflate::existingDuplicate()");
    } else {     
        runtest.fail("Conflate::existingDuplicate()");
    }
    way.id = 456789;
    way.polygon = stand;
    ids = conf.newDuplicatePolygon(way, boundaryout);
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
