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

    void Listen(CallbackFunctionType callback);

    void Stop();

    bool IsListening() const {
      return _is_listening;
    }

    void Destroy() override;

  private:

    bool _is_listening = false;
  };

} // namespace client
} // namespace carla
