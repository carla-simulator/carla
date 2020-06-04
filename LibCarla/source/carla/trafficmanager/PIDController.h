// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/DataStructures.h"

namespace carla {
namespace traffic_manager {

namespace chr = std::chrono;

using namespace constants::PID;

using TimeInstance = chr::time_point<chr::system_clock, chr::nanoseconds>;

namespace PID {

/// This function calculates the present state of the vehicle including
/// the accumulated integral component of the PID system.
StateEntry StateUpdate(StateEntry previous_state,
                       float current_velocity,
                       float target_velocity,
                       float angular_deviation,
                       TimeInstance current_time) {
  StateEntry current_state = {
      current_time,
      angular_deviation,
      (current_velocity - target_velocity) / target_velocity,
      0.0f,
      0.0f};

  // Calculating integrals.
  current_state.deviation_integral = angular_deviation * DT + previous_state.deviation_integral;
  current_state.velocity_integral = DT * current_state.velocity + previous_state.velocity_integral;

  // Clamp velocity integral to avoid accumulating over-compensation
  // with time for vehicles that take a long time to reach target velocity.
  current_state.velocity_integral = std::min(VELOCITY_INTEGRAL_MAX, std::max(current_state.velocity_integral, VELOCITY_INTEGRAL_MIN));

  return current_state;
}

/// This function calculates the actuation signals based on the resent state
/// change of the vehicle to minimize PID error.
ActuationSignal RunStep(StateEntry present_state,
                        StateEntry previous_state,
                        const std::vector<float> &longitudinal_parameters,
                        const std::vector<float> &lateral_parameters) {

  // Longitudinal PID calculation.
  const float expr_v =
      longitudinal_parameters[0] * present_state.velocity +
      longitudinal_parameters[1] * present_state.velocity_integral +
      longitudinal_parameters[2] * (present_state.velocity - previous_state.velocity) * INV_DT;

  float throttle;
  float brake;

  if (expr_v < 0.0f) {
    throttle = std::min(std::abs(expr_v), MAX_THROTTLE);
    brake = 0.0f;
  } else {
    throttle = 0.0f;
    brake = std::min(expr_v, MAX_BRAKE);
  }

  // Lateral PID calculation.
  float steer =
      lateral_parameters[0] * present_state.deviation +
      lateral_parameters[1] * present_state.deviation_integral +
      lateral_parameters[2] * (present_state.deviation - previous_state.deviation) * INV_DT;

  steer = std::max(-0.8f, std::min(steer, 0.8f));

  return ActuationSignal{throttle, brake, steer};
}

} // namespace PID
} // namespace traffic_manager
} // namespace carla
