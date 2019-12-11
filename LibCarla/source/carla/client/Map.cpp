// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Map.h"

#include "carla/client/Waypoint.h"
#include "carla/opendrive/OpenDriveParser.h"
#include "carla/road/Map.h"
#include "carla/road/RoadTypes.h"

#include <sstream>

namespace carla {
namespace client {

  static auto MakeMap(const std::string &opendrive_contents) {
    auto stream = std::istringstream(opendrive_contents);
    auto map = opendrive::OpenDriveParser::Load(stream.str());
    if (!map.has_value()) {
      throw_exception(std::runtime_error("failed to generate map"));
    }
    return std::move(*map);
  }

  Map::Map(rpc::MapInfo description)
    : _description(std::move(description)),
      _map(MakeMap(_description.open_drive_file)) {}

  Map::Map(std::string name, std::string xodr_content)
    : Map(rpc::MapInfo{
          std::move(name),
          std::move(xodr_content),
          std::vector<geom::Transform>{}}) {}

  Map::~Map() = default;

  SharedPtr<Waypoint> Map::GetWaypoint(
      const geom::Location &location,
      bool project_to_road,
      uint32_t lane_type) const {
    boost::optional<road::element::Waypoint> waypoint;
    if (project_to_road) {
      waypoint = _map.GetClosestWaypointOnRoad(location, lane_type);
    } else {
      waypoint = _map.GetWaypoint(location, lane_type);
    }
    return waypoint.has_value() ?
        SharedPtr<Waypoint>(new Waypoint{shared_from_this(), *waypoint}) :
        nullptr;
  }

  Map::TopologyList Map::GetTopology() const {
    namespace re = carla::road::element;
    std::unordered_map<re::Waypoint, SharedPtr<Waypoint>> waypoints;

    auto get_or_make_waypoint = [&](const auto &waypoint) {
      auto it = waypoints.find(waypoint);
      if (it == waypoints.end()) {
        it = waypoints.emplace(
            waypoint,
            SharedPtr<Waypoint>(new Waypoint{shared_from_this(), waypoint})).first;
      }
      return it->second;
    };

    TopologyList result;
    auto topology = _map.GenerateTopology();
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
    const auto waypoints = _map.GenerateWaypoints(distance);
    result.reserve(waypoints.size());
    for (const auto &waypoint : waypoints) {
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint{shared_from_this(), waypoint}));
    }
    return result;
  }

  std::vector<road::element::LaneMarking> Map::CalculateCrossedLanes(
      const geom::Location &origin,
      const geom::Location &destination) const {
    return _map.CalculateCrossedLanes(origin, destination);
  }

  const geom::GeoLocation &Map::GetGeoReference() const {
    return _map.GetGeoReference();
  }

  std::vector<geom::Location> Map::GetAllCrosswalkZones() const{
    return _map.GetAllCrosswalkZones();
  }

} // namespace client
} // namespace carla
