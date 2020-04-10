// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>
#include <unordered_map>

#include "carla/Memory.h"
#include "carla/NonCopyable.h"

#include "carla/client/Light.h"
#include "carla/client/LightState.h"

// TODO: on/off
// TODO: setters for groups, ie, setOn(LightType::Street)
// TODO: LightList like ActorList

namespace carla {
namespace client {

class LightManager
  : public EnableSharedFromThis<LightManager> {

public:

  LightManager() {}

  std::vector<Light> GetAllLights(LightGroup type = LightGroup::None);
  // std::vector<Light> GetAllLightsInRoad(RoadId id, LightGroup type = LightGroup::None);
  // std::vector<Light> GetAllLightsInDistance(Vec3 origin, float distance, LightGroup type = Light::LightType::None);

  void SetColor(std::vector<Light> lights, Color color);
  void SetColor(std::vector<Light> lights, std::vector<Color> colors);
  std::vector<Color> GetColor(std::vector<Light> lights);

  void SetIntensity(std::vector<Light> lights, float intensity);
  void SetIntensity(std::vector<Light> lights, std::vector<float> intensities);
  std::vector<float> GetIntensity(std::vector<Light> lights);

  void SetLightGroup(std::vector<Light> lights, LightGroup group);
  void SetLightGroup(std::vector<Light> lights, std::vector<LightGroup> groups);
  std::vector<LightGroup> GetLightGroup(std::vector<Light> lights);

  void SetState(std::vector<Light> lights, LightState state);
  void SetState(std::vector<Light> lights, std::vector<LightState> states);
  std::vector<LightState> GetState(std::vector<Light> lights);

  Color GetColor(LightId id);
  float GetIntensity(LightId id);
  LightState GetState(LightId id);
  LightGroup GetLightGroup(LightId id);

  void SetColor(LightId id, Color color);
  void SetIntensity(LightId id, float intensity);
  void SetState(LightId id, const LightState& state);
  void SetLightGroup(LightId id, LightGroup group);

private:

  LightState& RetrieveLightState(LightId id);

  void UpdateLocalLightsState();
  void UpdateServerLightsState() const;

  std::unordered_map<LightId, LightState> _lights_state;
  std::unordered_map<LightId, Light> _lights;

  LightState _state;
};

} // namespace client
} // namespace carla

