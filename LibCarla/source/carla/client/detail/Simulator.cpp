// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/Simulator.h"

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/RecurrentSharedFuture.h"
#include "carla/client/BlueprintLibrary.h"
#include "carla/client/Map.h"
#include "carla/client/Sensor.h"
#include "carla/client/TimeoutException.h"
#include "carla/client/WalkerAIController.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/sensor/Deserializer.h"

#include <exception>
#include <thread>

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

  static void SynchronizeFrame(uint64_t frame, const Episode &episode) {
    while (frame > episode.GetState()->GetTimestamp().frame) {
      std::this_thread::yield();
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
  // -- Load a new episode -----------------------------------------------------
  // ===========================================================================

  EpisodeProxy Simulator::LoadEpisode(std::string map_name) {
    const auto id = GetCurrentEpisode().GetId();
    _client.LoadEpisode(std::move(map_name));
    size_t number_of_attempts = _client.GetTimeout().milliseconds() / 10u;
    for (auto i = 0u; i < number_of_attempts; ++i) {
      using namespace std::literals::chrono_literals;
      _episode->WaitForState(10ms);
      auto episode = GetCurrentEpisode();
      if (episode.GetId() != id) {
        return episode;
      }
    }
    throw_exception(std::runtime_error("failed to connect to newly created map"));
  }

  // ===========================================================================
  // -- Access to current episode ----------------------------------------------
  // ===========================================================================

  EpisodeProxy Simulator::GetCurrentEpisode() {
    if (_episode == nullptr) {
      ValidateVersions(_client);
      _episode = std::make_shared<Episode>(_client);
      _episode->Listen();
      if (!GetEpisodeSettings().synchronous_mode) {
        WaitForTick(_client.GetTimeout());
      }
    }
    return EpisodeProxy{shared_from_this()};
  }

  SharedPtr<Map> Simulator::GetCurrentMap() {
    return MakeShared<Map>(_client.GetMapInfo());
  }

  // ===========================================================================
  // -- Tick -------------------------------------------------------------------
  // ===========================================================================

  WorldSnapshot Simulator::WaitForTick(time_duration timeout) {
    DEBUG_ASSERT(_episode != nullptr);
    auto result = _episode->WaitForState(timeout);
    if (!result.has_value()) {
      throw_exception(TimeoutException(_client.GetEndpoint(), timeout));
    }
    return *result;
  }

  uint64_t Simulator::Tick() {
    DEBUG_ASSERT(_episode != nullptr);
    const auto frame = _client.SendTickCue();
    SynchronizeFrame(frame, *_episode);
    RELEASE_ASSERT(frame == _episode->GetState()->GetTimestamp().frame);
    return frame;
  }

  // ===========================================================================
  // -- Access to global objects in the episode --------------------------------
  // ===========================================================================

  SharedPtr<BlueprintLibrary> Simulator::GetBlueprintLibrary() {
    auto defs = _client.GetActorDefinitions();
    return MakeShared<BlueprintLibrary>(std::move(defs));
  }

  SharedPtr<Actor> Simulator::GetSpectator() {
    return MakeActor(_client.GetSpectator());
  }

  uint64_t Simulator::SetEpisodeSettings(const rpc::EpisodeSettings &settings) {
    if (settings.synchronous_mode && !settings.fixed_delta_seconds) {
      log_warning(
          "synchronous mode enabled with variable delta seconds. It is highly "
          "recommended to set 'fixed_delta_seconds' when running on synchronous mode.");
    }
    const auto frame = _client.SetEpisodeSettings(settings);
    SynchronizeFrame(frame, *_episode);
    return frame;
  }

  // ===========================================================================
  // -- AI ---------------------------------------------------------------------
  // ===========================================================================

  void Simulator::RegisterAIController(const WalkerAIController &controller) {
    auto walker = controller.GetParent();
    if (walker == nullptr) {
      throw_exception(std::runtime_error(controller.GetDisplayId() + ": not attached to walker"));
      return;
    }
    DEBUG_ASSERT(_episode != nullptr);
    auto navigation = _episode->CreateNavigationIfMissing();
    DEBUG_ASSERT(navigation != nullptr);
    navigation->RegisterWalker(walker->GetId(), controller.GetId());
  }

  void Simulator::UnregisterAIController(const WalkerAIController &controller) {
    auto walker = controller.GetParent();
    if (walker == nullptr) {
      throw_exception(std::runtime_error(controller.GetDisplayId() + ": not attached to walker"));
      return;
    }
    DEBUG_ASSERT(_episode != nullptr);
    auto navigation = _episode->CreateNavigationIfMissing();
    DEBUG_ASSERT(navigation != nullptr);
    navigation->UnregisterWalker(walker->GetId(), controller.GetId());
  }

  boost::optional<geom::Location> Simulator::GetRandomLocationFromNavigation() {
    DEBUG_ASSERT(_episode != nullptr);
    auto navigation = _episode->CreateNavigationIfMissing();
    DEBUG_ASSERT(navigation != nullptr);
    return navigation->GetRandomLocation();
  }

  // ===========================================================================
  // -- General operations with actors -----------------------------------------
  // ===========================================================================

  SharedPtr<Actor> Simulator::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent,
      rpc::AttachmentType attachment_type,
      GarbageCollectionPolicy gc) {
    rpc::Actor actor;
    if (parent != nullptr) {
      actor = _client.SpawnActorWithParent(
          blueprint.MakeActorDescription(),
          transform,
          parent->GetId(),
          attachment_type);
    } else {
      actor = _client.SpawnActor(
          blueprint.MakeActorDescription(),
          transform);
    }
    DEBUG_ASSERT(_episode != nullptr);
    _episode->RegisterActor(actor);
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
    bool success = true;
    success = _client.DestroyActor(actor.GetId());
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
