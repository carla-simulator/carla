// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"

#include "carla/client/LightState.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"

namespace carla {
namespace client {

class LightManager;

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

  Color GetColor() const;

  LightId GetId() const;

  float GetIntensity() const;

  const geom::Location GetLocation() const;

  const geom::Rotation GetRotation() const;

  LightState GetState() const;

  LightGroup GetLightGroup() const;

  void SetColor(Color color);

  void SetIntensity(float intensity);

  void SetState(const LightState& state);

  void SetLightGroup(LightGroup group);

private:

  SharedPtr<LightManager> _light_manager;

  geom::Location _location;
  geom::Rotation _rotation;

  LightId _id;

};

} // namespace client
} // namespace carla