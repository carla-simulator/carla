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
///
/// control_dt is the measured simulation-time period between the two states.
/// The gains are tuned at the nominal DT (sync-mode 0.05 s); in asynchronous
/// mode the real period is one server frame, which under render load (e.g. a
/// path-traced rt_lens sensor) stretches to 2-4x nominal. Left uncompensated
/// that halves the loop's phase margin and the lateral PID goes from damped
/// to divergent weaving (measured on Town12: steer_std 0.11 at 19 fps ->
/// 0.71 with full-lock swings at 11 fps), which then trips the stuck/K-turn
/// recovery into alternating full-lock "jerking".
inline ActuationSignal RunStep(StateEntry present_state,
                        StateEntry previous_state,
                        const std::vector<float> &longitudinal_parameters,
                        const std::vector<float> &lateral_parameters,
                        const float vehicle_speed,
                        const float control_dt = DT) {

  const float dt = std::max(MIN_CONTROL_DT, std::min(control_dt, MAX_CONTROL_DT));
  const float inv_dt = 1.0f / dt;
  // Sample-rate compensation for the lateral loop. The dominant effect of a
  // longer control period is added loop delay: the crossover frequency the
  // gains were tuned for no longer fits inside the slower sampling, so the
  // loop must be slowed down proportionally. Never scale gains UP for fast
  // ticks -- the tuning point is the ceiling.
  const float gain_scale = std::min(1.0f, DT * inv_dt);

  // Longitudinal PID calculation. The speed loop is first-order and far less
  // delay-sensitive than the lateral loop, so only the integral/derivative
  // discretization uses the measured period; the proportional gain stays.
  const float expr_v =
      longitudinal_parameters[0] * present_state.velocity_deviation +
      longitudinal_parameters[1] * (present_state.velocity_deviation + previous_state.velocity_deviation) * dt +
      longitudinal_parameters[2] * (present_state.velocity_deviation - previous_state.velocity_deviation) * inv_dt;

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
  // 180 deg/s (MAX_DEVIATION_RATE in normalised units), so rates above that
  // can only come from the target point jumping discontinuously (lane change
  // start, buffer reseed). An unclamped derivative turns such jumps into a
  // one-tick full-lock steer spike that swings the vehicle across the lane.
  const float max_deviation_delta = MAX_DEVIATION_RATE * dt;
  float deviation_delta = present_state.angular_deviation - previous_state.angular_deviation;
  deviation_delta = std::max(-max_deviation_delta, std::min(deviation_delta, max_deviation_delta));
  float steer = gain_scale * (
      lateral_parameters[0] * present_state.angular_deviation +
      lateral_parameters[1] * (present_state.angular_deviation + previous_state.angular_deviation) * dt +
      lateral_parameters[2] * deviation_delta * inv_dt);

  // Steering slew limit, applied as a rate so the physical steering speed is
  // independent of the tick rate (a fixed per-tick step would triple the
  // allowed rate at 60 fps and strangle it at 7 fps).
  const float max_steering_diff = MAX_STEERING_RATE * dt;
  steer = std::max(previous_state.steer - max_steering_diff, std::min(steer, previous_state.steer + max_steering_diff));
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
