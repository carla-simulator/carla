// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Junction.h"
#include "carla/client/Lane.h"
#include "carla/client/Map.h"
#include "carla/client/Road.h"
#include "carla/road/element/Waypoint.h"

#include <algorithm>
#include <unordered_set>

namespace carla {
namespace client {

  Junction::Junction(SharedPtr<const Map> parent, const road::Junction *junction) : _parent(parent) {
    _bounding_box = junction->GetBoundingBox();
    _id = junction->GetId();
  }

  std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> Junction::GetWaypoints(
      road::Lane::LaneType type) const {
    return _parent->GetJunctionWaypoints(GetId(), type);
  }

  geom::BoundingBox Junction::GetBoundingBox() const {
    return _bounding_box;
  }

  std::vector<SharedPtr<Road>> Junction::GetConnectingRoads() const {
    std::vector<SharedPtr<Road>> result;
    std::unordered_set<road::RoadId> seen;
    const road::Junction *junction = _parent->GetMap().GetJunction(_id);
    if (junction == nullptr) {
      return result;
    }
    for (const auto &pair : junction->GetConnections()) {
      const road::RoadId road_id = pair.second.connecting_road;
      if (!seen.insert(road_id).second) {
        continue;
      }
      SharedPtr<Road> road = _parent->GetRoad(road_id);
      if (road != nullptr) {
        result.emplace_back(std::move(road));
      }
    }
    return result;
  }

  std::vector<SharedPtr<Road>> Junction::GetAdjacentRoads() const {
    std::vector<SharedPtr<Road>> result;
    std::unordered_set<road::RoadId> seen;
    const road::Junction *junction = _parent->GetMap().GetJunction(_id);
    if (junction == nullptr) {
      return result;
    }
    for (const auto &pair : junction->GetConnections()) {
      const road::RoadId road_id = pair.second.incoming_road;
      if (!seen.insert(road_id).second) {
        continue;
      }
      SharedPtr<Road> road = _parent->GetRoad(road_id);
      if (road != nullptr) {
        result.emplace_back(std::move(road));
      }
    }
    return result;
  }

  // Entry and exit lanes are found by walking the lane graph out of each
  // connecting lane rather than by reading Connection::lane_links, because the
  // graph already resolves which end of the adjacent road the junction meets
  // and which lane section applies there. Lanes still inside the junction are
  // dropped, so what comes back is always on an adjacent road.
  std::vector<SharedPtr<Lane>> Junction::GetNeighbourLanes(
      road::Lane::LaneType type,
      bool entering) const {
    std::vector<SharedPtr<Lane>> result;
    std::unordered_set<uint64_t> seen;
    for (const auto &pair : GetWaypoints(type)) {
      const SharedPtr<Waypoint> &waypoint = entering ? pair.first : pair.second;
      if (waypoint == nullptr) {
        continue;
      }
      SharedPtr<Lane> lane = waypoint->GetLane();
      if (lane == nullptr) {
        continue;
      }
      const auto neighbours = entering ? lane->GetPreviousLanes() : lane->GetNextLanes();
      for (const SharedPtr<Lane> &neighbour : neighbours) {
        if (neighbour == nullptr) {
          continue;
        }
        SharedPtr<Road> road = neighbour->GetRoad();
        // Skip anything still inside this junction.
        if (road == nullptr || road->GetJunctionId() == _id) {
          continue;
        }
        if (!seen.insert(neighbour->GetUniqueId()).second) {
          continue;
        }
        result.emplace_back(neighbour);
      }
    }
    return result;
  }

  std::vector<SharedPtr<Lane>> Junction::GetEntryLanes(road::Lane::LaneType type) const {
    return GetNeighbourLanes(type, true);
  }

  std::vector<SharedPtr<Lane>> Junction::GetExitLanes(road::Lane::LaneType type) const {
    return GetNeighbourLanes(type, false);
  }

  // The end of a lane that touches the junction. An entry lane carries traffic
  // into the junction, so the junction sits at its downstream end; an exit lane
  // carries traffic away, so the junction sits at its upstream end. Which end
  // that is in road coordinates depends on the lane's direction.
  static double JunctionSideDistance(const Lane &lane, bool entering) {
    // Kept a hair inside the lane so the waypoint is always valid rather than
    // landing exactly on the boundary.
    constexpr double kEpsilon = 1e-3;
    const double start = lane.GetDistance();
    const double end = start + lane.GetLength();
    const bool downstream = entering;
    const bool at_end = lane.IsPositiveDirection() == downstream;
    if (at_end) {
      return std::max(start, end - kEpsilon);
    }
    return std::min(end, start + kEpsilon);
  }

  std::vector<SharedPtr<Waypoint>> Junction::GetNeighbourWaypoints(
      road::Lane::LaneType type,
      bool entering) const {
    std::vector<SharedPtr<Waypoint>> result;
    for (const SharedPtr<Lane> &lane : GetNeighbourLanes(type, entering)) {
      SharedPtr<Waypoint> waypoint = lane->GetWaypoint(JunctionSideDistance(*lane, entering));
      if (waypoint != nullptr) {
        result.emplace_back(std::move(waypoint));
      }
    }
    return result;
  }

  std::vector<SharedPtr<Waypoint>> Junction::GetEntryWaypoints(road::Lane::LaneType type) const {
    return GetNeighbourWaypoints(type, true);
  }

  std::vector<SharedPtr<Waypoint>> Junction::GetExitWaypoints(road::Lane::LaneType type) const {
    return GetNeighbourWaypoints(type, false);
  }

} // namespace client
} // namespace carla
