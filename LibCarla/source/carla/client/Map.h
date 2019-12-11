// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/Map.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/rpc/MapInfo.h"
#include "carla/road/Lane.h"

#include <string>

namespace carla {
namespace geom { class GeoLocation; }
namespace client {

  class Waypoint;

  class Map
    : public EnableSharedFromThis<Map>,
      private NonCopyable {
  public:

    explicit Map(rpc::MapInfo description);

    explicit Map(std::string name, std::string xodr_content);

    ~Map();

    const std::string &GetName() const {
      return _description.name;
    }

    const road::Map &GetMap() const {
      return _map;
    }

    const std::string &GetOpenDrive() const {
      return _description.open_drive_file;
    }

    const std::vector<geom::Transform> &GetRecommendedSpawnPoints() const {
      return _description.recommended_spawn_points;
    }

    SharedPtr<Waypoint> GetWaypoint(
        const geom::Location &location,
        bool project_to_road = true,
        uint32_t lane_type = static_cast<uint32_t>(road::Lane::LaneType::Driving)) const;

    using TopologyList = std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>>;

    TopologyList GetTopology() const;

    std::vector<SharedPtr<Waypoint>> GenerateWaypoints(double distance) const;

    std::vector<road::element::LaneMarking> CalculateCrossedLanes(
        const geom::Location &origin,
        const geom::Location &destination) const;

    const geom::GeoLocation &GetGeoReference() const;

    std::vector<geom::Location> GetAllCrosswalkZones() const;

  private:

    const rpc::MapInfo _description;

    const road::Map _map;
  };

} // namespace client
} // namespace carla
