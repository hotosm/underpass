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

int
main(int argc, char *argv[])
{
    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
    dbglogfile.setWriteDisk(true);
    dbglogfile.setLogFilename("conflate-test.log");
    dbglogfile.setVerbosity(3);

    Test test;
    test.init();
    
    conflate::Conflate conf("conflate_test");

    std::string inview = "POLYGON((25.7799125 -24.6674081,25.7804301 -24.6673313,25.7807593 -24.6676784,25.7799895 -24.6678369,25.7799125 -24.6674081))";
    polygon_t bldin;
    boost::geometry::read_wkt(inview, bldin);
    std::string outview = "POLYGON((25.779663 -24.6681283,25.7800424 -24.6680503,25.7801583 -24.6685156,25.779779 -24.6685937,25.779663 -24.6681283))";
    polygon_t bldout;
    boost::geometry::read_wkt(outview, bldout);

    //There should be buildings in the specified area
    auto ids = conf.existingDuplicate(bldin);
    if (ids->size() > 0) {
        runtest.pass("Conflate::existingDuplicate()");
    } else {     
        runtest.fail("Conflate::existingDuplicate()");
    }

    // There should be no buildings in the specified area
    ids = conf.existingDuplicate(bldout);
    if (ids->size() == 0) {
        runtest.pass("Conflate::existingDuplicate()");
    } else {     
        runtest.fail("Conflate::existingDuplicate()");
    }

    osmobjects::OsmWay way;
    way.id = 12345;
    boost::geometry::read_wkt("POLYGON((25.7802443 -24.6676735,25.7802581 -24.6677421,25.7803698 -24.6677236,25.7803538 -24.667644,25.7802976 -24.6676533,25.7803043 -24.6676636,25.7802443 -24.6676735))", way.polygon);
    
    ids = conf.newDuplicate(way, bldin);
    if (ids->size() > 0) {
        runtest.pass("Conflate::newDuplicate()");
    } else {     
        runtest.fail("Conflate::newDuplicate()");
    }
    
    ids = conf.newDuplicate(way, bldout);
    if (ids->size() == 0) {
        runtest.pass("Conflate::newDuplicate()");
    } else {     
        runtest.fail("Conflate::newDuplicate()");
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
