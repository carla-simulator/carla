// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Lane.h"

#include "carla/client/LaneSection.h"
#include "carla/client/Map.h"
#include "carla/client/Road.h"
#include "carla/client/Waypoint.h"
#include "carla/road/LaneSection.h"
#include "carla/road/Road.h"
#include "carla/road/element/Waypoint.h"

#include <cmath>

namespace carla {
namespace client {

  // Stepping away from the centre line: -1 -> -2, 1 -> 2.
  static road::LaneId StepOutward(road::LaneId lane_id) {
    return lane_id > 0 ? lane_id + 1 : lane_id - 1;
  }

  // Stepping toward the centre line. At +-1 the next lane across is on the
  // other side of the road, so the sign flips and the neighbour runs the
  // opposite way. road::Map::GetLeft does exactly this; kept identical here on
  // purpose so Lane and Waypoint never disagree about who is to the left.
  static road::LaneId StepInward(road::LaneId lane_id) {
    if (std::abs(lane_id) == 1) {
      return -lane_id;
    }
    return lane_id > 0 ? lane_id - 1 : lane_id + 1;
  }

  const road::Lane &Lane::GetLane() const {
    return _parent->GetMap().GetData().GetRoad(_road_id).GetLaneById(_section_id, _lane_id);
  }

  uint64_t Lane::GetUniqueId() const {
    // road id and section id are unsigned, lane id is signed and small; pack
    // them so the value is stable and collision-free within one map.
    return (static_cast<uint64_t>(_road_id) << 32) ^
           (static_cast<uint64_t>(_section_id) << 16) ^
           static_cast<uint64_t>(static_cast<uint16_t>(_lane_id));
  }

  road::Lane::LaneType Lane::GetType() const {
    return GetLane().GetType();
  }

  SharedPtr<Road> Lane::GetRoad() const {
    return _parent->GetRoad(_road_id);
  }

  SharedPtr<LaneSection> Lane::GetSection() const {
    return _parent->GetLaneSection(_road_id, _section_id);
  }

  double Lane::GetLength() const {
    return GetLane().GetLength();
  }

  double Lane::GetDistance() const {
    return GetLane().GetDistance();
  }

  bool Lane::GetLevel() const {
    return GetLane().GetLevel();
  }

  bool Lane::IsStraight() const {
    return GetLane().IsStraight();
  }

  bool Lane::IsPositiveDirection() const {
    return GetLane().IsPositiveDirection();
  }

  double Lane::GetWidth(double s) const {
    return GetLane().GetWidth(s);
  }

  geom::Transform Lane::GetTransform(double s) const {
    return GetLane().ComputeTransform(s);
  }

  SharedPtr<Lane> Lane::GetRightLane() const {
    if (_lane_id == 0) {
      return nullptr;
    }
    const bool is_rht = GetLane().GetRoad()->IsRHT();
    const road::LaneId neighbour = is_rht ? StepOutward(_lane_id) : StepInward(_lane_id);
    return _parent->GetLane(_road_id, _section_id, neighbour);
  }

  SharedPtr<Lane> Lane::GetLeftLane() const {
    if (_lane_id == 0) {
      return nullptr;
    }
    const bool is_rht = GetLane().GetRoad()->IsRHT();
    const road::LaneId neighbour = is_rht ? StepInward(_lane_id) : StepOutward(_lane_id);
    return _parent->GetLane(_road_id, _section_id, neighbour);
  }

  std::vector<SharedPtr<Lane>> Lane::GetNextLanes() const {
    std::vector<SharedPtr<Lane>> result;
    for (const road::Lane *lane : GetLane().GetNextLanes()) {
      if (lane == nullptr || lane->GetLaneSection() == nullptr || lane->GetRoad() == nullptr) {
        continue;
      }
      result.emplace_back(_parent->GetLane(
          lane->GetRoad()->GetId(), lane->GetLaneSection()->GetId(), lane->GetId()));
    }
    return result;
  }

  std::vector<SharedPtr<Lane>> Lane::GetPreviousLanes() const {
    std::vector<SharedPtr<Lane>> result;
    for (const road::Lane *lane : GetLane().GetPreviousLanes()) {
      if (lane == nullptr || lane->GetLaneSection() == nullptr || lane->GetRoad() == nullptr) {
        continue;
      }
      result.emplace_back(_parent->GetLane(
          lane->GetRoad()->GetId(), lane->GetLaneSection()->GetId(), lane->GetId()));
    }
    return result;
  }

  SharedPtr<Waypoint> Lane::GetWaypoint(double s) const {
    const road::Lane &lane = GetLane();
    const double start = lane.GetDistance();
    const double end = start + lane.GetLength();
    if (s < start || s > end) {
      return nullptr;
    }
    return _parent->MakeWaypoint(
        road::element::Waypoint{_road_id, _section_id, _lane_id, s});
  }

  std::vector<SharedPtr<Waypoint>> Lane::GetWaypoints(double distance) const {
    std::vector<SharedPtr<Waypoint>> result;
    if (distance <= 0.0) {
      return result;
    }
    const road::Lane &lane = GetLane();
    const double start = lane.GetDistance();
    const double end = start + lane.GetLength();
    for (double s = start; s < end; s += distance) {
      result.emplace_back(_parent->MakeWaypoint(
          road::element::Waypoint{_road_id, _section_id, _lane_id, s}));
    }
    return result;
  }

} // namespace client
} // namespace carla
