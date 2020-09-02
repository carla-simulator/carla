// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"
#include "carla/geom/CubicPolynomial.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/ListView.h"
#include "carla/Logging.h"
#include "carla/road/element/RoadInfoElevation.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/Lane.h"
#include "carla/road/MapData.h"
#include "carla/road/Road.h"

#include <stdexcept>

namespace carla {
namespace road {

  const MapData *Road::GetMap() const {
    return _map_data;
  }

  RoadId Road::GetId() const {
    return _id;
  }

  std::string Road::GetName() const {
    return _name;
  }

  double Road::GetLength() const {
    return _length;
  }

  bool Road::IsJunction() const {
    return _is_junction;
  }

  JuncId Road::GetJunctionId() const {
    return _junction_id;
  }

  RoadId Road::GetSuccessor() const {
    return _successor;
  }

  RoadId Road::GetPredecessor() const {
    return _predecessor;
  }

  std::vector<Road *> Road::GetNexts() const {
    return _nexts;
  }

  std::vector<Road *> Road::GetPrevs() const {
    return _prevs;
  }

  const geom::CubicPolynomial &Road::GetElevationOn(const double s) const {
    auto info = GetInfo<element::RoadInfoElevation>(s);
    if (info == nullptr) {
      throw_exception(std::runtime_error("failed to find road elevation."));
    }
    return info->GetPolynomial();
  }

  Lane &Road::GetLaneByDistance(double s, LaneId lane_id) {
    for (auto &section : GetLaneSectionsAt(s)) {
      auto *lane = section.GetLane(lane_id);
      if (lane != nullptr) {
        return *lane;
      }
    }
    throw_exception(std::runtime_error("lane not found"));
  }

  const Lane &Road::GetLaneByDistance(double s, LaneId lane_id) const {
    return const_cast<Road *>(this)->GetLaneByDistance(s, lane_id);
  }

  std::vector<Lane*> Road::GetLanesByDistance(double s) {
    std::vector<Lane*> result;
    auto lane_sections = GetLaneSectionsAt(s);
    for (auto &lane_section : lane_sections) {
      for (auto & lane_pair : lane_section.GetLanes()) {
        result.emplace_back(&lane_pair.second);
      }
    }
    return result;
  }

  std::vector<const Lane*> Road::GetLanesByDistance(double s) const {
    std::vector<const Lane*> result;
    const auto lane_sections = GetLaneSectionsAt(s);
    for (const auto &lane_section : lane_sections) {
      for (const auto & lane_pair : lane_section.GetLanes()) {
        result.emplace_back(&lane_pair.second);
      }
    }
    return result;
  }

  Lane &Road::GetLaneById(SectionId section_id, LaneId lane_id) {
    return GetLaneSectionById(section_id).GetLanes().at(lane_id);
  }

  const Lane &Road::GetLaneById(SectionId section_id, LaneId lane_id) const {
    return const_cast<Road *>(this)->GetLaneById(section_id, lane_id);
  }

  // get the lane on a section next to 's'
  Lane *Road::GetNextLane(const double s, const LaneId lane_id) {

    auto upper = _lane_sections.upper_bound(s);

    while (upper != _lane_sections.end()) {
      // check id
      Lane *ptr = upper->second.GetLane(lane_id);
      if (ptr != nullptr) {
        return ptr;
      }
      ++upper;
    }

    return nullptr;
  }

  // get the lane on a section previous to 's'
  Lane *Road::GetPrevLane(const double s, const LaneId lane_id) {

    auto lower = _lane_sections.lower_bound(s);
    auto rlower = std::make_reverse_iterator(lower);

    while (rlower != _lane_sections.rend()) {
      // check id
      Lane *ptr = rlower->second.GetLane(lane_id);
      if (ptr != nullptr) {
        return ptr;
      }
      ++rlower;
    }

    return nullptr;
  }

  // get the start and end section with a lan id
  LaneSection *Road::GetStartSection(LaneId id) {
    auto it = _lane_sections.begin();
    while (it != _lane_sections.end()) {
      // check id
      Lane *ptr = it->second.GetLane(id);
      if (ptr != nullptr) {
        return &(it->second);
      }
      ++it;
    }
    return nullptr;
  }

  LaneSection *Road::GetEndSection(LaneId id) {
    auto it = _lane_sections.rbegin();
    while (it != _lane_sections.rend()) {
      // check id
      Lane *ptr = it->second.GetLane(id);
      if (ptr != nullptr) {
        return &(it->second);
      }
      ++it;
    }
    return nullptr;
  }

  element::DirectedPoint Road::GetDirectedPointIn(const double s) const {
    const auto clamped_s = geom::Math::Clamp(s, 0.0, _length);
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s);

    const auto lane_offset = _info.GetInfo<element::RoadInfoLaneOffset>(clamped_s);
    float offset = 0;
    if(lane_offset){
      offset = static_cast<float>(lane_offset->GetPolynomial().Evaluate(clamped_s));
    }
    // Apply road's lane offset record
    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance());
    // Unreal's Y axis hack (the minus on the offset)
    p.ApplyLateralOffset(-offset);

    // Apply road's elevation record
    const auto elevation_info = GetElevationOn(s);
    p.location.z = static_cast<float>(elevation_info.Evaluate(s));
    p.pitch = elevation_info.Tangent(s);

    return p;
  }

  element::DirectedPoint Road::GetDirectedPointInNoLaneOffset(const double s) const {
    const auto clamped_s = geom::Math::Clamp(s, 0.0, _length);
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s);

    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance());

    // Apply road's elevation record
    const auto elevation_info = GetElevationOn(s);
    p.location.z = static_cast<float>(elevation_info.Evaluate(s));
    p.pitch = elevation_info.Tangent(s);

    return p;
  }

  const std::pair<double, double> Road::GetNearestPoint(const geom::Location &loc) const {
    std::pair<double, double> last = { 0.0, std::numeric_limits<double>::max() };

    auto geom_info_list = _info.GetInfos<element::RoadInfoGeometry>();
    decltype(geom_info_list)::iterator nearest_geom = geom_info_list.end();

    for (auto g = geom_info_list.begin(); g != geom_info_list.end(); ++g) {
      DEBUG_ASSERT(*g != nullptr);
      auto dist = (*g)->GetGeometry().DistanceTo(loc);
      if (dist.second < last.second) {
        last = dist;
        nearest_geom = g;
      }
    }

    for (auto g = geom_info_list.begin();
        g != geom_info_list.end() && g != nearest_geom;
        ++g) {
      DEBUG_ASSERT(*g != nullptr);
      last.first += (*g)->GetGeometry().GetLength();
    }

    return last;
  }

  const std::pair<const Lane *, double> Road::GetNearestLane(
      const double s,
      const geom::Location &loc,
      uint32_t lane_type) const {
    using namespace carla::road::element;
    std::map<LaneId, const Lane *> lanes(GetLanesAt(s));
    // negative right lanes
    auto right_lanes = MakeListView(
        std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
    // positive left lanes
    auto left_lanes = MakeListView(
        lanes.lower_bound(1), lanes.end());

    const DirectedPoint dp_lane_zero = GetDirectedPointIn(s);
    std::pair<const Lane *, double> result =
        std::make_pair(nullptr, std::numeric_limits<double>::max());

    DirectedPoint current_dp = dp_lane_zero;
    for (const auto &lane : right_lanes) {
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s);
      const auto half_width = static_cast<float>(lane_width_info->GetPolynomial().Evaluate(s)) * 0.5f;

      current_dp.ApplyLateralOffset(half_width);
      const auto current_dist = geom::Math::Distance(current_dp.location, loc);

      // if the current_dp is near to loc, we are in the right way
      if (current_dist <= result.second) {
        // only consider the lanes that match the type flag for result
        // candidates
        if ((static_cast<uint32_t>(lane.second->GetType()) & lane_type) > 0) {
          result.first = &(*lane.second);
          result.second = current_dist;
        }
      } else {
        // elsewhere, we are be moving away
        break;
      }
      current_dp.ApplyLateralOffset(half_width);
    }

    current_dp = dp_lane_zero;
    for (const auto &lane : left_lanes) {
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s);
      const auto half_width = -static_cast<float>(lane_width_info->GetPolynomial().Evaluate(s)) * 0.5f;

      current_dp.ApplyLateralOffset(half_width);
      const auto current_dist = geom::Math::Distance(current_dp.location, loc);

      // if the current_dp is near to loc, we are in the right way
      if (current_dist <= result.second) {
        // only consider the lanes that match the type flag for result
        // candidates
        if ((static_cast<uint32_t>(lane.second->GetType()) & lane_type) > 0) {
          result.first = &(*lane.second);
          result.second = current_dist;
        }
      } else {
        // elsewhere, we are be moving away
        break;
      }
      current_dp.ApplyLateralOffset(half_width);
    }

    return result;
  }

  std::map<LaneId, const Lane *> Road::GetLanesAt(const double s) const {
    std::map<LaneId, const Lane *> map;
    for (auto &&lane_section : GetLaneSectionsAt(s)) {
      for (auto &&lane : lane_section.GetLanes()) {
        map[lane.first] = &(lane.second);
      }
    }
    return map;
  }

} // road
} // carla
