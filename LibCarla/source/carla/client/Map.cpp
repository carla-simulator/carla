// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Map.h"

#include "carla/client/Waypoint.h"
#include "carla/opendrive/OpenDrive.h"
#include "carla/road/Map.h"
#include "carla/road/WaypointGenerator.h"

#include <sstream>

namespace carla {
namespace client {

  static auto MakeMap(const std::string &opendrive_contents) {
    auto stream = std::istringstream(opendrive_contents);
    return opendrive::OpenDrive::Load(stream);
  }

  Map::Map(rpc::MapInfo description)
    : _description(std::move(description)),
      _map(MakeMap(_description.open_drive_file)) {}

  Map::~Map() = default;

  SharedPtr<Waypoint> Map::GetWaypoint(
      const geom::Location &location,
      bool project_to_road) const {
    DEBUG_ASSERT(_map != nullptr);
    Optional<road::element::Waypoint> waypoint;
    if (project_to_road) {
      waypoint = _map->GetClosestWaypointOnRoad(location);
    } else {
      waypoint = _map->GetWaypoint(location);
    }
    return waypoint.has_value() ?
        SharedPtr<Waypoint>(new Waypoint{shared_from_this(), *waypoint}) :
        nullptr;
  }

  Map::TopologyList Map::GetTopology() const {
    DEBUG_ASSERT(_map != nullptr);
    namespace re = carla::road::element;
    std::unordered_map<re::id_type, std::unordered_map<int, SharedPtr<Waypoint>>> waypoints;

    auto get_or_make_waypoint = [&](const auto &waypoint) {
      auto &waypoints_on_road = waypoints[waypoint.GetRoadId()];
      auto it = waypoints_on_road.find(waypoint.GetLaneId());
      if (it == waypoints_on_road.end()) {
        it = waypoints_on_road.emplace(
            waypoint.GetLaneId(),
            SharedPtr<Waypoint>(new Waypoint{shared_from_this(), waypoint})).first;
      }
      return it->second;
    };

    TopologyList result;
    auto topology = road::WaypointGenerator::GenerateTopology(*_map);
    result.reserve(topology.size());
    for (const auto &pair : topology) {
      result.emplace_back(
          get_or_make_waypoint(pair.first),
          get_or_make_waypoint(pair.second));
    }
    return result;
  }

  std::vector<SharedPtr<Waypoint>> Map::GenerateWaypoints(double distance) const {
    std::vector<SharedPtr<Waypoint>> result;
    const auto waypoints = road::WaypointGenerator::GenerateAll(*_map, distance);
    result.reserve(waypoints.size());
    for (const auto &waypoint : waypoints) {
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint{shared_from_this(), waypoint}));
    }
    return result;
  }

  std::vector<road::element::LaneMarking> Map::CalculateCrossedLanes(
      const geom::Location &origin,
      const geom::Location &destination) const {
    DEBUG_ASSERT(_map != nullptr);
    return _map->CalculateCrossedLanes(origin, destination);
  }

} // namespace client
} // namespace carla
