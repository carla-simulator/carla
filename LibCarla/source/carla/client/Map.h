// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/rpc/MapInfo.h"

#include <string>

namespace carla {
namespace road { class Map; }
namespace client {

  class Waypoint;

  class Map
    : public EnableSharedFromThis<Map>,
      private NonCopyable {
  public:

    explicit Map(rpc::MapInfo description);

    ~Map();

    const std::string &GetName() const {
      return _description.name;
    }

    const std::string &GetOpenDrive() const {
      return _description.open_drive_file;
    }

    const std::vector<geom::Transform> &GetRecommendedSpawnPoints() const {
      return _description.recommended_spawn_points;
    }

    SharedPtr<Waypoint> GetWaypoint(
        const geom::Location &location,
        bool project_to_road = true) const;

    using TopologyList = std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>>;

    TopologyList GetTopology() const;

    std::vector<road::element::LaneMarking> CalculateCrossedLanes(
        const geom::Location &origin,
        const geom::Location &destination) const;

  private:

    rpc::MapInfo _description;

    SharedPtr<road::Map> _map;
  };

} // namespace client
} // namespace carla
