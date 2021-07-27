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
#include <pqxx/pqxx>

#include "hottm.hh"

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace tmdb;
using namespace boost::posix_time;
using namespace boost::gregorian;

TestState runtest;

class TestTM : public TaskingManager
{
};

int
main(int argc, char *argv[])
{
    TestTM testtm;

    std::string database = "tmsnap";
    
    if (testtm.connect(database)) {
        runtest.pass("taskingManager::connect()");
    } else {
        runtest.fail("taskingManager::connect()");
    }

    std::shared_ptr<std::vector<long>> retl;
    std::shared_ptr<std::vector<TMUser>> retu;
    std::shared_ptr<std::vector<TMTeam>> rett;
    std::shared_ptr<std::vector<TMProject>> retp;

    retu = testtm.getUsers();
    if (retu->size() > 0 &&
        retu->at(0).id > 0 &&
        !retu->at(0).username.empty() &&
        retu->at(0).tasks_mapped >= 0) {
        runtest.pass("taskingManager::getUsers()");
    } else {
        runtest.fail("taskingManager::getUsers()");
    }
    
    rett  = testtm.getTeams(); 
    if (rett->size() > 0 &&
        rett->at(0).teamid > 0 &&
        rett->at(0).orgid > 0 &&
        !rett->at(0).name.empty()) {
        runtest.pass("taskingManager::getTeams()");
    } else {
        runtest.fail("taskingManager::getTeams()");
    }
   
    retl = testtm.getTeamMembers(1, true);
    if (retl->size() >= 0) {
        runtest.pass("taskingManager::getTeamMembers()");
    } else {
        runtest.fail("taskingManager::getTeamMembers()");
    }

    retp = testtm.getProjects();
    if (retp->size() > 0) {
        runtest.pass("taskingManager::getProjects()");
    } else {
        runtest.fail("taskingManager::getProjects()");
    }

    retl = testtm.getProjectTeams(1);
    if (retl->size() >= 0) {
        runtest.pass("taskingManager::getProjectTeams()");
    } else {
        runtest.fail("taskingManager::getProjectTeams()");
    }

}
