// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Waypoint.h"

#include "carla/client/Map.h"
#include "carla/client/Junction.h"
#include "carla/client/Landmark.h"

#include <unordered_set>

namespace carla {
namespace client {

  Waypoint::Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint)
    : _parent(std::move(parent)),
      _waypoint(std::move(waypoint)),
      _transform(_parent->GetMap().ComputeTransform(_waypoint)),
      _mark_record(_parent->GetMap().GetMarkRecord(_waypoint)) {}

  Waypoint::~Waypoint() = default;

  road::JuncId Waypoint::GetJunctionId() const {
    return _parent->GetMap().GetJunctionId(_waypoint.road_id);
  }

  bool Waypoint::IsJunction() const {
    return _parent->GetMap().IsJunction(_waypoint.road_id);
  }

  SharedPtr<Junction> Waypoint::GetJunction() const {
    if (IsJunction()) {
      return _parent->GetJunction(*this);
    }
    return nullptr;
  }

  double Waypoint::GetLaneWidth() const {
    return _parent->GetMap().GetLaneWidth(_waypoint);

  }

  road::Lane::LaneType Waypoint::GetType() const {
    return _parent->GetMap().GetLaneType(_waypoint);
  }

  std::vector<SharedPtr<Waypoint>> Waypoint::GetNext(double distance) const {
    auto waypoints = _parent->GetMap().GetNext(_waypoint, distance);
    std::vector<SharedPtr<Waypoint>> result;
    result.reserve(waypoints.size());
    for (auto &waypoint : waypoints) {
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint(_parent, std::move(waypoint))));
    }
    return result;
  }

  std::vector<SharedPtr<Waypoint>> Waypoint::GetPrevious(double distance) const {
    auto waypoints = _parent->GetMap().GetPrevious(_waypoint, distance);
    std::vector<SharedPtr<Waypoint>> result;
    result.reserve(waypoints.size());
    for (auto &waypoint : waypoints) {
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint(_parent, std::move(waypoint))));
    }
    return result;
  }

  std::vector<SharedPtr<Waypoint>> Waypoint::GetNextUntilLaneEnd(double distance) const {
    std::vector<SharedPtr<Waypoint>> result;
    std::vector<SharedPtr<Waypoint>> next = GetNext(distance);

    while (next.size() == 1 && next.front()->GetRoadId() == GetRoadId()) {
      result.emplace_back(next.front());
      next = result.back()->GetNext(distance);
    }
    double current_s = GetDistance();
    if(result.size()) {
      current_s = result.back()->GetDistance();
    }
    double remaining_length;
    double road_length = _parent->GetMap().GetLane(_waypoint).GetRoad()->GetLength();
    if(_waypoint.lane_id < 0) {
      remaining_length = road_length - current_s;
    } else {
      remaining_length = current_s;
    }
    remaining_length -= std::numeric_limits<double>::epsilon();
    if(result.size()) {
      result.emplace_back(result.back()->GetNext(remaining_length).front());
    } else {
      result.emplace_back(GetNext(remaining_length).front());
    }

    return result;
  }

  std::vector<SharedPtr<Waypoint>> Waypoint::GetPreviousUntilLaneStart(double distance) const {
    std::vector<SharedPtr<Waypoint>> result;
    std::vector<SharedPtr<Waypoint>> prev = GetPrevious(distance);

    while (prev.size() == 1 && prev.front()->GetRoadId() == GetRoadId()) {
      result.emplace_back(prev.front());
      prev = result.back()->GetPrevious(distance);
    }

    double current_s = GetDistance();
    if(result.size()) {
      current_s = result.back()->GetDistance();
    }

    double remaining_length;
    double road_length = _parent->GetMap().GetLane(_waypoint).GetRoad()->GetLength();
    if(_waypoint.lane_id < 0) {
      remaining_length = road_length - current_s;
    } else {
      remaining_length = current_s;
    }
    remaining_length -= std::numeric_limits<double>::epsilon();
    if(result.size()) {
      result.emplace_back(result.back()->GetPrevious(remaining_length).front());
    } else {
      result.emplace_back(GetPrevious(remaining_length).front());
    }

    return result;
  }

  SharedPtr<Waypoint> Waypoint::GetRight() const {
    auto right_lane_waypoint =
        _parent->GetMap().GetRight(_waypoint);
    if (right_lane_waypoint.has_value()) {
      return SharedPtr<Waypoint>(new Waypoint(_parent, std::move(*right_lane_waypoint)));
    }
    return nullptr;
  }

  SharedPtr<Waypoint> Waypoint::GetLeft() const {
    auto left_lane_waypoint =
        _parent->GetMap().GetLeft(_waypoint);
    if (left_lane_waypoint.has_value()) {
      return SharedPtr<Waypoint>(new Waypoint(_parent, std::move(*left_lane_waypoint)));
    }
    return nullptr;
  }

  std::optional<road::element::LaneMarking> Waypoint::GetRightLaneMarking() const {
    if (_mark_record.first != nullptr) {
      return road::element::LaneMarking(*_mark_record.first);
    }
    return std::optional<road::element::LaneMarking>{};
  }

  std::optional<road::element::LaneMarking> Waypoint::GetLeftLaneMarking() const {
    if (_mark_record.second != nullptr) {
      return road::element::LaneMarking(*_mark_record.second);
    }
    return std::optional<road::element::LaneMarking>{};
  }

  template <typename EnumT>
  static EnumT operator&(EnumT lhs, EnumT rhs) {
    return static_cast<EnumT>(
        static_cast<typename std::underlying_type<EnumT>::type>(lhs) &
        static_cast<typename std::underlying_type<EnumT>::type>(rhs));
  }

  template <typename EnumT>
  static EnumT operator|(EnumT lhs, EnumT rhs) {
    return static_cast<EnumT>(
        static_cast<typename std::underlying_type<EnumT>::type>(lhs) |
        static_cast<typename std::underlying_type<EnumT>::type>(rhs));
  }

  road::element::LaneMarking::LaneChange Waypoint::GetLaneChange() const {
    using lane_change_type = road::element::LaneMarking::LaneChange;

    const auto lane_change_right_info = _mark_record.first;
    lane_change_type c_right;
    if (lane_change_right_info != nullptr) {
      const auto lane_change_right = lane_change_right_info->GetLaneChange();
      c_right = static_cast<lane_change_type>(lane_change_right);
    } else {
      c_right = lane_change_type::Both;
    }

    const auto lane_change_left_info = _mark_record.second;
    lane_change_type c_left;
    if (lane_change_left_info != nullptr) {
      const auto lane_change_left = lane_change_left_info->GetLaneChange();
      c_left = static_cast<lane_change_type>(lane_change_left);
    } else {
      c_left = lane_change_type::Both;
    }

    if (_waypoint.lane_id > 0) {
      // if road goes backward
      if (c_right == lane_change_type::Right) {
        c_right = lane_change_type::Left;
      } else if (c_right == lane_change_type::Left) {
        c_right = lane_change_type::Right;
      }
    }

    if (((_waypoint.lane_id > 0) ? _waypoint.lane_id - 1 : _waypoint.lane_id + 1) > 0) {
      // if road goes backward
      if (c_left == lane_change_type::Right) {
        c_left = lane_change_type::Left;
      } else if (c_left == lane_change_type::Left) {
        c_left = lane_change_type::Right;
      }
    }

    return (c_right & lane_change_type::Right) | (c_left & lane_change_type::Left);
  }

  std::vector<SharedPtr<Landmark>> Waypoint::GetAllLandmarksInDistance(
      double distance, bool stop_at_junction) const {
    std::vector<SharedPtr<Landmark>> result;
    auto signals = _parent->GetMap().GetSignalsInDistance(
        _waypoint, distance, stop_at_junction);
    std::unordered_set<const road::element::RoadInfoSignal*> added_signals; // check for repeated signals
    for(auto &signal_data : signals){
      if(added_signals.count(signal_data.signal) > 0) {
        continue;
      }
      added_signals.insert(signal_data.signal);
      auto waypoint = SharedPtr<Waypoint>(new Waypoint(_parent, signal_data.waypoint));
      result.emplace_back(
          new Landmark(waypoint, _parent, signal_data.signal, signal_data.accumulated_s));
    }
    return result;
  }

  std::vector<SharedPtr<Landmark>> Waypoint::GetLandmarksOfTypeInDistance(
        double distance, std::string filter_type, bool stop_at_junction) const {
    std::vector<SharedPtr<Landmark>> result;
    std::unordered_set<const road::element::RoadInfoSignal*> added_signals; // check for repeated signals
    auto signals = _parent->GetMap().GetSignalsInDistance(
        _waypoint, distance, stop_at_junction);
    for(auto &signal_data : signals){
      if(signal_data.signal->GetSignal()->GetType() == filter_type) {
        if(added_signals.count(signal_data.signal) > 0) {
          continue;
        }
        auto waypoint = SharedPtr<Waypoint>(new Waypoint(_parent, signal_data.waypoint));
        result.emplace_back(
            new Landmark(waypoint, _parent, signal_data.signal, signal_data.accumulated_s));
      }
    }
    return result;
  }

} // namespace client
} // namespace carla
