// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Road.h"

#include "carla/geom/Math.h"

#include "carla/client/Junction.h"
#include "carla/client/Lane.h"
#include "carla/client/LaneSection.h"
#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include "carla/road/LaneSection.h"
#include "carla/road/Road.h"
#include "carla/road/element/Waypoint.h"

#include <unordered_set>

namespace carla {
namespace client {

  const road::Road &GetRoadData(const SharedPtr<const Map> &parent, road::RoadId road_id) {
    return parent->GetMap().GetData().GetRoad(road_id);
  }

  std::string Road::GetName() const {
    return GetRoadData(_parent, _road_id).GetName();
  }

  double Road::GetLength() const {
    return GetRoadData(_parent, _road_id).GetLength();
  }

  bool Road::IsJunction() const {
    return GetRoadData(_parent, _road_id).IsJunction();
  }

  road::JuncId Road::GetJunctionId() const {
    return GetRoadData(_parent, _road_id).GetJunctionId();
  }

  bool Road::IsRHT() const {
    return GetRoadData(_parent, _road_id).IsRHT();
  }

  road::RoadId Road::GetSuccessorId() const {
    return GetRoadData(_parent, _road_id).GetSuccessor();
  }

  road::RoadId Road::GetPredecessorId() const {
    return GetRoadData(_parent, _road_id).GetPredecessor();
  }

  std::vector<SharedPtr<Road>> Road::GetNextRoads() const {
    std::vector<SharedPtr<Road>> result;
    std::unordered_set<road::RoadId> seen;
    for (const road::Road *road : GetRoadData(_parent, _road_id).GetNexts()) {
      if (road == nullptr || !seen.insert(road->GetId()).second) {
        continue;
      }
      result.emplace_back(_parent->GetRoad(road->GetId()));
    }
    return result;
  }

  std::vector<SharedPtr<Road>> Road::GetPreviousRoads() const {
    std::vector<SharedPtr<Road>> result;
    std::unordered_set<road::RoadId> seen;
    for (const road::Road *road : GetRoadData(_parent, _road_id).GetPrevs()) {
      if (road == nullptr || !seen.insert(road->GetId()).second) {
        continue;
      }
      result.emplace_back(_parent->GetRoad(road->GetId()));
    }
    return result;
  }

  std::vector<SharedPtr<Lane>> Road::GetLanes(double s, road::Lane::LaneType type) const {
    std::vector<SharedPtr<Lane>> result;
    const road::Road &road = GetRoadData(_parent, _road_id);
    for (const road::Lane *lane : road.GetLanesByDistance(s)) {
      if (lane == nullptr || lane->GetId() == 0 || lane->GetLaneSection() == nullptr) {
        continue;
      }
      if ((static_cast<int32_t>(lane->GetType()) & static_cast<int32_t>(type)) == 0) {
        continue;
      }
      result.emplace_back(_parent->GetLane(
          _road_id, lane->GetLaneSection()->GetId(), lane->GetId()));
    }
    return result;
  }

  SharedPtr<Lane> Road::GetLane(road::SectionId section_id, road::LaneId lane_id) const {
    return _parent->GetLane(_road_id, section_id, lane_id);
  }

  SharedPtr<Lane> Road::GetLaneAt(double s, road::LaneId lane_id) const {
    SharedPtr<LaneSection> section = GetSectionAt(s);
    if (section == nullptr) {
      return nullptr;
    }
    return _parent->GetLane(_road_id, section->GetId(), lane_id);
  }

  std::vector<SharedPtr<LaneSection>> Road::GetSections() const {
    std::vector<SharedPtr<LaneSection>> result;
    for (const road::LaneSection &section : GetRoadData(_parent, _road_id).GetLaneSections()) {
      result.emplace_back(_parent->GetLaneSection(_road_id, section.GetId()));
    }
    return result;
  }

  SharedPtr<LaneSection> Road::GetSection(road::SectionId section_id) const {
    return _parent->GetLaneSection(_road_id, section_id);
  }

  SharedPtr<LaneSection> Road::GetSectionAt(double s) const {
    const road::Road &road = GetRoadData(_parent, _road_id);
    // GetLaneSectionsAt yields every section starting at or before s; the last
    // is the one that actually covers it.
    const road::LaneSection *covering = nullptr;
    for (const road::LaneSection &section : road.GetLaneSectionsAt(s)) {
      covering = &section;
    }
    if (covering == nullptr) {
      return nullptr;
    }
    return _parent->GetLaneSection(_road_id, covering->GetId());
  }

  SharedPtr<Junction> Road::GetJunction() const {
    if (!IsJunction()) {
      return nullptr;
    }
    return _parent->GetJunctionById(GetJunctionId());
  }

  geom::Transform Road::GetTransformAt(double s) const {
    auto directed_point = GetRoadData(_parent, _road_id).GetDirectedPointIn(s);
    // Same Unreal Y-axis flip road::Lane::ComputeTransform applies; without it
    // the transform comes back in the OpenDRIVE frame and disagrees with every
    // waypoint on the same road.
    directed_point.location.y *= -1.0f;
    directed_point.tangent *= -1.0;
    return geom::Transform(
        directed_point.location,
        geom::Rotation(
            geom::Math::ToDegrees(static_cast<float>(directed_point.pitch)),
            geom::Math::ToDegrees(static_cast<float>(directed_point.tangent)),
            0.0f));
  }

  std::vector<SharedPtr<Waypoint>> Road::GetWaypointsAt(
      double s,
      road::Lane::LaneType type) const {
    std::vector<SharedPtr<Waypoint>> result;
    for (const SharedPtr<Lane> &lane : GetLanes(s, type)) {
      SharedPtr<Waypoint> waypoint = lane->GetWaypoint(s);
      if (waypoint != nullptr) {
        result.emplace_back(std::move(waypoint));
      }
    }
    return result;
  }

  std::vector<SharedPtr<Waypoint>> Road::GetWaypoints(
      double distance,
      road::Lane::LaneType type) const {
    std::vector<SharedPtr<Waypoint>> result;
    if (distance <= 0.0) {
      return result;
    }
    const double length = GetLength();
    for (double s = 0.0; s < length; s += distance) {
      for (SharedPtr<Waypoint> &waypoint : GetWaypointsAt(s, type)) {
        result.emplace_back(std::move(waypoint));
      }
    }
    return result;
  }

} // namespace client
} // namespace carla
