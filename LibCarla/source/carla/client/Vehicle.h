// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/VehicleControl.h"
#include "carla/rpc/VehiclePhysicsControl.h"
#include "carla/rpc/TrafficLightState.h"

namespace carla {
namespace client {
  class TrafficLight;

  class Vehicle : public Actor {
  public:

    using Control = rpc::VehicleControl;
    using PhysicsControl = rpc::VehiclePhysicsControl;

    explicit Vehicle(ActorInitializer init) : Actor(std::move(init)) {}

    /// Switch on/off this vehicle's autopilot.
    void SetAutopilot(bool enabled = true);

    /// Apply @a control to this vehicle.
    void ApplyControl(const Control &control);

    /// Apply physics control to this vehicle
    void ApplyPhysicsControl(const PhysicsControl &physics_control);

    /// Return the control last applied to this vehicle.
    ///
    /// @note The following functions do not call the simulator, they return the
    /// data
    /// received in the last tick.
    //////////////////////////////////////////////////////////////////////////////////
    Control GetControl() const;
    PhysicsControl GetPhysicsControl() const;

    float GetSpeedLimit() const;

    rpc::TrafficLightState GetTrafficLightState() const;

    bool IsAtTrafficLight();

    SharedPtr<TrafficLight> GetTrafficLight() const;


  private:

    Control _control;
  };

} // namespace client
} // namespace carla
