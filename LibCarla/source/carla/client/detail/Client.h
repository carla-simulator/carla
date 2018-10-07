// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/client/GarbageCollectionPolicy.h"
#include "carla/client/World.h"
#include "carla/geom/Transform.h"

#include <functional>
#include <memory>
#include <string>

// Forward declarations.
namespace carla {
namespace client {
  class Actor;
  class ActorBlueprint;
  class BlueprintLibrary;
  class Vehicle;
  class World;
}
namespace rpc { class VehicleControl; }
namespace sensor { class SensorData; }
namespace streaming { class Token; }
}

namespace carla {
namespace client {
namespace detail {

  class Client
    : public EnableSharedFromThis<Client>,
      private NonCopyable {
  public:

    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u,
        bool enable_garbage_collection = false);

    ~Client();

    void SetTimeout(time_duration timeout);

    std::string GetClientVersion();

    std::string GetServerVersion();

    bool Ping();

    World GetWorld() {
      return shared_from_this();
    }

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    SharedPtr<Actor> GetSpectator();

    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const geom::Transform &transform,
        Actor *parent,
        GarbageCollectionPolicy gc = GarbageCollectionPolicy::Inherit);

    bool DestroyActor(Actor &actor);

    void SubscribeToStream(
        const streaming::Token &token,
        std::function<void(SharedPtr<sensor::SensorData>)> callback);

    geom::Location GetActorLocation(const Actor &actor);

    geom::Transform GetActorTransform(const Actor &actor);

    void SetActorLocation(Actor &actor, const geom::Location &location);

    void SetActorTransform(Actor &actor, const geom::Transform &transform);

    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true);

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control);

    GarbageCollectionPolicy GetGarbageCollectionPolicy() const {
      return _gc_policy;
    }

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;

    const GarbageCollectionPolicy _gc_policy;
  };

} // namespace detail
} // namespace client
} // namespace carla
