// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "LightManager.h"


namespace carla {
namespace client {

std::vector<Light> LightManager::GetAllLights(LightGroup /* type */) {
  std::vector<Light> result;
  /* for(auto lights_state : _lights_state) {
    LightGroup group = lights_state.second._group;
    if((type == LightGroup::None) || (group == type)) {
      auto it_light = _lights[lights_state.first];
      result.push_back(it_light);
    }
  } */
  return result;
}

void LightManager::SetColor(std::vector<Light> lights, Color color) {
  for(Light& light : lights) {
    SetColor(light.GetId(), color);
  }
}

void LightManager::SetColor(std::vector<Light> lights, std::vector<Color> colors) {
  size_t lights_to_update = (lights.size() < colors.size()) ? lights.size() : colors.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetColor(lights[i].GetId(), colors[i]);
  }
}

std::vector<Color> LightManager::GetColor(std::vector<Light> lights) {
  std::vector<Color> result;
  for(Light& light : lights) {
    result.push_back( GetColor(light.GetId()) );
  }
  return result;
}


void LightManager::SetIntensity(std::vector<Light> lights, float intensity) {
  for(Light& light : lights) {
    SetIntensity(light.GetId(), intensity);
  }
}

void LightManager::SetIntensity(std::vector<Light> lights, std::vector<float> intensities) {
  size_t lights_to_update = (lights.size() < intensities.size()) ? lights.size() : intensities.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetIntensity(lights[i].GetId(), intensities[i]);
  }
}

std::vector<float> LightManager::GetIntensity(std::vector<Light> lights) {
  std::vector<float> result;
  for(Light& light : lights) {
    result.push_back( GetIntensity(light.GetId()) );
  }
  return result;
}

void LightManager::SetLightGroup(std::vector<Light> lights, LightGroup group) {
  for(Light& light : lights) {
    SetLightGroup(light.GetId(), group);
  }
}

void LightManager::SetLightGroup(std::vector<Light> lights, std::vector<LightGroup> groups) {
  size_t lights_to_update = (lights.size() < groups.size()) ? lights.size() : groups.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetLightGroup(lights[i].GetId(), groups[i]);
  }
}

std::vector<LightGroup> LightManager::GetLightGroup(std::vector<Light> lights) {
  std::vector<LightGroup> result;
  for(Light& light : lights) {
    result.push_back( GetLightGroup(light.GetId()) );
  }
  return result;
}

void LightManager::SetState(std::vector<Light> lights, LightState state) {
  for(Light& light : lights) {
    SetState(light.GetId(), state);
  }
}

void LightManager::SetState(std::vector<Light> lights, std::vector<LightState> states) {
  size_t lights_to_update = (lights.size() < states.size()) ? lights.size() : states.size();
  for(size_t i = 0; i < lights_to_update; i++) {
    SetState(lights[i].GetId(), states[i]);
  }
}

std::vector<LightState> LightManager::GetState(std::vector<Light> lights) {
  std::vector<LightState> result;
  for(Light& light : lights) {
    result.push_back( GetState(light.GetId()) );
  }
  return result;
}


inline Color LightManager::GetColor(LightId id) {
  return RetrieveLightState(id)._color;
}

inline float LightManager::GetIntensity(LightId id) {
  return RetrieveLightState(id)._intensity;
}

inline LightState LightManager::GetState(LightId id) {
  return RetrieveLightState(id);
}

inline LightGroup LightManager::GetLightGroup(LightId id) {
  return RetrieveLightState(id)._group;
}

inline void LightManager::SetColor(LightId id, Color color) {
  RetrieveLightState(id)._color = color;
}

inline void LightManager::SetIntensity(LightId id, float intensity) {
  RetrieveLightState(id)._intensity = intensity;
}

inline void LightManager::SetState(LightId id, const LightState& state) {
  RetrieveLightState(id) = state;
}

inline void LightManager::SetLightGroup(LightId id, LightGroup group) {
  RetrieveLightState(id)._group = group;
}

inline LightState& LightManager::RetrieveLightState(LightId id) {
  auto it = _lights_state.find(id);
  if(it == _lights_state.end()) {
    carla::log_warning("Invalid light", id);
    _state.Reset();
    return _state;
  }
  return it->second;
}

void LightManager::UpdateLocalLightsState() {

}

void LightManager::UpdateServerLightsState() const {

}

#undef GETTER
#undef TRAVERSE_AND_GET

} // namespace client
} // namespace carla