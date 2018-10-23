// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Client.h"

#include "carla/Version.h"
#include "carla/client/Actor.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Vehicle.h"
#include "carla/client/World.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/client/detail/Episode.h"
#include "carla/rpc/Client.h"
#include "carla/sensor/Deserializer.h"
#include "carla/streaming/Client.h"

#include <thread>

namespace carla {
namespace client {
namespace detail {

  // ===========================================================================
  // -- Client::Pimpl ----------------------------------------------------------
  // ===========================================================================

  class Client::Pimpl {
  public:

    Pimpl(const std::string &host, uint16_t port, size_t worker_threads)
      : rpc_client(host, port),
        streaming_client(host) {
      streaming_client.AsyncRun(
          worker_threads > 0u ? worker_threads : std::thread::hardware_concurrency());
    }

    template <typename T, typename... Args>
    T CallAndWait(const std::string &function, Args &&... args) {
      return rpc_client.call(function, std::forward<Args>(args)...).template as<T>();
    }

    template <typename... Args>
    void AsyncCall(const std::string &function, Args &&... args) {
      // Discard returned future.
      rpc_client.async_call(function, std::forward<Args>(args)...);
    }

    rpc::Client rpc_client;

    streaming::Client streaming_client;
  };

  // ===========================================================================
  // -- Client -----------------------------------------------------------------
  // ===========================================================================

  Client::Client(
      const std::string &host,
      const uint16_t port,
      const size_t worker_threads,
      const bool enable_garbage_collection)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER("carla::client::detail::Client"),
      _pimpl(std::make_unique<Pimpl>(host, port, worker_threads)),
      _gc_policy(enable_garbage_collection ?
        GarbageCollectionPolicy::Enabled : GarbageCollectionPolicy::Disabled) {}

  Client::~Client() = default;

  void Client::SetTimeout(time_duration timeout) {
    _pimpl->rpc_client.set_timeout(timeout.milliseconds());
  }

  // Keep this function in the cpp, to avoid recompiling everything on each
  // commit.
  std::string Client::GetClientVersion() {
    return ::carla::version();
  }

  std::string Client::GetServerVersion() {
    return _pimpl->CallAndWait<std::string>("version");
  }

  bool Client::Ping() {
    return _pimpl->CallAndWait<bool>("ping");
  }

  SharedPtr<BlueprintLibrary> Client::GetBlueprintLibrary() {
    using return_type = std::vector<carla::rpc::ActorDefinition>;
    auto result = _pimpl->CallAndWait<return_type>("get_actor_definitions");
    return MakeShared<BlueprintLibrary>(result);
  }

  SharedPtr<Actor> Client::GetSpectator() {
    auto spectator = _pimpl->CallAndWait<carla::rpc::Actor>("get_spectator");
    return ActorFactory::MakeActor(
        GetCurrentEpisode(),
        spectator,
        GarbageCollectionPolicy::Disabled);
  }

  SharedPtr<Actor> Client::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent,
      GarbageCollectionPolicy gc) {
    rpc::Actor actor;
    if (parent != nullptr) {
      actor = _pimpl->CallAndWait<rpc::Actor>("spawn_actor_with_parent",
          transform,
          blueprint.MakeActorDescription(),
          parent->Serialize());
    } else {
      actor = _pimpl->CallAndWait<rpc::Actor>("spawn_actor",
          transform,
          blueprint.MakeActorDescription());
    }
    return ActorFactory::MakeActor(GetCurrentEpisode(), actor, gc);
  }

  bool Client::DestroyActor(Actor &actor) {
    auto result = _pimpl->CallAndWait<bool>("destroy_actor", actor.Serialize());
    // Remove it's persistent state so it cannot access the client anymore.
    actor.GetEpisode().ClearState();
    log_debug(actor.GetDisplayId(), "destroyed.");
    return result;
  }

  void Client::SubscribeToStream(
      const streaming::Token &token,
      std::function<void(SharedPtr<sensor::SensorData>)> callback) {
    _pimpl->streaming_client.Subscribe(token, [callback](auto buffer) {
      callback(sensor::Deserializer::Deserialize(std::move(buffer)));
    });
  }

  void Client::UnSubscribeFromStream(const streaming::Token &token) {
    _pimpl->streaming_client.UnSubscribe(token);
  }

  geom::Location Client::GetActorLocation(const Actor &actor) {
    return _pimpl->CallAndWait<geom::Location>("get_actor_location", actor.Serialize());
  }

  geom::Transform Client::GetActorTransform(const Actor &actor) {
    return _pimpl->CallAndWait<geom::Transform>("get_actor_transform", actor.Serialize());
  }

  void Client::SetActorLocation(Actor &actor, const geom::Location &location) {
    _pimpl->AsyncCall("set_actor_location", actor.Serialize(), location);
  }

  void Client::SetActorTransform(Actor &actor, const geom::Transform &transform) {
    _pimpl->AsyncCall("set_actor_transform", actor.Serialize(), transform);
  }

  void Client::SetVehicleAutopilot(
      Vehicle &vehicle,
      const bool enabled) {
    _pimpl->AsyncCall("set_actor_autopilot", vehicle.Serialize(), enabled);
  }

  void Client::ApplyControlToVehicle(
      Vehicle &vehicle,
      const rpc::VehicleControl &control) {
    _pimpl->AsyncCall("apply_control_to_actor", vehicle.Serialize(), control);
  }

} // namespace detail
} // namespace client
} // namespace carla
