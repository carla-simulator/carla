// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Simulator.h"

#include "carla/Logging.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Sensor.h"
#include "carla/client/detail/ActorFactory.h"

#include <exception>

using namespace std::string_literals;

namespace carla {
namespace client {
namespace detail {

  // ===========================================================================
  // -- Constructor ------------------------------------------------------------
  // ===========================================================================

  Simulator::Simulator(
      const std::string &host,
      const uint16_t port,
      const size_t worker_threads,
      const bool enable_garbage_collection)
    : LIBCARLA_INITIALIZE_LIFETIME_PROFILER("SimulatorClient("s + host + ":" + std::to_string(port) + ")"),
      _client(host, port, worker_threads),
      _episode(), /// @todo
      _gc_policy(enable_garbage_collection ?
        GarbageCollectionPolicy::Enabled : GarbageCollectionPolicy::Disabled) {}

  // ===========================================================================
  // -- Access to global objects in the episode --------------------------------
  // ===========================================================================

  SharedPtr<BlueprintLibrary> Simulator::GetBlueprintLibrary() {
    return MakeShared<BlueprintLibrary>(_client.GetActorDefinitions());
  }

  SharedPtr<Actor> Simulator::GetSpectator() {
    return ActorFactory::MakeActor(
        GetCurrentEpisode(),
        _client.GetSpectator(),
        GarbageCollectionPolicy::Disabled);
  }

  // ===========================================================================
  // -- General operations with actors -----------------------------------------
  // ===========================================================================

  SharedPtr<Actor> Simulator::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent,
      GarbageCollectionPolicy gc) {
    rpc::Actor actor;
    if (parent != nullptr) {
      actor = _client.SpawnActorWithParent(
          blueprint.MakeActorDescription(),
          transform,
          parent->Serialize());
    } else {
      actor = _client.SpawnActor(
          blueprint.MakeActorDescription(),
          transform);
    }
    const auto gca = (gc == GarbageCollectionPolicy::Inherit ? _gc_policy : gc);
    auto result = ActorFactory::MakeActor(GetCurrentEpisode(), actor, gca);
    log_debug(
        result->GetDisplayId(),
        "created",
        gca == GarbageCollectionPolicy::Enabled ? "with" : "without",
        "garbage collection");
    return result;
  }

  bool Simulator::DestroyActor(Actor &actor) {
    auto success = _client.DestroyActor(actor.Serialize());
    if (success) {
      // Remove it's persistent state so it cannot access the client anymore.
      actor.GetEpisode().ClearState();
      log_debug(actor.GetDisplayId(), "destroyed.");
    } else {
      log_debug("failed to destroy", actor.GetDisplayId());
    }
    return success;
  }

  geom::Location Simulator::GetActorLocation(const Actor &) {
    throw std::runtime_error("GetActorLocation() not implemented!");
  }

  geom::Transform Simulator::GetActorTransform(const Actor &) {
    throw std::runtime_error("GetActorTransform() not implemented!");
  }

  // ===========================================================================
  // -- Operations with sensors ------------------------------------------------
  // ===========================================================================

  void Simulator::SubscribeToSensor(
      const Sensor &sensor,
      std::function<void(SharedPtr<sensor::SensorData>)> callback) {
    _client.SubscribeToStream(
        sensor.GetActorDescription().GetStreamToken(),
        std::move(callback));
  }

  void Simulator::UnSubscribeFromSensor(const Sensor &sensor) {
    _client.UnSubscribeFromStream(sensor.GetActorDescription().GetStreamToken());
  }

} // namespace detail
} // namespace client
} // namespace carla
