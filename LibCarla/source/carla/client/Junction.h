// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/road/Junction.h"
#include "carla/road/RoadTypes.h"
#include "carla/geom/BoundingBox.h"
#include "carla/client/Waypoint.h"

#include <vector>

namespace carla {
namespace client {

  class Map;
  class Road;
  class Lane;

  class Junction
    : public EnableSharedFromThis<Junction>,
    private NonCopyable
  {
  public:

    carla::road::JuncId GetId() const {
      return _id;
    }

    std::vector<std::pair<SharedPtr<Waypoint>,SharedPtr<Waypoint>>> GetWaypoints(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    geom::BoundingBox GetBoundingBox() const;

    /// The roads *inside* the junction, carrying traffic through it. This is
    /// OpenDRIVE's "connecting road"; the name is kept so the API agrees with
    /// the .xodr.
    std::vector<SharedPtr<Road>> GetConnectingRoads() const;

    /// The roads *outside* the junction that meet it. OpenDRIVE calls these
    /// "incoming", but a road has no inherent direction -- the same road is an
    /// exit for traffic going the other way -- so they are adjacent here.
    std::vector<SharedPtr<Road>> GetAdjacentRoads() const;

    /// Lanes on the adjacent roads that feed into the junction.
    std::vector<SharedPtr<Lane>> GetEntryLanes(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    /// Lanes on the adjacent roads that the junction feeds into.
    std::vector<SharedPtr<Lane>> GetExitLanes(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    /// One waypoint per entry lane, on the adjacent road where it meets the
    /// junction. Outside the junction, like GetEntryLanes(); for the waypoints
    /// on the connecting roads inside it, use GetWaypoints().
    std::vector<SharedPtr<Waypoint>> GetEntryWaypoints(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

    /// One waypoint per exit lane, on the adjacent road where it leaves the
    /// junction.
    std::vector<SharedPtr<Waypoint>> GetExitWaypoints(
        road::Lane::LaneType type = road::Lane::LaneType::Driving) const;

  private:

    friend class Map;

    /// Shared implementation of GetEntryLanes / GetExitLanes.
    std::vector<SharedPtr<Lane>> GetNeighbourLanes(
        road::Lane::LaneType type,
        bool entering) const;

    /// Shared implementation of GetEntryWaypoints / GetExitWaypoints.
    std::vector<SharedPtr<Waypoint>> GetNeighbourWaypoints(
        road::Lane::LaneType type,
        bool entering) const;

    Junction(SharedPtr<const Map> parent, const road::Junction *junction);

    SharedPtr<const Map> _parent;

    geom::BoundingBox _bounding_box;

    road::JuncId _id;
  };

} // namespace client
} // namespace carla
