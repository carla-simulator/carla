// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/element/LaneMarking.h"
#include "carla/road/Lane.h"
#include "carla/road/Map.h"
#include "carla/road/RoadTypes.h"
#include "carla/rpc/MapInfo.h"
#include "Landmark.h"

#include <string>

namespace carla {
namespace geom { class GeoLocation; }
namespace client {

  class Waypoint;
  class Junction;

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

    SharedPtr<Waypoint> GetWaypointXODR(
      carla::road::RoadId road_id,
      carla::road::LaneId lane_id,
      float s) const;

    using TopologyList = std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>>;

    TopologyList GetTopology() const;

    std::vector<SharedPtr<Waypoint>> GenerateWaypoints(double distance) const;

    std::vector<road::element::LaneMarking> CalculateCrossedLanes(
        const geom::Location &origin,
        const geom::Location &destination) const;

    const geom::GeoLocation &GetGeoReference() const;

    std::vector<geom::Location> GetAllCrosswalkZones() const;

    SharedPtr<Junction> GetJunction(const Waypoint &waypoint) const;

    /// Returns a pair of waypoints (start and end) for each lane in the
    /// junction
    std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> GetJunctionWaypoints(
        road::JuncId id, road::Lane::LaneType type) const;

    /// Returns all the larndmarks in the map
    std::vector<SharedPtr<Landmark>> GetAllLandmarks() const;

    /// Returns all the larndmarks in the map with a specific OpenDRIVE id
    std::vector<SharedPtr<Landmark>> GetLandmarksFromId(std::string id) const;

    /// Returns all the landmarks in the map of a specific type
    std::vector<SharedPtr<Landmark>> GetAllLandmarksOfType(std::string type) const;

    /// Returns all the landmarks in the same group including this one
    std::vector<SharedPtr<Landmark>> GetLandmarkGroup(const Landmark &landmark) const;

  private:

    const rpc::MapInfo _description;

    const road::Map _map;
  };

} // namespace client
} // namespace carla
