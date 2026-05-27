// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/rpc/VehicleControl.h"

#include <deque>

namespace carla {

namespace client {
  class Vehicle;
  class Waypoint;
} // namespace client

namespace agents {
namespace navigation {

  /// Longitudinal PID controller. Mirrors
  /// `agents.navigation.controller.PIDLongitudinalController`.
  class PIDLongitudinalController {
  public:
    PIDLongitudinalController(
        SharedPtr<client::Vehicle> vehicle,
        float K_P = 1.0f,
        float K_I = 0.0f,
        float K_D = 0.0f,
        float dt = 0.03f);

    /// Returns the throttle/brake command (clamped to [-1, 1]).
    float RunStep(float target_speed_kmh, bool debug = false);

    void ChangeParameters(float K_P, float K_I, float K_D, float dt);

  private:
    float PidControl(float target_speed_kmh, float current_speed_kmh);

    SharedPtr<client::Vehicle> _vehicle;
    float _k_p;
    float _k_i;
    float _k_d;
    float _dt;
    std::deque<float> _error_buffer;   // mirrors deque(maxlen=10)
  };

  /// Lateral PID controller. Mirrors
  /// `agents.navigation.controller.PIDLateralController`.
  class PIDLateralController {
  public:
    PIDLateralController(
        SharedPtr<client::Vehicle> vehicle,
        float offset = 0.0f,
        float K_P = 1.0f,
        float K_I = 0.0f,
        float K_D = 0.0f,
        float dt = 0.03f);

    /// Returns a steering command in the range [-1, 1].
    float RunStep(SharedPtr<client::Waypoint> waypoint);

    void SetOffset(float offset);
    void ChangeParameters(float K_P, float K_I, float K_D, float dt);

  private:
    SharedPtr<client::Vehicle> _vehicle;
    float _k_p;
    float _k_i;
    float _k_d;
    float _dt;
    float _offset;
    std::deque<float> _e_buffer;       // mirrors deque(maxlen=10)
  };

  /// Combined longitudinal + lateral PID controller. Mirrors
  /// `agents.navigation.controller.VehiclePIDController`.
  class VehiclePIDController {
  public:
    struct LateralArgs {
      float K_P = 1.95f;
      float K_I = 0.05f;
      float K_D = 0.20f;
      float dt  = 0.05f;
    };

    struct LongitudinalArgs {
      float K_P = 1.50f;
      float K_I = 0.05f;
      float K_D = 0.20f;
      float dt  = 0.05f;
    };

    VehiclePIDController(
        SharedPtr<client::Vehicle> vehicle,
        const LateralArgs &lateral,
        const LongitudinalArgs &longitudinal,
        float offset = 0.0f,
        float max_throttle = 0.75f,
        float max_brake = 0.30f,
        float max_steering = 0.80f);

    /// Drives towards @a wp at @a target_speed_kmh and returns the resulting
    /// VehicleControl. Steering rate is rate-limited the same way as the
    /// Python original (delta clamped to +-0.1 per step).
    rpc::VehicleControl RunStep(
        float target_speed_kmh,
        SharedPtr<client::Waypoint> wp);

    void ChangeLongitudinalPID(const LongitudinalArgs &args);
    void ChangeLateralPID(const LateralArgs &args);
    void SetOffset(float offset);

  private:
    SharedPtr<client::Vehicle>  _vehicle;
    float _max_throttle;
    float _max_brake;
    float _max_steering;
    float _past_steering;
    PIDLongitudinalController _lon_controller;
    PIDLateralController      _lat_controller;
  };

} // namespace navigation
} // namespace agents
} // namespace carla
