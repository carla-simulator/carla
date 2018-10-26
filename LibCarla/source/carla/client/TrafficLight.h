// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"

namespace carla {
namespace client {

  enum class TrafficLightState {
    Unknown,
    Red,
    Yellow,
    Green
  };

  class TrafficLight : public Actor {
  public:

    explicit TrafficLight(ActorInitializer init) : Actor(std::move(init)) {}

    /// Return the current state of the traffic light.
    ///
    /// @note This function does not call the simulator, it returns the
    /// traffic light state received in the last tick.
    TrafficLightState GetState();
  };

} // namespace client
} // namespace carla
