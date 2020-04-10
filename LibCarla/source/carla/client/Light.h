// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"

#include "carla/client/LightState.h"
#include "carla/client/LightManager.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"

namespace carla {
namespace client {

class Light {

public:

  Light(SharedPtr<LightManager> light_manager,
    geom::Location& location,
    geom::Rotation& rotation,
    LightId id)
  : _light_manager(light_manager),
    _location (location),
    _rotation (rotation),
    _id (id) {}

  Color GetColor() const {
    return _light_manager->GetColor(_id);
  }

  LightId GetId() const {
    return _id;
  }

  float GetIntensity() const {
    return _light_manager->GetIntensity(_id);
  }

  geom::Location GetLocation const {
    return _location;
  }

  geom::Rotation GetRotation {
    return _location;
  }

  LightState GetState() const {
    return _light_manager->GetState(_id);
  }

  LightGroup GetLightGroup() const {
    return _light_manager->GetLightGroup(_id);
  }

  void SetColor(Color color) {
    _light_manager->SetColor(_id, color);
  }

  void SetIntensity(float intensity) {
    _light_manager->SetIntensity(_id, intensity);
  }

  void SetState(const LightState& state) {
    _light_manager->SetState(_id, state);
  }

  void SetLightGroup(LightGroup group) {
    _light_manager->SetLightGroup(_id, group);
  }

private:

  SharedPtr<LightManager> _light_manager;

  geom::Location _location;
  geom::Rotation _rotation;

  LightId _id;

};

} // namespace client
} // namespace carla