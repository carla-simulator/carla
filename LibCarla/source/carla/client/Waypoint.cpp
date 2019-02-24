// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Waypoint.h"

#include "carla/client/Map.h"
#include "carla/road/WaypointGenerator.h"

#include <boost/optional.hpp>

namespace carla {
namespace client {

  Waypoint::Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint)
    : _parent(std::move(parent)),
      _waypoint(std::move(waypoint)),
      _transform(_waypoint.ComputeTransform()),
      _mark_record(_waypoint.GetMarkRecord()) {}

  Waypoint::~Waypoint() = default;

  std::vector<SharedPtr<Waypoint>> Waypoint::Next(double distance) const {
    auto waypoints = road::WaypointGenerator::GetNext(_waypoint, distance);
    std::vector<SharedPtr<Waypoint>> result;
    result.reserve(waypoints.size());
    for (auto &waypoint : waypoints) {
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint(_parent, std::move(waypoint))));
    }
    return result;
  }

  SharedPtr<Waypoint> Waypoint::Right() const {
    auto right_lane_waypoint =
        road::WaypointGenerator::GetRight(_waypoint);
    if (right_lane_waypoint.has_value()) {
      return SharedPtr<Waypoint>(new Waypoint(_parent, std::move(*right_lane_waypoint)));
    }
    return nullptr;
  }

  SharedPtr<Waypoint> Waypoint::Left() const {
    auto left_lane_waypoint =
        road::WaypointGenerator::GetLeft(_waypoint);
    if (left_lane_waypoint.has_value()) {
      return SharedPtr<Waypoint>(new Waypoint(_parent, std::move(*left_lane_waypoint)));
    }
    return nullptr;
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

  Waypoint::LaneChange Waypoint::GetLaneChange() const {
    const auto lane_change_right = _mark_record.first.GetLaneChange();
    const auto lane_change_left = _mark_record.second.GetLaneChange();

    auto c_right = static_cast<Waypoint::LaneChange>(lane_change_right);
    auto c_left = static_cast<Waypoint::LaneChange>(lane_change_left);

    if (_mark_record.first.GetLaneId() > 0) {
      // if road goes backward
      if (c_right == Waypoint::LaneChange::Right) {
        c_right = Waypoint::LaneChange::Left;
      } else if (c_right == Waypoint::LaneChange::Left) {
        c_right = Waypoint::LaneChange::Right;
      }
    }

    if (_mark_record.second.GetLaneId() > 0) {
      // if road goes backward
      if (c_left == Waypoint::LaneChange::Right) {
        c_left = Waypoint::LaneChange::Left;
      } else if (c_left == Waypoint::LaneChange::Left) {
        c_left = Waypoint::LaneChange::Right;
      }
    }

    return (c_right & Waypoint::LaneChange::Right) | (c_left & Waypoint::LaneChange::Left);
  }

} // namespace client
} // namespace carla
