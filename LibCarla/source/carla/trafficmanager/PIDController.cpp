// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/trafficmanager/PIDController.h"

#include <algorithm>

#define CLAMP(__v, __hi, __lo) (__v > __hi? __hi : (__v < __lo? __lo: __v))

namespace carla {
namespace traffic_manager {

namespace PIDControllerConstants {

  const float MAX_THROTTLE = 0.7f;
  const float MAX_BRAKE = 1.0f;
  const float VELOCITY_INTEGRAL_MAX = 5.0f;
  const float VELOCITY_INTEGRAL_MIN = -5.0f;
  // PID will be stable only over 20 FPS.
  const float dt = 1/20.0f;

} // namespace PIDControllerConstants

  using namespace PIDControllerConstants;

  PIDController::PIDController() {}

  // Initializing present state.
  StateEntry PIDController::StateUpdate(
      StateEntry previous_state,
      float current_velocity,
      float target_velocity,
      float angular_deviation,
      float distance,
      TimeInstance current_time) {

    traffic_manager::StateEntry current_state = {
      angular_deviation, distance,
      (current_velocity - target_velocity) / target_velocity,
      current_time,
      0.0f,
      0.0f,
      0.0f
    };

    // Calculating integrals.
    current_state.deviation_integral = angular_deviation * dt + previous_state.deviation_integral;
    current_state.distance_integral = distance * dt + previous_state.distance_integral;
    current_state.velocity_integral = dt * current_state.velocity + previous_state.velocity_integral;

    // Clamp velocity integral to avoid accumulating over-compensation
    // with time for vehicles that take a long time to reach target velocity.
    current_state.velocity_integral = CLAMP(current_state.velocity_integral,
                                            VELOCITY_INTEGRAL_MAX, VELOCITY_INTEGRAL_MIN);

    return current_state;
  }

  ActuationSignal PIDController::RunStep(
      StateEntry present_state,
      StateEntry previous_state,
      const std::vector<float> &longitudinal_parameters,
      const std::vector<float> &lateral_parameters) const {

    // Longitudinal PID calculation.
    const float expr_v =
        longitudinal_parameters[0] * present_state.velocity +
        longitudinal_parameters[1] * present_state.velocity_integral +
        longitudinal_parameters[2] * (present_state.velocity -
        previous_state.velocity) / dt;

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
        lateral_parameters[2] * (present_state.deviation -
        previous_state.deviation) / dt;

    steer = std::max(-0.8f, std::min(steer, 0.8f));

    return ActuationSignal{throttle, brake, steer};
  }

} // namespace traffic_manager
} // namespace carla
