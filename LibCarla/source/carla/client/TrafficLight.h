// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"
#include "carla/rpc/TrafficLightState.h"

namespace carla {
namespace client {

  class TrafficLight : public Actor {
  public:

    explicit TrafficLight(ActorInitializer init) : Actor(std::move(init)) {}

    /// Return the current state of the traffic light.
    ///
    /// @note These functions do not call the simulator, they return the
    /// data received in the last tick.
    void SetState(rpc::TrafficLightState state);

    rpc::TrafficLightState GetState() const;

    void SetGreenTime(float green_time);

    float GetGreenTime() const;

    void SetYellowTime(float yellow_time);

    float GetYellowTime() const;

    void SetRedTime(float red_time);

    float GetRedTime() const;

    float GetElapsedTime() const;

    void Freeze(bool freeze);

    bool IsFrozen() const;

  };

} // namespace client
} // namespace carla
