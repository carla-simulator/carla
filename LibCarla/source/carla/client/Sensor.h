// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Actor.h"

#include <functional>

namespace carla {
namespace sensor { class SensorData; }
namespace client {

  class Sensor : public Actor {
  public:

    using CallbackFunctionType = std::function<void(SharedPtr<sensor::SensorData>)>;

    using Actor::Actor;

    /// Register a @a callback to be executed each time a new measurement is
    /// received.
    virtual void Listen(CallbackFunctionType callback) = 0;

    /// Stop listening for new measurements.
    virtual void Stop() = 0;

    /// Return whether this Sensor instance is currently listening to new data.
    virtual bool IsListening() const = 0;
  };

} // namespace client
} // namespace carla
