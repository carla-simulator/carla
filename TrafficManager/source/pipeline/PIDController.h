#pragma once

#include <cmath>
#include <vector>
#include <chrono>

namespace traffic_manager {

  struct ActuationSignal {
    float throttle;
    float brake;
    float steer;
  };

  typedef std::chrono::time_point<
      std::chrono::_V2::system_clock,
      std::chrono::nanoseconds
      > TimeInstance;

  struct StateEntry {
    float deviation;
    float velocity;
    TimeInstance time_instance;
    float deviation_integral;
    float velocity_integral;
  };

  class PIDController {

    /// This class calculates PID actuation signals to control the vehicle
    /// such that the vehicle maintains a target velocity and aligns its
    /// orientation along it's trajectory.

  public:

    PIDController();

    /// This method calculates the present state of the vehicle including
    /// the accumulated integral component of the PID system.
    StateEntry StateUpdate(
        StateEntry previous_state,
        float current_velocity,
        float target_velocity,
        float angular_deviation,
        TimeInstance current_time);

    /// This method calculates the actuation signals based on the resent state
    /// change of the vehicle to minimize PID error.
    ActuationSignal RunStep(
        StateEntry present_state,
        StateEntry previous_state,
        const std::vector<float> &longitudinal_parameters,
        const std::vector<float> &lateral_parameters) const;

  };

}
