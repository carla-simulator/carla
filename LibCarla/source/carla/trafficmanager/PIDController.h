// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
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

/// This function calculates the actuation signals based on the resent state
/// change of the vehicle to minimize PID error.
inline ActuationSignal RunStep(StateEntry present_state,
                        StateEntry previous_state,
                        const std::vector<float> &longitudinal_parameters,
                        const std::vector<float> &lateral_parameters) {

  double const delta_time = present_state.time_instance.elapsed_seconds - previous_state.time_instance.elapsed_seconds;
  if ( delta_time <= 1e-6) {
    // don't update the actuation signal if the time difference is too small to avoid erratic behavior due to noise in the state measurements.
    return previous_state.actuation_signal;
  }
  double const delta_time_inv = 1.0 / delta_time;

  // Longitudinal PID calculation.
  const float expr_v =
      longitudinal_parameters[0] * present_state.velocity_deviation +
      longitudinal_parameters[1] * (present_state.velocity_deviation + previous_state.velocity_deviation) * delta_time +
      longitudinal_parameters[2] * (present_state.velocity_deviation - previous_state.velocity_deviation) * delta_time_inv;

  float throttle;
  float brake;

  if (expr_v > 0.0f) {
    throttle = std::min(expr_v, MAX_THROTTLE);
    brake = 0.0f;
  } else {
    throttle = 0.0f;
    brake = std::min(std::abs(expr_v), MAX_BRAKE);
  }

  // Lateral PID calculation.
  float steer =
      lateral_parameters[0] * present_state.angular_deviation +
      lateral_parameters[1] * (present_state.angular_deviation + previous_state.angular_deviation) * delta_time +
      lateral_parameters[2] * (present_state.angular_deviation - previous_state.angular_deviation) * delta_time_inv;

  steer = std::max(previous_state.actuation_signal.steer - MAX_STEERING_DIFF, std::min(steer, previous_state.actuation_signal.steer + MAX_STEERING_DIFF));
  steer = std::max(-MAX_STEERING, std::min(steer, MAX_STEERING));

  return ActuationSignal{throttle, brake, steer};
}

} // namespace PID
} // namespace traffic_manager
} // namespace carla
