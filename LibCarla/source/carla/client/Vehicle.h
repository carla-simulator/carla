// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/client/Control.h"

namespace carla {
namespace client {

  class Vehicle : public Actor {
  public:

    void ApplyControl(const VehicleControl &control) {
      GetWorld()->GetClient().ApplyControlToActor(*this, control);
    }

    void SetAutopilot(bool enabled = true) {
      GetWorld()->GetClient().SetActorAutopilot(*this, enabled);
    }

  private:

    friend class Client;

    template <typename ... Args>
    Vehicle(Args && ... args) : Actor(std::forward<Args>(args)...) {}
  };

} // namespace client
} // namespace carla
