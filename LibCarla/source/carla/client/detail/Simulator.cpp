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
#include "carla/client/FileTransfer.h"
#include "carla/client/Map.h"
#include "carla/client/Sensor.h"
#include "carla/client/TimeoutException.h"
#include "carla/client/WalkerAIController.h"
#include "carla/client/detail/ActorFactory.h"
#include "carla/trafficmanager/TrafficManager.h"
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

  static bool SynchronizeFrame(uint64_t frame, const Episode &episode, time_duration timeout) {
    bool result = true;
    auto start = std::chrono::system_clock::now();
    while (frame > episode.GetState()->GetTimestamp().frame) {
      std::this_thread::yield();
      auto end = std::chrono::system_clock::now();
      auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
      if(timeout.to_chrono() < diff) {
        result = false;
        break;
      }
    }
    if(result) {
      carla::traffic_manager::TrafficManager::Tick();
    }
    
    return result;
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
      _light_manager(new LightManager()),
      _gc_policy(enable_garbage_collection ?
        GarbageCollectionPolicy::Enabled : GarbageCollectionPolicy::Disabled) {}

  // ===========================================================================
  // -- Load a new episode -----------------------------------------------------
  // ===========================================================================

  EpisodeProxy Simulator::LoadEpisode(std::string map_name, bool reset_settings, rpc::MapLayer map_layers) {
    const auto id = GetCurrentEpisode().GetId();
    _client.LoadEpisode(std::move(map_name), reset_settings, map_layers);

    // We are waiting 50ms for the server to reload the episode.
    // If in this time we have not detected a change of episode, we try again
    // 'number_of_attempts' times.
    // TODO This time is completly arbitrary so we need to improve
    // this pipeline to not depend in this time because this timeout
    // could result that the client resume the simulation in different
    // initial ticks when loading a map in syncronous mode.
    size_t number_of_attempts = _client.GetTimeout().milliseconds() / 50u;

    for (auto i = 0u; i < number_of_attempts; ++i) {
      using namespace std::literals::chrono_literals;
      if (_client.GetEpisodeSettings().synchronous_mode)
        _client.SendTickCue();

      _episode->WaitForState(50ms);
      auto episode = GetCurrentEpisode();

      if (episode.GetId() != id) {
        return episode;
      }
    }
    throw_exception(std::runtime_error("failed to connect to newly created map"));
  }

  EpisodeProxy Simulator::LoadOpenDriveEpisode(
      std::string opendrive,
      const rpc::OpendriveGenerationParameters & params, bool reset_settings) {
    // The "OpenDriveMap" is an ".umap" located in:
    // "carla/Unreal/CarlaUE4/Content/Carla/Maps/"
    // It will load the last sended OpenDRIVE by client's "LoadOpenDriveEpisode()"
    constexpr auto custom_opendrive_map = "OpenDriveMap";
    _client.CopyOpenDriveToServer(std::move(opendrive), params);
    return LoadEpisode(custom_opendrive_map, reset_settings);
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
      _light_manager->SetEpisode(WeakEpisodeProxy{shared_from_this()});
    }
    return EpisodeProxy{shared_from_this()};
  }

  bool Simulator::ShouldUpdateMap(rpc::MapInfo& map_info) {
    if (!_cached_map) {
      return true;
    }
    if (map_info.name != _cached_map->GetName() ||
        _open_drive_file.size() != _cached_map->GetOpenDrive().size()) {
      return true;
    }
    return false;
  }

  SharedPtr<Map> Simulator::GetCurrentMap() {
    DEBUG_ASSERT(_episode != nullptr);
    if (!_cached_map || _episode->HasMapChangedSinceLastCall()) {
      rpc::MapInfo map_info = _client.GetMapInfo();
      std::string map_name;
      std::string map_base_path;
      bool fill_base_string = false;
      for (int i = map_info.name.size() - 1; i >= 0; --i) {
        if (fill_base_string == false && map_info.name[i] != '/') {
          map_name += map_info.name[i];
        } else {
          map_base_path += map_info.name[i];
          fill_base_string = true;
        }
      }
      std::reverse(map_name.begin(), map_name.end());
      std::reverse(map_base_path.begin(), map_base_path.end());
      std::string XODRFolder = map_base_path + "/OpenDrive/" + map_name + ".xodr";
      if (FileTransfer::FileExists(XODRFolder) == false) _client.GetRequiredFiles();
      _open_drive_file = _client.GetMapData();
      _cached_map = MakeShared<Map>(map_info, _open_drive_file);
    }

    return _cached_map;
  }

  // ===========================================================================
  // -- Required files ---------------------------------------------------------
  // ===========================================================================


    bool Simulator::SetFilesBaseFolder(const std::string &path) {
      return _client.SetFilesBaseFolder(path);
    }

    std::vector<std::string> Simulator::GetRequiredFiles(const std::string &folder, const bool download) const {
      return _client.GetRequiredFiles(folder, download);
    }

    void Simulator::RequestFile(const std::string &name) const {
      _client.RequestFile(name);
    }

    std::vector<uint8_t> Simulator::GetCacheFile(const std::string &name, const bool request_otherwise) const {
      return _client.GetCacheFile(name, request_otherwise);
    }

  // ===========================================================================
  // -- Tick -------------------------------------------------------------------
  // ===========================================================================

  WorldSnapshot Simulator::WaitForTick(time_duration timeout) {
    DEBUG_ASSERT(_episode != nullptr);

    // tick pedestrian navigation
    _episode->NavigationTick();

    auto result = _episode->WaitForState(timeout);
    if (!result.has_value()) {
      throw_exception(TimeoutException(_client.GetEndpoint(), timeout));
    }
    return *result;
  }

  uint64_t Simulator::Tick(time_duration timeout) {
    DEBUG_ASSERT(_episode != nullptr);
    
    // tick pedestrian navigation
    _episode->NavigationTick();
    
    // send tick command
    const auto frame = _client.SendTickCue();

    // waits until new episode is received
    bool result = SynchronizeFrame(frame, *_episode, timeout);
    if (!result) {
      throw_exception(TimeoutException(_client.GetEndpoint(), timeout));
    }
    return frame;
  }

  // ===========================================================================
  // -- Access to global objects in the episode --------------------------------
  // ===========================================================================

  SharedPtr<BlueprintLibrary> Simulator::GetBlueprintLibrary() {
    auto defs = _client.GetActorDefinitions();
    return MakeShared<BlueprintLibrary>(std::move(defs));
  }

  rpc::VehicleLightStateList Simulator::GetVehiclesLightStates() {
    return _client.GetVehiclesLightStates();
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
    else if (settings.synchronous_mode && settings.substepping) {
      if(settings.max_substeps < 1 || settings.max_substeps > 16) {
        log_warning(
            "synchronous mode and substepping are enabled but the number of substeps is not valid. "
            "Please be aware that this value needs to be in the range [1-16].");
      }
      double n_substeps = settings.fixed_delta_seconds.get() / settings.max_substep_delta_time;

      if (n_substeps > static_cast<double>(settings.max_substeps)) {
        log_warning(
            "synchronous mode and substepping are enabled but the values for the simulation are not valid. "
            "The values should fulfil fixed_delta_seconds <= max_substep_delta_time * max_substeps. "
            "Be very careful about that, the time deltas are not guaranteed.");
      }
    }
    const auto frame = _client.SetEpisodeSettings(settings);

    using namespace std::literals::chrono_literals;
    SynchronizeFrame(frame, *_episode, 1s);

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

  void Simulator::SetPedestriansCrossFactor(float percentage) {
    DEBUG_ASSERT(_episode != nullptr);
    auto navigation = _episode->CreateNavigationIfMissing();
    DEBUG_ASSERT(navigation != nullptr);
    navigation->SetPedestriansCrossFactor(percentage);
  }

  void Simulator::SetPedestriansSeed(unsigned int seed) {
    DEBUG_ASSERT(_episode != nullptr);
    auto navigation = _episode->CreateNavigationIfMissing();
    DEBUG_ASSERT(navigation != nullptr);
    navigation->SetPedestriansSeed(seed);
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
  
  void Simulator::UnSubscribeFromSensor(Actor &sensor) {
    _client.UnSubscribeFromStream(sensor.GetActorDescription().GetStreamToken());
    // If in the future we need to unsubscribe from each gbuffer individually, it should be done here.
  }

  void Simulator::SubscribeToGBuffer(
      Actor &actor,
      uint32_t gbuffer_id,
      std::function<void(SharedPtr<sensor::SensorData>)> callback) {
    _client.SubscribeToGBuffer(actor.GetId(), gbuffer_id,
        [cb=std::move(callback), ep=WeakEpisodeProxy{shared_from_this()}](auto buffer) {
          auto data = sensor::Deserializer::Deserialize(std::move(buffer));
          data->_episode = ep.TryLock();
          cb(std::move(data));
        });
  }

  void Simulator::UnSubscribeFromGBuffer(Actor &actor, uint32_t gbuffer_id) {
    _client.UnSubscribeFromGBuffer(actor.GetId(), gbuffer_id);
  }

  void Simulator::FreezeAllTrafficLights(bool frozen) {
    _client.FreezeAllTrafficLights(frozen);
  }

  // =========================================================================
  /// -- Texture updating operations
  // =========================================================================

  void Simulator::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureColor& Texture) {
    _client.ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }

  void Simulator::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _client.ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }

  std::vector<std::string> Simulator::GetNamesOfAllObjects() const {
    return _client.GetNamesOfAllObjects();
  }

} // namespace detail
} // namespace client
} // namespace carla
