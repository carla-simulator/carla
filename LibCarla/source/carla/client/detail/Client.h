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
#include "carla/client/detail/Episode.h"
#include "carla/geom/Transform.h"
#include "carla/profiler/LifetimeProfiled.h"

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
  class Episode;
}
namespace rpc { class VehicleControl; }
namespace sensor { class SensorData; }
namespace streaming { class Token; }
}

namespace carla {
namespace client {
namespace detail {

  /// @todo Make sure this class is really thread-safe.
  class Client
    : public EnableSharedFromThis<Client>,
      private profiler::LifetimeProfiled,
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

    void UnSubscribeFromStream(const streaming::Token &token);

    geom::Location GetActorLocation(const Actor &actor);

    geom::Transform GetActorTransform(const Actor &actor);

    void SetActorLocation(Actor &actor, const geom::Location &location);

    void SetActorTransform(Actor &actor, const geom::Transform &transform);

    void SetVehicleAutopilot(Vehicle &vehicle, bool enabled = true);

    void ApplyControlToVehicle(Vehicle &vehicle, const rpc::VehicleControl &control);

    GarbageCollectionPolicy GetGarbageCollectionPolicy() const {
      return _gc_policy;
    }

    size_t GetCurrentEpisodeId() const {
      return _episode_id;
    }

    Episode GetCurrentEpisode() {
      return shared_from_this();
    }

  private:

    class Pimpl;
    const std::unique_ptr<Pimpl> _pimpl;

    const GarbageCollectionPolicy _gc_policy;

    // At this point the id won't change because we cannot yet restart the
    // episode from the client.
    const size_t _episode_id = 0u;
  };

} // namespace detail
} // namespace client
} // namespace carla
