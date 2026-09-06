// Copyright (c) 2018-2020 CVC.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/agents/navigation/BasicAgent.h"
#include "carla/rpc/VehicleControl.h"

#include <limits>

namespace carla {

namespace client {
  class Map;
  class Sensor;
  class Vehicle;
} // namespace client

namespace agents {
namespace navigation {

  class GlobalRoutePlanner;

  /// C++ port of agents.navigation.constant_velocity_agent.ConstantVelocityAgent.
  ///
  /// Drives the ego at a fixed velocity, using BasicAgent's planner just to
  /// keep the wheels rotating. A collision sensor is attached on construction
  /// and a collision flips the agent into a "stopped" state.
  class ConstantVelocityAgent : public BasicAgent {
  public:

    struct ConstantOptions : Options {
      bool  use_basic_behavior = false;
      float restart_time       = std::numeric_limits<float>::infinity();
    };

    /// Out-of-line default factory; see LocalPlanner::DefaultOptions().
    static ConstantOptions DefaultConstantOptions();

    ConstantVelocityAgent(SharedPtr<client::Vehicle> vehicle,
                          float target_speed_kmh = 20.0f,
                          const ConstantOptions &opts = DefaultConstantOptions(),
                          SharedPtr<client::Map> map = nullptr,
                          SharedPtr<GlobalRoutePlanner> grp = nullptr);

    ~ConstantVelocityAgent() override;

    /// Hides BasicAgent::SetTargetSpeed; converts km/h to m/s for the
    /// internal `_target_speed_ms` and forwards to the planner.
    void SetTargetSpeed(float speed_kmh);

    /// Disables constant velocity mode and stamps the stop time. The agent
    /// will sit idle until `restart_time` has elapsed (then auto-restart) or
    /// `RestartConstantVelocity` is called manually.
    void StopConstantVelocity();

    /// Re-enable constant velocity at the current `_target_speed_ms`.
    void RestartConstantVelocity();

    rpc::VehicleControl RunStep() override;

    /// Tear down the collision sensor (called from the destructor too).
    void DestroySensor();

  private:

    void SetCollisionSensor();
    void SetConstantVelocity(float speed_ms);

    bool   _use_basic_behavior;
    float  _target_speed_ms;             // m/s — note BasicAgent's _target_speed is km/h
    float  _restart_time;
    bool   _is_constant_velocity_active;
    double _constant_velocity_stop_time;
    bool   _has_stop_timestamp;

    SharedPtr<client::Sensor> _collision_sensor;
  };

  inline ConstantVelocityAgent::ConstantOptions
  ConstantVelocityAgent::DefaultConstantOptions() {
    return ConstantOptions{};
  }

} // namespace navigation
} // namespace agents
} // namespace carla
