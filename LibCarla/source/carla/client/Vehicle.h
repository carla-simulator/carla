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
    /// @warning This function only takes into account the control applied to
    /// this instance of Vehicle. Note that several instances of Vehicle (even
    /// in different processes) may point to the same vehicle in the simulator.
    const Control &GetControl() const {
      return _control;
    }

  private:

    Control _control;
  };

} // namespace client
} // namespace carla
