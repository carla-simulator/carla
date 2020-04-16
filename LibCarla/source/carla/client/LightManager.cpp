// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LightManager.h"

#include "carla/client/detail/Simulator.h"


namespace carla {
namespace client {

  using LightGroup = rpc::LightState::LightGroup;

LightManager::~LightManager(){
  if(_episode.IsValid()) {
    _episode.Lock()->RemoveOnTickEvent(_on_light_update_register_id);
    _episode.Lock()->RemoveLightUpdateChangeEvent(_on_light_update_register_id);
  }
  UpdateServerLightsState(true);
}

void LightManager::SetEpisode(detail::EpisodeProxy episode) {

  _episode = episode;

  _on_tick_register_id = _episode.Lock()->RegisterOnTickEvent(
    [&](const WorldSnapshot&) {
      UpdateServerLightsState();
    });

  _on_light_update_register_id = _episode.Lock()->RegisterLightUpdateChangeEvent(
    [&](const WorldSnapshot& ) {
      QueryLightsStateToServer();
      ApplyChanges();
    });

    QueryLightsStateToServer();
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
  std::lock_guard<std::mutex> lock(_mutex);
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._active = active;
  _lights_changes[id] = state;
  _dirty = true;
}

void LightManager::SetColor(LightId id, Color color) {
  std::lock_guard<std::mutex> lock(_mutex);
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._color = color;
  _lights_changes[id] = state;
  _dirty = true;
}

void LightManager::SetIntensity(LightId id, float intensity) {
  std::lock_guard<std::mutex> lock(_mutex);
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._intensity = intensity;
  _lights_changes[id] = state;
  _dirty = true;
}

void LightManager::SetLightState(LightId id, const LightState& new_state) {
  std::lock_guard<std::mutex> lock(_mutex);
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state = new_state;
  _lights_changes[id] = state;
  _dirty = true;
}

void LightManager::SetLightGroup(LightId id, LightGroup group) {
  std::lock_guard<std::mutex> lock(_mutex);
  LightState& state = const_cast<LightState&>(RetrieveLightState(id));
  state._group = group;
  _lights_changes[id] = state;
  _dirty = true;
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
  std::lock_guard<std::mutex> lock(_mutex);
  // Send blocking query
  std::vector<rpc::LightState> lights_snapshot = _episode.Lock()->QueryLightsStateToServer();

  // Update lights
  SharedPtr<LightManager> lm = _episode.Lock()->GetLightManager();

  for(const auto& it : lights_snapshot) {
    _lights_state[it._id] = LightState(
        it._intensity,
        Color(it._color.r, it._color.g, it._color.b),
        static_cast<LightState::LightGroup>(it._group),
        it._active
    );

    if(_lights.find(it._id) == _lights.end())
    {
      _lights[it._id] = Light(lm, it._location, it._id);
    }
  }
}

void LightManager::UpdateServerLightsState(bool discard_client) {
  std::lock_guard<std::mutex> lock(_mutex);

  if(_dirty) {
    std::vector<rpc::LightState> message;
    for(auto it : _lights_changes) {
      auto it_light = _lights.find(it.first);
      if(it_light != _lights.end()) {
        rpc::LightState state(
          it_light->second.GetLocation(),
          it.second._intensity,
          it.second._group,
          rpc::Color(it.second._color.r, it.second._color.g, it.second._color.b),
          it.second._active
        );
        state._id = it.first;
        // Add to command
        message.push_back(state);
      }
    }
    _episode.Lock()->UpdateServerLightsState(message, discard_client);

    _lights_changes.clear();
    _dirty = false;
  }
}

void LightManager::ApplyChanges() {
  std::lock_guard<std::mutex> lock(_mutex);
  for(const auto& it : _lights_changes) {
    SetLightState(it.first, it.second);
  }
}

} // namespace client
} // namespace carla
