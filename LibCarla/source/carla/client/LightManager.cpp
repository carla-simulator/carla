// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LightManager.h"

#include "carla/client/detail/Simulator.h"


namespace carla {
namespace client {

LightManager::LightManager() {
  // QueryLightsStateToServer();
}

LightManager::~LightManager(){
  if(_episode.IsValid()) {
    _episode.Lock()->RemoveOnTickEvent(_on_tick_register_id);
  }
  // TODO: send pending changes
}

void LightManager::SetEpisode(detail::EpisodeProxy& episode) {
  _episode = episode;

  auto self = boost::static_pointer_cast<LightManager>(shared_from_this());

  _on_tick_register_id = _episode.Lock()->RegisterOnTickEvent(
    [&](const WorldSnapshot& /* snapshot */) {
      // if (snapshot.lights_dirty)
      //  QueryLightsStateToServer();
      UpdateServerLightsState();
    });
}

std::vector<Light> LightManager::GetAllLights(LightGroup type) const {
  std::vector<Light> result;

  for(auto lights_state : _lights_state) {
    LightGroup group = lights_state.second._group;
    if((type == LightGroup::None) || (group == type)) {
      auto it_light = _lights.find(lights_state.first);
      result.push_back(it_light->second);
    }
  }

  return result;
}

void LightManager::TurnOn(std::vector<Light>& lights) {
  for(Light& light : lights) {
    SetActive(light._id, true);
  }
}

void LightManager::TurnOff(std::vector<Light>& lights) {
  for(Light& light : lights) {
    SetActive(light._id, false);
  }
}

void LightManager::SetActive(std::vector<Light>& lights, std::vector<bool>& active) {
  size_t lights_to_update = (lights.size() < active.size()) ? lights.size() : active.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetActive(lights[i]._id, active[i]);
  }
}

std::vector<bool> LightManager::IsActive(std::vector<Light>& lights) const {
  std::vector<bool> result;
  for(Light& light : lights) {
    result.push_back( IsActive(light._id) );
  }
  return result;
}

std::vector<Light> LightManager::GetTurnedOnLights(LightGroup type) const {
  std::vector<Light> result;

  for(auto lights_state : _lights_state) {
    LightState& state = lights_state.second;
    LightGroup group = state._group;
    if( (type == LightGroup::None || group == type) && state._active ) {
      auto it_light = _lights.find(lights_state.first);
      result.push_back(it_light->second);
    }
  }

  return result;
}

std::vector<Light> LightManager::GetTurnedOffLights(LightGroup type) const {
  std::vector<Light> result;

  for(auto lights_state : _lights_state) {
    LightState& state = lights_state.second;
    LightGroup group = state._group;
    if( (type == LightGroup::None || group == type) && !state._active ) {
      auto it_light = _lights.find(lights_state.first);
      result.push_back(it_light->second);
    }
  }

  return result;
}

void LightManager::SetColor(std::vector<Light>& lights, Color color) {
  for(Light& light : lights) {
    SetColor(light._id, color);
  }
}

void LightManager::SetColor(std::vector<Light>& lights, std::vector<Color>& colors) {
  size_t lights_to_update = (lights.size() < colors.size()) ? lights.size() : colors.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetColor(lights[i]._id, colors[i]);
  }
}

std::vector<Color> LightManager::GetColor(std::vector<Light>& lights) const {
  std::vector<Color> result;
  for(Light& light : lights) {
    result.push_back( GetColor(light._id) );
  }
  return result;
}


void LightManager::SetIntensity(std::vector<Light>& lights, float intensity) {
  for(Light& light : lights) {
    SetIntensity(light._id, intensity);
  }
}

void LightManager::SetIntensity(std::vector<Light>& lights, std::vector<float>& intensities) {
  size_t lights_to_update = (lights.size() < intensities.size()) ? lights.size() : intensities.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetIntensity(lights[i]._id, intensities[i]);
  }
}

std::vector<float> LightManager::GetIntensity(std::vector<Light>& lights) const {
  std::vector<float> result;
  for(Light& light : lights) {
    result.push_back( GetIntensity(light._id) );
  }
  return result;
}

void LightManager::SetLightGroup(std::vector<Light>& lights, LightGroup group) {
  for(Light& light : lights) {
    SetLightGroup(light._id, group);
  }
}

void LightManager::SetLightGroup(std::vector<Light>& lights, std::vector<LightGroup>& groups) {
  size_t lights_to_update = (lights.size() < groups.size()) ? lights.size() : groups.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetLightGroup(lights[i]._id, groups[i]);
  }
}

std::vector<LightGroup> LightManager::GetLightGroup(std::vector<Light>& lights) const {
  std::vector<LightGroup> result;
  for(Light& light : lights) {
    result.push_back( GetLightGroup(light._id) );
  }
  return result;
}


void LightManager::SetLightState(std::vector<Light>& lights, LightState state) {
  for(Light& light : lights) {
    SetLightState(light._id, state);
  }
}

void LightManager::SetLightState(std::vector<Light>& lights, std::vector<LightState>& states) {
  size_t lights_to_update = (lights.size() < states.size()) ? lights.size() : states.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetLightState(lights[i]._id, states[i]);
  }
}

std::vector<LightState> LightManager::GetLightState(std::vector<Light>& lights) const {
  std::vector<LightState> result;
  for(Light& light : lights) {
    result.push_back( RetrieveLightState(light._id) );
  }
  return result;
}

Color LightManager::GetColor(LightId id) const {
  return RetrieveLightState(id)._color;
}

float LightManager::GetIntensity(LightId id) const {
  return RetrieveLightState(id)._intensity;
}

LightState LightManager::GetLightState(LightId id) const {
  return RetrieveLightState(id);
}

LightGroup LightManager::GetLightGroup(LightId id) const {
  return RetrieveLightState(id)._group;
}

bool LightManager::IsActive(LightId id) const {
  return RetrieveLightState(id)._active;
}

void LightManager::SetActive(LightId id, bool active) {
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._active = active;
  _lights_changes[id] = state;
}

void LightManager::SetColor(LightId id, Color color) {
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._color = color;
  _lights_changes[id] = state;
}

void LightManager::SetIntensity(LightId id, float intensity) {
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._intensity = intensity;
  _lights_changes[id] = state;
}

void LightManager::SetLightState(LightId id, const LightState& new_state) {
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state = new_state;
  _lights_changes[id] = state;
}

void LightManager::SetLightGroup(LightId id, LightGroup group) {
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._group = group;
  _lights_changes[id] = state;
}

const LightState& LightManager::RetrieveLightState(LightId id) const {
  auto it = _lights_state.find(id);
  if(it == _lights_state.end()) {
    carla::log_warning("Invalid light", id);
    return _state;
  }
  return it->second;
}

void LightManager::QueryLightsStateToServer() {
  carla::log_warning("LightManager::QueryLightsStateToServer");
  // Send blocking query
  // Update lights
  // _lights_state = new_lights_state
}

void LightManager::UpdateServerLightsState() {
  if(_dirty) {
    // std::vector<LightChangeCmd> message;
    carla::log_warning("LightManager::UpdateServerLightsState is dirty");
    // for(const auto& it : _lights_changes) {
      // Add to command
      // message.push_back();
      // Reapply change to local snapshot, in case other client has set the light state first we need to reapply our local change
      // SetState(it.first, it.second);
    // }
    _lights_changes.clear();
    _dirty = false;
  } else {
    carla::log_warning("LightManager::UpdateServerLightsState ignore");
  }

}

} // namespace client
} // namespace carla
