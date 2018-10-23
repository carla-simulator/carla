// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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

    explicit Sensor(ActorInitializer init) : Actor(std::move(init)) {}

    ~Sensor();

    using CallbackFunctionType = std::function<void(SharedPtr<sensor::SensorData>)>;

    /// Register a @a callback to be executed each time a new measurement is
    /// received.
    ///
    /// @warning Calling this function on a sensor that is already listening
    /// steals the data stream from the previously set callback. Note that
    /// several instances of Sensor (even in different processes) may point to
    /// the same sensor in the simulator.
    void Listen(CallbackFunctionType callback);

    /// Stop listening for new measurements.
    void Stop();

    /// Return whether this Sensor instance is currently listening to the
    /// associated sensor in the simulator.
    bool IsListening() const {
      return _is_listening;
    }

    /// @copydoc Actor::Destroy()
    ///
    /// Additionally stop listening.
    bool Destroy() override;

  private:

    bool _is_listening = false;
  };

} // namespace client
} // namespace carla
