
// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Light.h"
#include "carla/client/LightManager.h"

namespace carla {
namespace client {

Color Light::GetColor() const {
  return _light_manager->GetColor(_id);
}

LightId Light::GetId() const {
  return _id;
}

float Light::GetIntensity() const {
  return _light_manager->GetIntensity(_id);
}

const geom::Location Light::GetLocation() const {
  return _location;
}

const geom::Rotation Light::GetRotation() const {
  return _rotation;
}

LightState Light::GetState() const {
  return _light_manager->GetState(_id);
}

LightGroup Light::GetLightGroup() const {
  return _light_manager->GetLightGroup(_id);
}

void Light::SetColor(Color color) {
  _light_manager->SetColor(_id, color);
}

void Light::SetIntensity(float intensity) {
  _light_manager->SetIntensity(_id, intensity);
}

void Light::SetState(const LightState& state) {
  _light_manager->SetState(_id, state);
}

void Light::SetLightGroup(LightGroup group) {
  _light_manager->SetLightGroup(_id, group);
}

} // namespace client
} // namespace carla