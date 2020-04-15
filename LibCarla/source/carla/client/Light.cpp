
// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Light.h"
#include "carla/client/LightManager.h"

#include <assert.h>

namespace carla {
namespace client {

  using LightGroup = rpc::LightState::LightGroup;

Color Light::GetColor() const {
  assert(_light_manager && "No light_manager");
  return _light_manager->GetColor(_id);
}

float Light::GetIntensity() const {
  assert(_light_manager && "No light_manager");
  return _light_manager->GetIntensity(_id);
}

LightGroup Light::GetLightGroup() const {
  assert(_light_manager && "No light_manager");
  return _light_manager->GetLightGroup(_id);
}

LightState Light::GetLightState() const {
  assert(_light_manager && "No light_manager");
  return _light_manager->GetLightState(_id);
}

bool Light::IsOn() const {
  assert(_light_manager && "No light_manager");
  return _light_manager->IsActive(_id) == true;
}

bool Light::IsOff() const {
  assert(_light_manager && "No light_manager");
  return _light_manager->IsActive(_id) == false;
}

void Light::SetColor(Color color) {
  assert(_light_manager && "No light_manager");
  _light_manager->SetColor(_id, color);
}

void Light::SetIntensity(float intensity) {
  assert(_light_manager && "No light_manager");
  _light_manager->SetIntensity(_id, intensity);
}

void Light::SetLightGroup(LightGroup group) {
  assert(_light_manager && "No light_manager");
  _light_manager->SetLightGroup(_id, group);
}

void Light::SetLightState(const LightState& state) {
  assert(_light_manager && "No light_manager");
  _light_manager->SetLightState(_id, state);
}

void Light::TurnOn() {
  assert(_light_manager && "No light_manager");
  _light_manager->SetActive(_id, true);
}

void Light::TurnOff() {
  assert(_light_manager && "No light_manager");
  _light_manager->SetActive(_id, false);
}


} // namespace client
} // namespace carla