//
// Copyright (c) 2023 Humanitarian OpenStreetMap Team
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

#include "validate/queryvalidate.hh"
#include "raw/queryraw.hh"
#include "underpassconfig.hh"
#include "validate/validate.hh"

using namespace queryvalidate;
using namespace queryraw;

typedef std::shared_ptr<Validate>(plugin_t)();

namespace bootstrap {

/// \struct BootstrapTask
/// \brief Represents a bootstrap task
struct BootstrapTask {
    std::string query = "";
    int processed = 0;
};

struct WayTask {
    std::shared_ptr<Validate> plugin;
    std::shared_ptr<QueryValidate> queryvalidate;
    underpassconfig::UnderpassConfig config;
    int taskIndex;
    std::shared_ptr<std::vector<BootstrapTask>> tasks;
    std::shared_ptr<std::vector<OsmWay>> ways;
};

void startProcessingWays(const underpassconfig::UnderpassConfig &config);

// This thread get started for every page of way
void threadBootstrapTask(WayTask wayTask);

static std::mutex tasks_change_mutex;

}