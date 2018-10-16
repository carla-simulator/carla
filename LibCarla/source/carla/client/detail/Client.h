// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/geom/Transform.h"
#include "carla/rpc/Actor.h"
#include "carla/rpc/ActorDefinition.h"
#include "carla/rpc/WeatherParameters.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// Forward declarations.
namespace carla {
namespace rpc {
  class ActorDescription;
  class VehicleControl;
}
namespace sensor { class SensorData; }
namespace streaming { class Token; }
}

namespace carla {
namespace client {
namespace detail {

  /// Provides communication with the rpc and streaming servers of a CARLA
  /// simulator.
  ///
  /// @todo Make sure this class is really thread-safe.
  class Client : private NonCopyable {
  public:

    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    ~Client();

    void SetTimeout(time_duration timeout);

    std::string GetServerVersion();

    bool Ping();

    std::vector<rpc::ActorDefinition> GetActorDefinitions();

    rpc::Actor GetSpectator();

    rpc::WeatherParameters GetWeatherParameters();

    void SetWeatherParameters(const rpc::WeatherParameters &weather);

    rpc::Actor SpawnActor(
        const rpc::ActorDescription &description,
        const geom::Transform &transform);

    rpc::Actor SpawnActorWithParent(
        const rpc::ActorDescription &description,
        const geom::Transform &transform,
        const rpc::Actor &parent);

    bool DestroyActor(const rpc::Actor &actor);

    void SetActorLocation(
        const rpc::Actor &actor,
        const geom::Location &location);

    void SetActorTransform(
        const rpc::Actor &actor,
        const geom::Transform &transform);

    void SetActorAutopilot(
        const rpc::Actor &vehicle,
        bool enabled);

    void ApplyControlToActor(
        const rpc::Actor &vehicle,
        const rpc::VehicleControl &control);

    void SubscribeToStream(
        const streaming::Token &token,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    void UnSubscribeFromStream(const streaming::Token &token);

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;
  };

} // namespace detail
} // namespace client
} // namespace carla
