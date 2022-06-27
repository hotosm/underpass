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

/// \file tmteams.hh
/// \brief Store teams from the Tasking Manager in the Galaxy database

#ifndef __TMTEAMS_HH__
#define __TMTEAMS_HH__

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
# include "unconfig.h"
#endif

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <pqxx/pqxx>

#include <boost/date_time.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace tmdb {

/// \class TMTeam
/// \brief The class contains Team information from the Tasking Manger
class TMTeam
{
public:
    TMTeam(pqxx::result::const_iterator &row);
    /// Get the Team name
    std::string &getName(void) { return name; };
    /// Get the Team ID
    int getID(void) const { return teamid; };
    /// Get the Orgtanization ID
    int getOrgID(void) const { return orgid; };
//private:
    int teamid; ///< The Team ID from Tasking Manager
    int orgid; ///< The Oragnization ID from Tasking Manager
    std::string name; ///< The team name from Tasking Manager
    std::vector<long> members; ///< The members of this team
};

} // EOF tmdb namespace
#endif  // EOF __TMTEAMS_HH__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
