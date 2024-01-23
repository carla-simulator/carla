// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"

#include "carla/client/LightState.h"
#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/rpc/LightState.h"

namespace carla {
namespace client {

class LightManager;

class Light {

  using LightGroup = rpc::LightState::LightGroup;

public:

  Light() {}

  Color GetColor() const;

  LightId GetId() const {
    return _id;
  }

  float GetIntensity() const;

  const geom::Location GetLocation() const {
    return _location;
  }

  LightGroup GetLightGroup() const;

  LightState GetLightState() const;

  bool IsOn() const;

  bool IsOff() const;

  void SetColor(Color color);

  void SetIntensity(float intensity);

  void SetLightGroup(LightGroup group);

  void SetLightState(const LightState& state);

  void TurnOn();

  void TurnOff();

private:

  friend class LightManager;

  Light(WeakPtr<LightManager> light_manager,
    geom::Location location,
    LightId id)
  : _light_manager(light_manager),
    _location (location),
    _id (id) {}

  WeakPtr<LightManager> _light_manager;
  geom::Location _location;

  LightId _id;

};

} // namespace client
} // namespace carla
