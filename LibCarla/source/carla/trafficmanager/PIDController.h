// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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
                        const std::vector<float> &lateral_parameters,
                        const float vehicle_speed) {

  // Longitudinal PID calculation.
  const float expr_v =
      longitudinal_parameters[0] * present_state.velocity_deviation +
      longitudinal_parameters[1] * (present_state.velocity_deviation + previous_state.velocity_deviation) * DT +
      longitudinal_parameters[2] * (present_state.velocity_deviation - previous_state.velocity_deviation) * INV_DT;

  float throttle;
  float brake;

  if (expr_v > 0.0f) {
    throttle = std::min(expr_v, MAX_THROTTLE);
    brake = 0.0f;
  } else {
    throttle = 0.0f;
    brake = std::min(std::abs(expr_v), MAX_BRAKE);
  }

  // Lateral PID calculation. The derivative input is clamped to the maximum
  // physically plausible heading-error rate: a vehicle yaws well under
  // 180 deg/s, so |d(deviation)/dt| above that can only come from the target
  // point jumping discontinuously (lane change start, buffer reseed). An
  // unclamped derivative turns such jumps into a one-tick full-lock steer
  // spike that swings the vehicle across the lane.
  float deviation_delta = present_state.angular_deviation - previous_state.angular_deviation;
  deviation_delta = std::max(-MAX_DEVIATION_DELTA, std::min(deviation_delta, MAX_DEVIATION_DELTA));
  float steer =
      lateral_parameters[0] * present_state.angular_deviation +
      lateral_parameters[1] * (present_state.angular_deviation + previous_state.angular_deviation) * DT +
      lateral_parameters[2] * deviation_delta * INV_DT;

  steer = std::max(previous_state.steer - MAX_STEERING_DIFF, std::min(steer, previous_state.steer + MAX_STEERING_DIFF));
  steer = std::max(-MAX_STEERING, std::min(steer, MAX_STEERING));

  // Speed-scaled steering envelope (lateral-acceleration cap). See
  // STEER_LIMIT_GAIN in Constants.h.
  const float speed_sq = std::max(vehicle_speed * vehicle_speed, 1.0f);
  const float steer_limit = std::min(MAX_STEERING, STEER_LIMIT_GAIN / speed_sq);
  steer = std::max(-steer_limit, std::min(steer, steer_limit));

  return ActuationSignal{throttle, brake, steer};
}

} // namespace PID
} // namespace traffic_manager
} // namespace carla
