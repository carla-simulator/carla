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

    void SetAutopilot(bool enabled = true);

    void ApplyControl(const Control &control);

    const Control &GetControl() const {
      return _control;
    }

  private:

    Control _control;
  };

} // namespace client
} // namespace carla
