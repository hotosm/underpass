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

#include <dejagnu.h>
#include <iostream>
#include <string>
#include "hotosm.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace apidb;
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace hotosmtest {

TestState runtest;

class TestPG : public QueryStats
{
public:
    TestPG() {
        uid = 760688; 
    };
private:
    long uid;
};

class TestTM : public QueryTM
{
public:
    TestTM() {
        uid = 760688;        
    };
private:
    long uid;
};

void test_pgsnapshot(void);
void test_tm(void);
void test_leaderboard(void);

int
main(int argc, char *argv[])
{

    test_pgsnapshot();
    test_tm();
    test_leaderboard();
}

void
test_pgsnapshot(void)
{
// FIXME: database should be a command line argument
    std::string database = "pgsnapshot";    
    long uid = 7606880;
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

    ptime start = time_from_string("2010-07-08 13:29:46");
    ptime end = second_clock::local_time();

    // std::cout << to_simple_string(start) << std::endl;
    // std::cout << to_simple_string(end) << std::endl;
    
    long ret = testpg.getCount(QueryStats::building, uid,
                             QueryStats::totals, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::building)");
    } else {
        runtest.fail("getCount(QueryStats::building)");
    }
    ret = testpg.getCount(QueryStats::highway, uid,
                        QueryStats::totals, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::highway)");
    } else {
        runtest.fail("getCount(QueryStats::highway)");
    }

    ret = testpg.getCount(QueryStats::waterway, uid,
                        QueryStats::totals, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::waterway)");
    } else {
        runtest.fail("getCount(QueryStats::waterway)");
    }

    ret = testpg.getCount(QueryStats::highway, uid,
                        QueryStats::changesets, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::highway,changesets)");
    } else {
        runtest.fail("getCount(QueryStats::highway,changesets)");
    }
    ret = testpg.getCount(QueryStats::building, uid,
                        QueryStats::changesets, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::building,changesets)");
    } else {
        runtest.fail("getCount(QueryStats::building,changesets)");
    }
    ret = testpg.getCount(QueryStats::waterway, uid,
                        QueryStats::changesets, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::waterway,changesets)");
    } else {
        runtest.fail("getCount(QueryStats::waterway,changesets)");
    }
    
    ret = testpg.getLength(QueryStats::waterway, uid, start, end);
    if (ret >= 0) {
        runtest.pass("getCount(QueryStats::getLength(waterway))");
    } else {
        runtest.fail("getCount(QueryStats::getLength(waterway)");
    }
}

void
test_tm(void)
{
    //
    // Tasking Manager Tests
    //
    TestTM testtm;
    std::string database = "tmsnap";
    long projid = 2057;
    long uid = 7606880;

    std::shared_ptr<std::vector<long>> retv;
    int reti;
    long retl;
    
    // These testpgs are for the base class, which handles
    // the database connection.
    if (testtm.connect(database)) {
        runtest.pass("QueryTM::connect()");
    } else {
        runtest.fail("QueryTM::connect()");
    }

    retv = testtm.getProjects(0);
    if (retv->size() > 0) {
        runtest.pass("getProjects(0)");
    } else {
        runtest.fail("getProjects(0)");
    }

    retv = testtm.getProjects(uid);
    if (retv->size() >= 0) {
        runtest.pass("getProjects(uid)");
    } else {
        runtest.fail("getProjects(uid)");
    }

    retv = testtm.getUserTasks(projid, uid);
    if (retv->size() >= 0) {
        runtest.pass("getProjects()");
    } else {
        runtest.fail("getProjects()");
    }
    
    reti = testtm.getTasksMapped(uid);
    if (reti >= 0) {
        runtest.pass("getTasksMapped()");
    } else {
        runtest.fail("getTasksMapped()");
    }


    reti = testtm.getTasksValidated(uid);
    if (reti >= 0) {
        runtest.pass("getTasksValidated()");
    } else {
        runtest.fail("getTasksValidated()");
    }

    std::shared_ptr<std::vector<int>> retvi;
    retvi = testtm.getUserTeams(uid);
    if (retvi->size() >= 0) {
        runtest.pass("getUserTeams()");
    } else {
        runtest.fail("getUserTeams()");
    }
}

//
// Collect all the stats the MM Leaderboard needs. This requires
// querying both the TM database and the OSM database.
//
void
test_leaderboard(void)
{
    TestTM testtm;

    ptime start = time_from_string("2010-01-01 04:20:00");
    ptime end = second_clock::local_time();

    std::cout << "MM Leaderboard tests" << std::endl;
    
    std::string database = "tmsnap";
    if (testtm.connect(database)) {
        runtest.pass("QueryDB::connect()");
    } else {
        runtest.fail("QueryDB::connect()");
    }
    
    TestPG testpg;
    database = "pgsnapshot";
    // These testpgs are for the base class, which handles
    // the database connection.
    if (testpg.connect(database)) {
        runtest.pass("QueryDB::connect()");
    } else {
        runtest.fail("QueryDB::connect()");
    }

    // Their account name
    // The Team
    long uid = 7606880;
    std::shared_ptr<std::vector<int>> retvi;
    retvi = testtm.getUserTeams(uid);
    if (retvi->size() >= 0) {
        runtest.pass("QueryTM::getUserTeams()");
    } else {
        runtest.fail("QueryTM::getUserTeams()");
    }
    database = "pgsnapshot";

// Total edits
    
    // Building Edits
    long ret = testpg.getCount(QueryStats::building, uid,
                             QueryStats::totals, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::building)");
    } else {
        runtest.fail("getCount(QueryStats::building)");
    }    
    // KM of Roads
    ret = testpg.getLength(QueryStats::highway, uid, start, end);
    if (ret > 0) {
        runtest.pass("getCount(QueryStats::getLength(highway)");
    } else {
        runtest.fail("getCount(QueryStats::getLength(highway)");
    }
    
    // Last Update
    ptime last;
    last = testpg.lastUpdate(uid, last);
    std::cout << "LAST: " << last << std::endl;
}

}

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
