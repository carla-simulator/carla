// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/Road.h"
#include "carla/road/Lane.h"
#include "carla/road/MapData.h"
#include "carla/geom/Math.h"
#include "carla/Logging.h"
#include "carla/ListView.h"
#include "carla/geom/Location.h"
#include "carla/geom/CubicPolynomial.h"
#include "carla/road/element/RoadInfoGeometry.h"
#include "carla/road/element/RoadInfoLaneOffset.h"
#include "carla/road/element/RoadInfoLaneWidth.h"
#include "carla/road/element/RoadInfoElevation.h"

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

  float Road::GetLength() const {
    return _length;
  }

  bool Road::IsJunction() const {
    return _is_junction;
  }

  JuncId Road::GetJunction() const {
    return _junction_id;
  }

  std::vector<const Road *> Road::GetNexts() const {
    std::vector<const Road *> vec;
    for (auto &&next : _nexts) {
      vec.emplace_back(_map_data->GetRoad(next));
    }
    return vec;
  }

  std::vector<const Road *> Road::GetPrevs() const {
    std::vector<const Road *> vec;
    for (auto &&prev : _prevs) {
      vec.emplace_back(_map_data->GetRoad(prev));
    }
    return vec;
  }

  const geom::CubicPolynomial &Road::GetElevationOn(const float s) const {
    return GetInfo<element::RoadInfoElevation>(s)->GetPolynomial();
  }

  Lane *Road::GetLane(const LaneId id, const float s) {
    for (auto &lane_section : GetLaneSectionsAt(s)) {
      auto search = lane_section.GetLanes().find(id);
      if (search != lane_section.GetLanes().end()) {
        return &search->second;
      }
    }
    return nullptr;
  }

  const Lane *Road::GetLane(const LaneId id, const float s) const {
    for (auto &lane_section : GetLaneSectionsAt(s)) {
      auto search = lane_section.GetLanes().find(id);
      if (search != lane_section.GetLanes().end()) {
        return &search->second;
      }
    }
    return nullptr;
  }

  // get the lane on a section next to 's'
  Lane *Road::GetNextLane(const float s, const LaneId lane_id) {

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
  Lane *Road::GetPrevLane(const float s, const LaneId lane_id) {

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

  carla::road::signal::Signal *Road::GetSignal(const SignId id) {
    auto search = _signals.find(id);
    if (search != _signals.end()) {
      return &search->second;
    }
    return nullptr;
  }

  carla::road::signal::SignalReference *Road::GetSignalRef(const SignRefId id) {
    const auto search = _sign_ref.find(id);
    if (search != _sign_ref.end()) {
      return &search->second;
    }
    return nullptr;
  }

  std::unordered_map<SignId, signal::Signal> *Road::getSignals() {
    return &_signals;
  }

  std::unordered_map<SignId, signal::SignalReference> *Road::getSignalReferences() {
    return &_sign_ref;
  }

  element::DirectedPoint Road::GetDirectedPointIn(const float s) const {
    const float clamped_s = geom::Math::clamp(s, 0.0f, _length);
    const auto geometry = _info.GetInfo<element::RoadInfoGeometry>(clamped_s);

    const auto lane_offset = _info.GetInfo<element::RoadInfoLaneOffset>(clamped_s);
    const float offset = lane_offset->GetPolynomial().Evaluate(clamped_s);

    element::DirectedPoint p = geometry->GetGeometry().PosFromDist(clamped_s - geometry->GetDistance());
    // Unreal's Y axis hack
    p.ApplyLateralOffset(-offset);

    const auto elevation_info = GetElevationOn(s);
    p.location.z = elevation_info.Evaluate(s);
    p.pitch = elevation_info.Tangent(s);

    return p;
  }

  const std::pair<float, float> Road::GetNearestPoint(const geom::Location &loc) const {
    std::pair<float, float> last = { 0.0, std::numeric_limits<float>::max() };

    auto geom_info_list = _info.GetInfos<element::RoadInfoGeometry>();
    decltype(geom_info_list)::iterator nearest_geom = geom_info_list.end();

    for (auto g = geom_info_list.begin(); g != geom_info_list.end(); ++g) {
      auto dist = (*g)->GetGeometry().DistanceTo(loc);
      if (dist.second < last.second) {
        last = dist;
        nearest_geom = g;
      }
    }

    for (auto g = geom_info_list.begin();
        g != geom_info_list.end() && g != nearest_geom;
        ++g) {
      last.first += (*g)->GetGeometry().GetLength();
    }

    return last;
  }

  const std::pair<const Lane *, float> Road::GetNearestLane(
      const float s,
      const geom::Location &loc) const {
    using namespace carla::road::element;
    std::map<LaneId, const Lane *> lanes(GetLanesAt(s));
    // negative right lanes
    auto right_lanes = MakeListView(
        std::make_reverse_iterator(lanes.lower_bound(0)), lanes.rend());
    // positive left lanes
    auto left_lanes = MakeListView(
        lanes.lower_bound(1), lanes.end());

    DirectedPoint dp_lane_zero = GetDirectedPointIn(s);
    std::pair<const Lane *, float> result =
        std::make_pair(nullptr, std::numeric_limits<float>::max());

    // Unreal's Y axis hack
    // dp_lane_zero.location.y *= -1;

    DirectedPoint current_dp = dp_lane_zero;
    for (const auto &lane : right_lanes) {
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s);
      const auto half_width = lane_width_info->GetPolynomial().Evaluate(s) / 2.0f;
      current_dp.ApplyLateralOffset(half_width);
      const auto current_dist = geom::Math::Distance(current_dp.location, loc);
      // if the current_dp is near to loc, we are in the right way
      if (current_dist <= result.second) {
        result.first = &(*lane.second);
        result.second = current_dist;
      } else {
        // elsewhere, we are be moving away
        break;
      }
      current_dp.ApplyLateralOffset(half_width);
    }

    current_dp = dp_lane_zero;
    for (const auto &lane : left_lanes) {
      const auto lane_width_info = lane.second->GetInfo<RoadInfoLaneWidth>(s);
      const auto half_width = -lane_width_info->GetPolynomial().Evaluate(s) / 2.0f;
      current_dp.ApplyLateralOffset(half_width);
      const auto current_dist = geom::Math::Distance(current_dp.location, loc);
      // if the current_dp is near to loc, we are in the right way
      if (current_dist <= result.second) {
        result.first = &(*lane.second);
        result.second = current_dist;
      } else {
        // elsewhere, we are be moving away
        break;
      }
      current_dp.ApplyLateralOffset(half_width);
    }

    return result;
  }

  std::map<LaneId, const Lane *> Road::GetLanesAt(const float s) const {
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
