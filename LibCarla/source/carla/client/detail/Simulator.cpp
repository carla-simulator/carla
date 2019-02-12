// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Simulator.h"

#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/RecurrentSharedFuture.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/Sensor.h"
#include "carla/client/TimeoutException.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/sensor/Deserializer.h"

#include <exception>

using namespace std::string_literals;

namespace carla {
namespace client {
namespace detail {

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  static void ValidateVersions(Client &client) {
    const auto vc = client.GetClientVersion();
    const auto vs = client.GetServerVersion();
    if (vc != vs) {
      log_warning(
          "Version mismatch detected: You are trying to connect to a simulator",
          "that might be incompatible with this API");
      log_warning("Client API version     =", vc);
      log_warning("Simulator API version  =", vs);
    }
  }

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
      _gc_policy(enable_garbage_collection ?
        GarbageCollectionPolicy::Enabled : GarbageCollectionPolicy::Disabled) {}

  // ===========================================================================
  // -- Access to current episode ----------------------------------------------
  // ===========================================================================

  EpisodeProxy Simulator::GetCurrentEpisode() {
    if (_episode == nullptr) {
      ValidateVersions(_client);
      _episode = std::make_shared<Episode>(_client);
      _episode->Listen();
      WaitForTick(_client.GetTimeout());
    }
    return EpisodeProxy{shared_from_this()};
  }

  SharedPtr<Map> Simulator::GetCurrentMap() {
    return MakeShared<Map>(_client.GetMapInfo());
  }

  // ===========================================================================
  // -- Tick -------------------------------------------------------------------
  // ===========================================================================

  Timestamp Simulator::WaitForTick(time_duration timeout) {
    DEBUG_ASSERT(_episode != nullptr);
    auto result = _episode->WaitForState(timeout);
    if (!result.has_value()) {
      throw_exception(TimeoutException(_client.GetEndpoint(), timeout));
    }
    return *result;
  }

  // ===========================================================================
  // -- Access to global objects in the episode --------------------------------
  // ===========================================================================

  SharedPtr<BlueprintLibrary> Simulator::GetBlueprintLibrary() {
    auto defs = _client.GetActorDefinitions();
    { /// @todo
      rpc::ActorDefinition def;
      def.id = "sensor.other.lane_detector";
      def.tags = "sensor,other,lane_detector";
      defs.emplace_back(def);
    }
    return MakeShared<BlueprintLibrary>(std::move(defs));
  }

  SharedPtr<Actor> Simulator::GetSpectator() {
    return ActorFactory::MakeActor(
        GetCurrentEpisode(),
        _client.GetSpectator(),
        nullptr,
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
    if (blueprint.GetId() == "sensor.other.lane_detector") { /// @todo
      actor.description = blueprint.MakeActorDescription();
    } else if (parent != nullptr) {
      actor = _client.SpawnActorWithParent(
          blueprint.MakeActorDescription(),
          transform,
          parent->Serialize());
    } else {
      actor = _client.SpawnActor(
          blueprint.MakeActorDescription(),
          transform);
    }
    DEBUG_ASSERT(_episode != nullptr);
    _episode->RegisterActor(actor);
    const auto gca = (gc == GarbageCollectionPolicy::Inherit ? _gc_policy : gc);
    auto parent_ptr = parent != nullptr ? parent->shared_from_this() : SharedPtr<Actor>();
    auto result = ActorFactory::MakeActor(GetCurrentEpisode(), actor, parent_ptr, gca);
    log_debug(
        result->GetDisplayId(),
        "created",
        gca == GarbageCollectionPolicy::Enabled ? "with" : "without",
        "garbage collection");
    return result;
  }

  bool Simulator::DestroyActor(Actor &actor) {
    bool success = true;
    if (actor.GetTypeId() != "sensor.other.lane_detector") { /// @todo
      success = _client.DestroyActor(actor.Serialize());
    }
    if (success) {
      // Remove it's persistent state so it cannot access the client anymore.
      actor.GetEpisode().Clear();
      log_debug(actor.GetDisplayId(), "destroyed.");
    } else {
      log_debug("failed to destroy", actor.GetDisplayId());
    }
    return success;
  }

  // ===========================================================================
  // -- Operations with sensors ------------------------------------------------
  // ===========================================================================

  void Simulator::SubscribeToSensor(
      const Sensor &sensor,
      std::function<void(SharedPtr<sensor::SensorData>)> callback) {
    DEBUG_ASSERT(_episode != nullptr);
    _client.SubscribeToStream(
        sensor.GetActorDescription().GetStreamToken(),
        [cb=std::move(callback), ep=WeakEpisodeProxy{shared_from_this()}](auto buffer) {
          auto data = sensor::Deserializer::Deserialize(std::move(buffer));
          data->_episode = ep.TryLock();
          cb(std::move(data));
        });
  }

  void Simulator::UnSubscribeFromSensor(const Sensor &sensor) {
    _client.UnSubscribeFromStream(sensor.GetActorDescription().GetStreamToken());
  }

} // namespace detail
} // namespace client
} // namespace carla
