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
#include <mutex>

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
    int taskIndex;
    std::shared_ptr<std::vector<BootstrapTask>> tasks;
    std::shared_ptr<std::vector<OsmWay>> ways;
};

struct NodeTask {
    int taskIndex;
    std::shared_ptr<std::vector<BootstrapTask>> tasks;
    std::shared_ptr<std::vector<OsmNode>> nodes;
};

struct RelationTask {
    int taskIndex;
    std::shared_ptr<std::vector<BootstrapTask>> tasks;
    std::shared_ptr<std::vector<OsmRelation>> relations;
};

class Bootstrap {
  public:
    Bootstrap(void);
    ~Bootstrap(void){};

    static const std::string polyTable;
    static const std::string lineTable;
    
    void start(const underpassconfig::UnderpassConfig &config);
    void processWays();
    void processNodes();
    void processRelations();

    // This thread get started for every page of way
    void threadBootstrapWayTask(WayTask wayTask);
    void threadBootstrapNodeTask(NodeTask nodeTask);
    void threadBootstrapRelationTask(RelationTask relationTask);
    std::string allTasksQueries(std::shared_ptr<std::vector<BootstrapTask>> tasks);
    
    std::shared_ptr<Validate> validator;
    std::shared_ptr<QueryValidate> queryvalidate;
    std::shared_ptr<QueryRaw> queryraw;
    std::shared_ptr<Pq> db;
    std::shared_ptr<Pq> osmdb;
    bool norefs;
    unsigned int concurrency;
    unsigned int page_size;
};

static std::mutex tasks_change_mutex;

}
