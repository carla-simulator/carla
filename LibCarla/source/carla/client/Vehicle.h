// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/VehicleControl.h"

namespace carla {
namespace client {

  class Vehicle : public Actor {
  public:

    using Control = rpc::VehicleControl;

    explicit Vehicle(ActorInitializer init) : Actor(std::move(init)) {}

    /// Switch on/off this vehicle's autopilot.
    void SetAutopilot(bool enabled = true);

    /// Apply @a control to this vehicle.
    void ApplyControl(const Control &control);

    /// Return the control last applied to this vehicle.
    ///
    /// @note This function does not call the simulator, it returns the Control
    /// received in the last tick.
    Control GetVehicleControl() const;

  private:

    Control _control;
  };

} // namespace client
} // namespace carla
