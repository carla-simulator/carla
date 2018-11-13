// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Map.h"

#include "carla/client/Waypoint.h"
#include "carla/opendrive/OpenDrive.h"
#include "carla/road/Map.h"

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

  std::vector<road::element::LaneMarking> Map::CalculateCrossedLanes(
      const geom::Location &origin,
      const geom::Location &destination) const {
    DEBUG_ASSERT(_map != nullptr);
    return _map->CalculateCrossedLanes(origin, destination);
  }

} // namespace client
} // namespace carla
