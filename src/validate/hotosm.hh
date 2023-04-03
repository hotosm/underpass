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

/// \file hotosm.hh
/// \brief This file implements the data validation used by HOT

#ifndef __HOTOSM_HH__
#define __HOTOSM_HH__

// This is generated by autoconf
#ifdef HAVE_CONFIG_H
# include "unconfig.h"
#endif

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <iostream>

#include <boost/config.hpp>
#include <boost/geometry.hpp>
#include <boost/dll/alias.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

#include "utils/yaml.hh"

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION

#include "validate.hh"

/// \namespace hotosm
namespace hotosm {

/// \class Hotosm
/// \brief This is the plugin class, deprived from the Validate class
class Hotosm : public Validate
{
public:
    Hotosm(void);
    ~Hotosm(void) {  };
    // Hotosm(std::vector<std::shared_ptr<osmchange::OsmChange>> &changes);

    /// Check a POI for tags. A node that is part of a way shouldn't have any
    /// tags, this is to check actual POIs, like a school.
    std::shared_ptr<ValidateStatus> checkPOI(const osmobjects::OsmNode &node, const std::string &type);

    /// This checks a way. A way should always have some tags. Often a polygon
    /// is a building
    std::shared_ptr<ValidateStatus> checkWay(const osmobjects::OsmWay &way, const std::string &type);

    /// Check a tag
    std::shared_ptr<ValidateStatus> checkTag(const std::string &key, const std::string &value);

    /// Check a set of changes
    std::vector<ValidateStatus> checkOsmChange(const std::string &xml, const std::string &check);

    // Factory method
    static std::shared_ptr<Hotosm> create(void) {
    return std::make_shared<Hotosm>();
    };
private:
    std::vector<long> buildings;       ///< 
    std::vector<long> node_errors;     ///< 
    std::vector<long> way_errors;      ///< 
    std::vector<long> relation_errors; ///<
    std::map<std::string, std::vector<std::string>> tests;
    bool isValidTag(const std::string &key, const std::string &value, yaml::Node tags);
    bool isRequiredTag(const std::string &key, yaml::Node required_tags);
};

BOOST_DLL_ALIAS(Hotosm::create, create_plugin)

} // EOF hotosm namespace

#endif  // EOF __HOTOSM_HH__

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
