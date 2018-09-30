// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/Time.h"
#include "carla/Version.h"
#include "carla/client/Control.h"
#include "carla/client/Memory.h"
#include "carla/client/Transform.h"
#include "carla/rpc/Client.h"
#include "carla/streaming/Client.h"

#include <string>

namespace carla {
namespace client {

  class Actor;
  class ActorBlueprint;
  class BlueprintLibrary;
  class World;

  class Client
    : public EnableSharedFromThis<Client>,
      private NonCopyable {
  public:

    /// Construct a carla client.
    ///
    /// @param host IP address of the host machine running the simulator.
    /// @param port TCP port to connect with the simulator.
    /// @param worker_threads number of asynchronous threads to use, or 0 to use
    ///        all available hardware concurrency.
    explicit Client(
        const std::string &host,
        uint16_t port,
        size_t worker_threads = 0u);

    void SetTimeout(time_duration timeout) {
      _client.set_timeout(timeout.milliseconds());
    }

    template <typename T, typename ... Args>
    T Call(const std::string &function, Args && ... args) {
      return _client.call(function, std::forward<Args>(args) ...).template as<T>();
    }

    template <typename Functor>
    void SubscribeToStream(const streaming::Token &token, Functor &&callback) {
      _streaming_client.Subscribe(token, std::forward<Functor>(callback));
    }

    std::string GetClientVersion() const {
      return ::carla::version();
    }

    std::string GetServerVersion() {
      return Call<std::string>("version");
    }

    bool Ping() {
      return Call<bool>("ping");
    }

    SharedPtr<World> GetWorld();

    SharedPtr<BlueprintLibrary> GetBlueprintLibrary();

    SharedPtr<Actor> GetSpectator();

    SharedPtr<Actor> SpawnActor(
        const ActorBlueprint &blueprint,
        const Transform &transform,
        Actor *parent = nullptr);

    void DestroyActor(Actor &actor);

    Location GetActorLocation(Actor &actor);

    Transform GetActorTransform(Actor &actor);

    bool SetActorLocation(Actor &actor, const Location &location);

    bool SetActorTransform(Actor &actor, const Transform &transform);

    void ApplyControlToActor(
        Actor &actor,
        const VehicleControl &control);

    void SetActorAutopilot(
        Actor &actor,
        bool enabled = true);

  private:

    carla::rpc::Client _client;

    carla::streaming::Client _streaming_client;

    SharedPtr<World> _active_world;
  };

} // namespace client
} // namespace carla
