// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/World.h"

#include "carla/Logging.h"
#include "carla/client/Actor.h"
#include "carla/client/ActorBlueprint.h"
#include "carla/client/ActorList.h"
#include "carla/client/detail/Simulator.h"
#include "carla/StringUtil.h"
#include "carla/road/SignalType.h"
#include "carla/road/Junction.h"
#include "carla/client/TrafficLight.h"

#include <exception>

namespace carla {
namespace client {

  SharedPtr<Map> World::GetMap() const {
    return _episode.Lock()->GetCurrentMap();
  }

  void World::LoadLevelLayer(rpc::MapLayer map_layers) const {
    _episode.Lock()->LoadLevelLayer(map_layers);
  }

  void World::UnloadLevelLayer(rpc::MapLayer map_layers) const {
    _episode.Lock()->UnloadLevelLayer(map_layers);
  }

  SharedPtr<BlueprintLibrary> World::GetBlueprintLibrary() const {
    return _episode.Lock()->GetBlueprintLibrary();
  }

  rpc::VehicleLightStateList World::GetVehiclesLightStates() const {
    return _episode.Lock()->GetVehiclesLightStates();
  }

  std::optional<geom::Location> World::GetRandomLocationFromNavigation() const {
    return _episode.Lock()->GetRandomLocationFromNavigation();
  }

  SharedPtr<Actor> World::GetSpectator() const {
    return _episode.Lock()->GetSpectator();
  }

  rpc::EpisodeSettings World::GetSettings() const {
    return _episode.Lock()->GetEpisodeSettings();
  }

  uint64_t World::ApplySettings(const rpc::EpisodeSettings &settings, time_duration timeout) {
    rpc::EpisodeSettings new_settings = settings;
    uint64_t id = _episode.Lock()->SetEpisodeSettings(settings);

    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout;

    if (settings.fixed_delta_seconds.has_value()) {
      using namespace std::literals::chrono_literals;

      const auto number_of_attemps = 30u;
      uint64_t tics_correct = 0;
      for (auto i = 0u; i < number_of_attemps; i++) {
        const auto curr_snapshot = GetSnapshot();

        const double error = abs(new_settings.fixed_delta_seconds.value() - curr_snapshot.GetTimestamp().delta_seconds);
        if (error < std::numeric_limits<float>::epsilon())
          tics_correct++;

        if (tics_correct >= 2)
          return id;

        Tick(local_timeout);
      }

      log_warning("World::ApplySettings: After", number_of_attemps, " attemps, the settings were not correctly set. Please check that everything is consistent.");
    }
    return id;
  }

  rpc::WeatherParameters World::GetWeather() const {
    return _episode.Lock()->GetWeatherParameters();
  }

  void World::SetWeather(const rpc::WeatherParameters &weather) {
    _episode.Lock()->SetWeatherParameters(weather);
  }

  bool World::IsWeatherEnabled() const {
    return _episode.Lock()->IsWeatherEnabled();
  }

  WorldSnapshot World::GetSnapshot() const {
    return _episode.Lock()->GetWorldSnapshot();
  }

  SharedPtr<Actor> World::GetActor(ActorId id) const {
    auto simulator = _episode.Lock();
    auto description = simulator->GetActorById(id);
    return description.has_value() ?
        simulator->MakeActor(std::move(*description)) :
        nullptr;
  }

  SharedPtr<ActorList> World::GetActors() const {
    return SharedPtr<ActorList>{new ActorList{
                                  _episode,
                                  _episode.Lock()->GetAllTheActorsInTheEpisode()}};
  }

  SharedPtr<ActorList> World::GetActors(const std::vector<ActorId> &actor_ids) const {
    return SharedPtr<ActorList>{new ActorList{
                                  _episode,
                                  _episode.Lock()->GetActorsById(actor_ids)}};
  }

  SharedPtr<Actor> World::SpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent_actor,
      rpc::AttachmentType attachment_type) {
    return _episode.Lock()->SpawnActor(blueprint, transform, parent_actor, attachment_type);
  }

  SharedPtr<Actor> World::TrySpawnActor(
      const ActorBlueprint &blueprint,
      const geom::Transform &transform,
      Actor *parent_actor,
      rpc::AttachmentType attachment_type) noexcept {
    try {
      return SpawnActor(blueprint, transform, parent_actor, attachment_type);
    } catch (const std::exception &) {
      return nullptr;
    }
  }

  WorldSnapshot World::WaitForTick(time_duration timeout) const {
    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout;

    return _episode.Lock()->WaitForTick(local_timeout);
  }

  size_t World::OnTick(std::function<void(WorldSnapshot)> callback) {
    return _episode.Lock()->RegisterOnTickEvent(std::move(callback));
  }

  void World::RemoveOnTick(size_t callback_id) {
    _episode.Lock()->RemoveOnTickEvent(callback_id);
  }

  uint64_t World::Tick(time_duration timeout) {
    time_duration local_timeout = timeout.milliseconds() == 0 ?
        _episode.Lock()->GetNetworkingTimeout() : timeout;
    return _episode.Lock()->Tick(local_timeout);
  }

  void World::SetPedestriansCrossFactor(float percentage) {
    _episode.Lock()->SetPedestriansCrossFactor(percentage);
  }

  void World::SetPedestriansSeed(unsigned int seed) {
    _episode.Lock()->SetPedestriansSeed(seed);
  }

  SharedPtr<Actor> World::GetTrafficSign(const Landmark& landmark) const {
    SharedPtr<ActorList> actors = GetActors();
    SharedPtr<TrafficSign> result;
    std::string landmark_id = landmark.GetId();
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i);
      if (StringUtil::Match(actor->GetTypeId(), "*traffic.*")) {
        TrafficSign* sign = static_cast<TrafficSign*>(actor.get());
        if(sign && (sign->GetSignId() == landmark_id)) {
          return actor;
        }
      }
    }
    return nullptr;
  }

  SharedPtr<Actor> World::GetTrafficLight(const Landmark& landmark) const {
    SharedPtr<ActorList> actors = GetActors();
    SharedPtr<TrafficLight> result;
    std::string landmark_id = landmark.GetId();
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i);
      if (StringUtil::Match(actor->GetTypeId(), "*traffic_light*")) {
        TrafficLight* tl = static_cast<TrafficLight*>(actor.get());
        if(tl && (tl->GetSignId() == landmark_id)) {
          return actor;
        }
      }
    }
    return nullptr;
  }

  SharedPtr<Actor> World::GetTrafficLightFromOpenDRIVE(const road::SignId& sign_id) const {
    SharedPtr<ActorList> actors = GetActors();
    SharedPtr<TrafficLight> result;
    for (size_t i = 0; i < actors->size(); i++) {
      SharedPtr<Actor> actor = actors->at(i);
      if (StringUtil::Match(actor->GetTypeId(), "*traffic_light*")) {
        TrafficLight* tl = static_cast<TrafficLight*>(actor.get());
        if(tl && (tl->GetSignId() == sign_id)) {
          return actor;
        }
      }
    }
    return nullptr;
  }

  void World::ResetAllTrafficLights() {
    _episode.Lock()->ResetAllTrafficLights();
  }

  SharedPtr<LightManager> World::GetLightManager() const {
    return _episode.Lock()->GetLightManager();
  }

  void World::FreezeAllTrafficLights(bool frozen) {
    _episode.Lock()->FreezeAllTrafficLights(frozen);
  }

  std::vector<geom::BoundingBox> World::GetLevelBBs(uint8_t queried_tag) const {
    return _episode.Lock()->GetLevelBBs(queried_tag);
  }

  std::vector<rpc::EnvironmentObject> World::GetEnvironmentObjects(uint8_t queried_tag) const {
    return _episode.Lock()->GetEnvironmentObjects(queried_tag);
  }

  void World::EnableEnvironmentObjects(
      std::vector<uint64_t> env_objects_ids,
      bool enable) const {
    _episode.Lock()->EnableEnvironmentObjects(env_objects_ids, enable);
  }

  std::optional<rpc::LabelledPoint> World::ProjectPoint(
      geom::Location location, geom::Vector3D direction, float search_distance) const {
    auto result = _episode.Lock()->ProjectPoint(location, direction, search_distance);
    if (result.first) {
      return result.second;
    }
    return {};
  }

  std::optional<rpc::LabelledPoint> World::GroundProjection(
      geom::Location location, float search_distance) const {
    const geom::Vector3D DownVector(0,0,-1);
    return ProjectPoint(location, DownVector, search_distance);
  }

  std::vector<rpc::LabelledPoint> World::CastRay(
      geom::Location start_location, geom::Location end_location) const {
    return _episode.Lock()->CastRay(start_location, end_location);
  }

  std::vector<SharedPtr<Actor>> World::GetTrafficLightsFromWaypoint(
      const Waypoint& waypoint, double distance) const {
    std::vector<SharedPtr<Actor>> Result;
    std::vector<SharedPtr<Landmark>> landmarks =
        waypoint.GetAllLandmarksInDistance(distance);
    std::set<std::string> added_signals;
    for (auto& landmark : landmarks) {
      if (road::SignalType::IsTrafficLight(landmark->GetType())) {
        SharedPtr<Actor> traffic_light = GetTrafficLight(*(landmark.get()));
        if (traffic_light) {
          if(added_signals.count(landmark->GetId()) == 0) {
            Result.emplace_back(traffic_light);
            added_signals.insert(landmark->GetId());
          }
        }
      }
    }
    return Result;
  }

  std::vector<SharedPtr<Actor>> World::GetTrafficLightsInJunction(
      const road::JuncId junc_id) const {
    std::vector<SharedPtr<Actor>> Result;
    SharedPtr<Map> map = GetMap();
    const road::Junction* junction = map->GetMap().GetJunction(junc_id);
    for (const road::ContId& cont_id : junction->GetControllers()) {
      const std::unique_ptr<road::Controller>& controller =
          map->GetMap().GetControllers().at(cont_id);
      for (road::SignId sign_id : controller->GetSignals()) {
        SharedPtr<Actor> traffic_light = GetTrafficLightFromOpenDRIVE(sign_id);
        if (traffic_light) {
          Result.emplace_back(traffic_light);
        }
      }
    }
    return Result;
  }

  void World::ApplyColorTextureToObject(
      const std::string &object_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects({object_name}, parameter, Texture);
  }

  void World::ApplyColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }

  void World::ApplyFloatColorTextureToObject(
      const std::string &object_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects({object_name}, parameter, Texture);
  }

  void World::ApplyFloatColorTextureToObjects(
      const std::vector<std::string> &objects_name,
      const rpc::MaterialParameter& parameter,
      const rpc::TextureFloatColor& Texture) {
    _episode.Lock()->ApplyColorTextureToObjects(objects_name, parameter, Texture);
  }

  std::vector<std::string> World::GetNamesOfAllObjects() const {
    return _episode.Lock()->GetNamesOfAllObjects();
  }

  void World::ApplyTexturesToObject(
      const std::string &object_name,
      const rpc::TextureColor& diffuse_texture,
      const rpc::TextureFloatColor& emissive_texture,
      const rpc::TextureFloatColor& normal_texture,
      const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture)
  {
    if (diffuse_texture.GetWidth() && diffuse_texture.GetHeight()) {
      ApplyColorTextureToObject(
          object_name, rpc::MaterialParameter::Tex_Diffuse, diffuse_texture);
    }
    if (normal_texture.GetWidth() && normal_texture.GetHeight()) {
      ApplyFloatColorTextureToObject(
          object_name, rpc::MaterialParameter::Tex_Normal, normal_texture);
    }
    if (ao_roughness_metallic_emissive_texture.GetWidth() &&
        ao_roughness_metallic_emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObject(
          object_name,
          rpc::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive,
          ao_roughness_metallic_emissive_texture);
    }
    if (emissive_texture.GetWidth() && emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObject(
          object_name, rpc::MaterialParameter::Tex_Emissive, emissive_texture);
    }
  }

  void World::ApplyTexturesToObjects(
      const std::vector<std::string> &objects_names,
      const rpc::TextureColor& diffuse_texture,
      const rpc::TextureFloatColor& emissive_texture,
      const rpc::TextureFloatColor& normal_texture,
      const rpc::TextureFloatColor& ao_roughness_metallic_emissive_texture)
  {
    if (diffuse_texture.GetWidth() && diffuse_texture.GetHeight()) {
      ApplyColorTextureToObjects(
          objects_names, rpc::MaterialParameter::Tex_Diffuse, diffuse_texture);
    }
    if (normal_texture.GetWidth() && normal_texture.GetHeight()) {
      ApplyFloatColorTextureToObjects(
          objects_names, rpc::MaterialParameter::Tex_Normal, normal_texture);
    }
    if (ao_roughness_metallic_emissive_texture.GetWidth() &&
        ao_roughness_metallic_emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObjects(
          objects_names,
          rpc::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive,
          ao_roughness_metallic_emissive_texture);
    }
    if (emissive_texture.GetWidth() && emissive_texture.GetHeight()) {
      ApplyFloatColorTextureToObjects(
          objects_names, rpc::MaterialParameter::Tex_Emissive, emissive_texture);
    }
  }

} // namespace client
} // namespace carla
