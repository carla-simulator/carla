// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace agents {
namespace navigation {

  /// Topological direction emitted by the planners. Mirrors
  /// agents.navigation.local_planner.RoadOption from PythonAPI.
  enum class RoadOption : int8_t {
    Void = -1,
    Left = 1,
    Right = 2,
    Straight = 3,
    LaneFollow = 4,
    ChangeLaneLeft = 5,
    ChangeLaneRight = 6,
  };

  /// Parameter set consumed by BehaviorAgent. Mirrors
  /// agents.navigation.behavior_types.{Cautious,Normal,Aggressive}.
  struct BehaviorParameters {
    float max_speed                = 50.0f;
    float speed_lim_dist           = 3.0f;
    float speed_decrease           = 10.0f;
    float safety_time              = 3.0f;
    float min_proximity_threshold  = 10.0f;
    float braking_distance         = 5.0f;
    int   tailgate_counter         = 0;

    static constexpr BehaviorParameters Cautious() {
      return {40.0f, 6.0f, 12.0f, 3.0f, 12.0f, 6.0f, 0};
    }
    static constexpr BehaviorParameters Normal() {
      return {50.0f, 3.0f, 10.0f, 3.0f, 10.0f, 5.0f, 0};
    }
    static constexpr BehaviorParameters Aggressive() {
      return {70.0f, 1.0f,  8.0f, 3.0f,  8.0f, 4.0f, -1};
    }
  };

} // namespace navigation
} // namespace agents
} // namespace carla
