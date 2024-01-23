
// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetColor(_id);
}

float Light::GetIntensity() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetIntensity(_id);
}

LightGroup Light::GetLightGroup() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetLightGroup(_id);
}

LightState Light::GetLightState() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->GetLightState(_id);
}

bool Light::IsOn() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->IsActive(_id) == true;
}

bool Light::IsOff() const {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  return light_manager->IsActive(_id) == false;
}

void Light::SetColor(Color color) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetColor(_id, color);
}

void Light::SetIntensity(float intensity) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetIntensity(_id, intensity);
}

void Light::SetLightGroup(LightGroup group) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetLightGroup(_id, group);
}

void Light::SetLightState(const LightState& state) {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetLightState(_id, state);
}

void Light::TurnOn() {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetActive(_id, true);
}

void Light::TurnOff() {
  auto light_manager = _light_manager.lock();
  assert(light_manager && "No light_manager");
  light_manager->SetActive(_id, false);
}


} // namespace client
} // namespace carla
