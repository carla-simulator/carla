#include "PIDController.h"

namespace traffic_manager {
namespace PIDControllerConstants {
  const float MAX_THROTTLE = 0.8f;
  const float MAX_BRAKE = 1.0f;
}
  using namespace PIDControllerConstants;

  PIDController::PIDController() {}

  // Initializing present state
  StateEntry PIDController::StateUpdate(
      StateEntry previous_state,
      float current_velocity,
      float target_velocity,
      float angular_deviation,
      TimeInstance current_time) {
    traffic_manager::StateEntry current_state = {
      angular_deviation,
      (current_velocity - target_velocity) / target_velocity,
      current_time,
      0.0f,
      0.0f
    };

    // Calculating dt for 'D' and 'I' controller components
    chr::duration<double> duration = current_state.time_instance - previous_state.time_instance;
    auto dt = duration.count();

    // Calculating integrals
    current_state.deviation_integral = angular_deviation * dt + previous_state.deviation_integral;
    current_state.velocity_integral = dt * current_state.velocity + previous_state.velocity_integral;

    return current_state;
  }

  ActuationSignal PIDController::RunStep(
      StateEntry present_state,
      StateEntry previous_state,
      const std::vector<float> &longitudinal_parameters,
      const std::vector<float> &lateral_parameters) const {

    // Calculating dt for updating integral component
    chr::duration<double> duration = present_state.time_instance - previous_state.time_instance;
    auto dt = duration.count();

    // Longitudinal PID calculation
    float expr_v =
        longitudinal_parameters[0] * present_state.velocity +
        longitudinal_parameters[1] * present_state.velocity_integral +
        longitudinal_parameters[2] * (present_state.velocity - previous_state.velocity) / dt;

    float throttle;
    float brake;

    if (expr_v < 0.0f) {
      throttle = std::min(std::abs(expr_v), MAX_THROTTLE);
      brake = 0.0f;
    } else {
      throttle = 0.0f;
      brake = MAX_BRAKE;
    }

    // Lateral PID calculation
    float steer;
    steer =
        lateral_parameters[0] * present_state.deviation +
        lateral_parameters[1] * present_state.deviation_integral +
        lateral_parameters[2] * (present_state.deviation - previous_state.deviation) / dt;
    steer = std::max(-1.0f, std::min(steer, 1.0f));

    return ActuationSignal{throttle, brake, steer};
  }
}
