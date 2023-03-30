//
// Copyright (c) 2020, 2021, 2022 Humanitarian OpenStreetMap Team
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

#include "data/pq.hh"
#include "utils/log.hh"
#include <dejagnu.h>
#include <iostream>
#include <string>

TestState runtest;

class TestPQ : public pq::Pq {
  public:
    TestPQ(void){};
};

int
main(int argc, char *argv[])
{
    TestPQ tp;
    bool ret;

    logger::LogFile &dbglogfile = logger::LogFile::getDefaultInstance();

    dbglogfile.setLogFilename("pq-test.log");
    dbglogfile.setVerbosity(3);

    if (tp.dbname.empty()) {
        runtest.pass("PQ::PQ(void)");
    } else {
        runtest.fail("PQ::PQ(void)");
    }

    ret = tp.parseURL("testdb");
    // tp.dump();
    if (ret && !tp.dbname.empty() && tp.host.empty()) {
        runtest.pass("PQ::parseURL(dbname)");
    } else {
        runtest.fail("PQ::parseURL(dbname)");
    }
    ret = tp.parseURL("localhost/testdb");
    //tp.dump();
    if (ret && tp.dbname == "dbname=testdb" && tp.host.empty()) {
        runtest.pass("PQ::parseURL(localhost/dbname)");
    } else {
        runtest.fail("PQ::parseURL(localhost/dbname)");
    }
    ret = tp.parseURL("testhost/testdb");
    //tp.dump();
    if (ret && tp.dbname == "dbname=testdb" && tp.host == "host=testhost") {
        runtest.pass("PQ::parseURL(remote/dbname)");
    } else {
        runtest.fail("PQ::parseURL(remote/dbname)");
    }

    ret = tp.parseURL("foo@testhost/testdb");
    //tp.dump();
    if (ret && tp.dbname == "dbname=testdb" && tp.host == "host=testhost" &&
        tp.user == "user=foo" && tp.passwd.empty()) {
        runtest.pass("PQ::parseURL(user@remote/dbname)");
    } else {
        runtest.fail("PQ::parseURL(user@remote/dbname)");
    }

    ret = tp.parseURL("foo:bar@testhost/testdb");
    //tp.dump();
    if (ret && tp.dbname == "dbname=testdb" && tp.host == "host=testhost" &&
        tp.user == "user=foo" && tp.passwd == "password=bar") {
        runtest.pass("PQ::parseURL(user:pass@remote/dbname)");
    } else {
        runtest.fail("PQ::parseURL(user:pass@remote/dbname)");
    }

    ret = tp.parseURL("foo:bar@testhost");
    //tp.dump();
    if (ret && tp.dbname == "" && tp.host == "host=testhost" &&
        tp.user == "user=foo" && tp.passwd == "password=bar") {
        runtest.pass("PQ::parseURL(user:pass@remote)");
    } else {
        runtest.fail("PQ::parseURL(user:pass@remote)");
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
