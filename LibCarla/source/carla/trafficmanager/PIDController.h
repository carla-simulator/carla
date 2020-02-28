// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <chrono>
#include <cmath>
#include <vector>

namespace carla {
namespace traffic_manager {

  namespace chr = std::chrono;
  using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;

  /// Structure to hold the actuation signals.
  struct ActuationSignal {
    float throttle;
    float brake;
    float steer;
  };

  /// Structure to hold the controller state.
  struct StateEntry {
    float deviation;
    float distance;
    float velocity;
    TimeInstance time_instance;
    float deviation_integral;
    float distance_integral;
    float velocity_integral;
  };

  /// This class calculates PID actuation signals to control the vehicle
  /// such that the vehicle maintains a target velocity and aligns its
  /// orientation along its trajectory.

  class PIDController {

  public:

    PIDController();

    /// This method calculates the present state of the vehicle including
    /// the accumulated integral component of the PID system.
    StateEntry StateUpdate(
        StateEntry previous_state,
        float current_velocity,
        float target_velocity,
        float angular_deviation,
        float distance,
        TimeInstance current_time);

    /// This method calculates the actuation signals based on the resent state
    /// change of the vehicle to minimize PID error.
    ActuationSignal RunStep(
        StateEntry present_state,
        StateEntry previous_state,
        const std::vector<float> &longitudinal_parameters,
        const std::vector<float> &lateral_parameters) const;

  };

} // namespace traffic_manager
} // namespace carla
