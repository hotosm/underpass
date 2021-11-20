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
    //! Clear the test DB and fill it with with initial test data
    bool init(void) {
        logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();
        dbglogfile.setVerbosity();
        
        // Create the database
        std::string datadir = DATADIR;
        pq::Pq postgres("postgres");
        if (!postgres.isOpen()) {
            exit(0);
        }
        postgres.query("DROP DATABASE IF EXISTS conflate_test;");
        postgres.query("CREATE DATABASE conflate_test;");
        pq::Pq conf_db("conflate_test");
        conf_db.query("CREATE EXTENSION postgis");
        conf_db.query("CREATE EXTENSION hstore");

        // Create schema
        std::string file;
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
    
    // test.duplicate();
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
