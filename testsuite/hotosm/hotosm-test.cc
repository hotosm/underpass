//
// Copyright (c) 2020, Humanitarian OpenStreetMap Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <dejagnu.h>
#include <iostream>
#include <string>
#include "hotosm.h"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace apidb;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestPG : public QueryStats
{
public:
    TestPG()
        {
            std::cout << "Hello World!" << std::endl;
        };
};

class TestTM : public QueryTM
{
public:
    TestTM()
        {
            std::cout << "Hello World!" << std::endl;
        };
};

int
main(int argc, char *argv[])
{
    int c;
    bool dump = false;
    char buffer[300];
    std::string filespec;

    // FIXME: database should be a command line argument
    std::string database = "pgsnapshot";
    
    TestPG testpg;
    // These testpgs are for the base class, which handles
    // the database connection.
    if (testpg.connect(database)) {
        runtest.pass("QueryDB::connect()");
    } else {
        runtest.fail("QueryDB::connect()");
    }
    std::string sql = "SELECT * FROM ways;";
    pqxx::result result = testpg.query(sql);
    if (!result.empty()) {
        runtest.pass("QueryDB::query()");
    } else {
        runtest.fail("QueryDB::query()");
    }

    
    ptime last;
    last = testpg.lastUpdate(191112, last);
    std::cout << "LAST: " << last << std::endl;

    ptime start = time_from_string("2020-07-08 13:29:46");
    ptime end = second_clock::local_time();

    // std::cout << to_simple_string(start) << std::endl;
    // std::cout << to_simple_string(end) << std::endl;
    
    long ret = testpg.getCount(QueryStats::building, 191112,
                             QueryStats::totals, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::building)");
    } else {
        runtest.fail("getCount(QueryStats::building)");
    }
    ret = testpg.getCount(QueryStats::highway, 191112,
                        QueryStats::totals, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::highway)");
    } else {
        runtest.fail("getCount(QueryStats::highway)");
    }

    ret = testpg.getCount(QueryStats::waterway, 191112,
                        QueryStats::totals, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::waterway)");
    } else {
        runtest.fail("getCount(QueryStats::waterway)");
    }

    ret = testpg.getCount(QueryStats::highway, 191112,
                        QueryStats::changesets, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::highway,changesets)");
    } else {
        runtest.fail("getCount(QueryStats::highway,changesets)");
    }
    ret = testpg.getCount(QueryStats::building, 191112,
                        QueryStats::changesets, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::building,changesets)");
    } else {
        runtest.fail("getCount(QueryStats::building,changesets)");
    }
    ret = testpg.getCount(QueryStats::waterway, 191112,
                        QueryStats::changesets, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::waterway,changesets)");
    } else {
        runtest.fail("getCount(QueryStats::waterway,changesets)");
    }
    
    ret = testpg.getLength(QueryStats::highway, 191112, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::getLength(highway)");
    } else {
        runtest.fail("getCount(QueryStats::getLength(highway)");
    }
    
    ret = testpg.getLength(QueryStats::waterway, 191112, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::getLength(waterway))");
    } else {
        runtest.fail("getCount(QueryStats::getLength(waterway)");
    }

    //
    // Tasking Manager Tests
    //
    TestTM testtm;
    database = "tm4_git";
    // These testpgs are for the base class, which handles
    // the database connection.
    if (testtm.connect(database)) {
        runtest.pass("QueryTM::connect()");
    } else {
        runtest.fail("QueryTM::connect()");
    }

    std::shared_ptr<std::vector<long>> reta;
    reta = testtm.getProjects(0);
    if (reta->size() > 0) {
        runtest.pass("getProjects(0)");
    } else {
        runtest.fail("getProjects(0)");
    }

    reta = testtm.getUserTasks(10, 4606673);
    if (reta->size() > 0) {
        runtest.pass("getProjects(10, 4606673)");
    } else {
        runtest.fail("getProjects(10, 4606673)");
    }
    
    ret = testtm.getTasksMapped(4606673);
    if (ret > 0) {
        runtest.pass("getTasksMapped(4606673)");
    } else {
        runtest.fail("getTasksMapped(4606673)");
    }

    ret = testtm.getTasksValidated(4606673);
    if (ret >= 0) {
        runtest.pass("getTasksValidated(4606673)");
    } else {
        runtest.fail("getTasksValidated(4606673)");
    }

    std::shared_ptr<std::vector<int>> retb;
    retb = testtm.getUserTeams(4606673);
    if (retb->size() > 0) {
        runtest.pass("getUserTeams(4606673)");
    } else {
        runtest.fail("getUserTeams(4606673)");
    }

}

