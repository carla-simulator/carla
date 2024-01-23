// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/Color.h"
#include "carla/rpc/LightState.h"

namespace carla {
namespace client {

using Color = sensor::data::Color;
using LightId = uint32_t;

struct LightState {

  using LightGroup = rpc::LightState::LightGroup;

  LightState() {}

  LightState(
      float intensity,
      Color color,
      LightGroup group,
      bool active)
  : _intensity(intensity),
    _color(color),
    _group(group),
    _active(active){}

  void Reset () {
    _intensity = 0.0f;
    _color.r = 0;
    _color.g = 0;
    _color.b = 0;
    _group = LightGroup::None;
    _active = false;
  }

  float _intensity = 0.0f;
  Color _color;
  LightGroup _group = LightGroup::None;
  bool _active = false;
};

} // namespace client
} // namespace carla
