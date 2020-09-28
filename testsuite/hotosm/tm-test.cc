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
