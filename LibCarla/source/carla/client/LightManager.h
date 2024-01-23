// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "carla/Memory.h"
#include "carla/NonCopyable.h"

#include "carla/client/detail/Episode.h"
#include "carla/client/Light.h"
#include "carla/client/LightState.h"
#include "carla/rpc/LightState.h"

namespace carla {
namespace client {

class LightManager
  : public EnableSharedFromThis<LightManager> {

  using LightGroup = rpc::LightState::LightGroup;

public:

  LightManager() {}

  ~LightManager();

  LightManager(const LightManager& other) : EnableSharedFromThis<LightManager>() {
    _lights_state = other._lights_state;
    _lights_changes = other._lights_changes;
    _lights = other._lights;
    _episode = other._episode;
    _on_tick_register_id = other._on_tick_register_id;
    _on_light_update_register_id = other._on_light_update_register_id;
    _dirty = other._dirty;
  }

  void SetEpisode(detail::WeakEpisodeProxy episode);

  std::vector<Light> GetAllLights(LightGroup type = LightGroup::None) const;
  // TODO: std::vector<Light> GetAllLightsInRoad(RoadId id, LightGroup type = LightGroup::None);
  // TODO: std::vector<Light> GetAllLightsInDistance(Vec3 origin, float distance, LightGroup type = Light::LightType::None);

  void TurnOn(std::vector<Light>& lights);
  void TurnOff(std::vector<Light>& lights);
  void SetActive(std::vector<Light>& lights, std::vector<bool>& active);
  std::vector<bool> IsActive(std::vector<Light>& lights) const;
  std::vector<Light> GetTurnedOnLights(LightGroup type = LightGroup::None) const;
  std::vector<Light> GetTurnedOffLights(LightGroup type = LightGroup::None) const;

  void SetColor(std::vector<Light>& lights, Color color);
  void SetColor(std::vector<Light>& lights, std::vector<Color>& colors);
  std::vector<Color> GetColor(std::vector<Light>& lights) const;

  void SetIntensity(std::vector<Light>& lights, float intensity);
  void SetIntensity(std::vector<Light>& lights, std::vector<float>& intensities);
  std::vector<float> GetIntensity(std::vector<Light>& lights) const;

  void SetLightGroup(std::vector<Light>& lights, LightGroup group);
  void SetLightGroup(std::vector<Light>& lights, std::vector<LightGroup>& groups);
  std::vector<LightGroup> GetLightGroup(std::vector<Light>& lights) const;

  void SetLightState(std::vector<Light>& lights, LightState state);
  void SetLightState(std::vector<Light>& lights, std::vector<LightState>& states);
  std::vector<LightState> GetLightState(std::vector<Light>& lights) const;

  Color GetColor(LightId id) const;
  float GetIntensity(LightId id) const;
  LightState GetLightState(LightId id) const;
  LightGroup GetLightGroup(LightId id) const;
  bool IsActive(LightId id) const;

  void SetActive(LightId id, bool active);
  void SetColor(LightId id, Color color);
  void SetIntensity(LightId id, float intensity);
  void SetLightState(LightId id, const LightState& new_state);
  void SetLightStateNoLock(LightId id, const LightState& new_state);
  void SetLightGroup(LightId id, LightGroup group);

  void SetDayNightCycle(const bool active);

private:

  const LightState& RetrieveLightState(LightId id) const;

  void QueryLightsStateToServer();
  void UpdateServerLightsState(bool discard_client = false);
  void ApplyChanges();

  std::unordered_map<LightId, LightState> _lights_state;
  std::unordered_map<LightId, LightState> _lights_changes;
  std::unordered_map<LightId, Light> _lights;

  detail::WeakEpisodeProxy _episode;

  std::mutex _mutex;

  LightState _state;
  size_t _on_tick_register_id = 0;
  size_t _on_light_update_register_id = 0;
  bool _dirty = false;
};

} // namespace client
} // namespace carla
