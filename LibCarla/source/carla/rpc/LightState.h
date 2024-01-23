// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/geom/Rotation.h"
#include "carla/rpc/Color.h"

namespace carla {
namespace rpc {

using LightId = uint32_t;

class LightState {
public:

  using flag_type = uint8_t;

  enum class LightGroup : flag_type {
    None = 0,
    Vehicle,
    Street,
    Building,
    Other
  };

  LightState() {}

  LightState(
      geom::Location location,
      float intensity,
      LightGroup group,
      Color color,
      bool active)
  : _location(location),
    _intensity(intensity),
    _group(static_cast<flag_type>(group)),
    _color(color),
    _active(active) {}

  geom::Location _location;
  float _intensity = 0.0f;
  LightId _id;
  flag_type _group = static_cast<flag_type>(LightGroup::None);
  Color _color;
  bool _active = false;

  MSGPACK_DEFINE_ARRAY(_id, _location, _intensity, _group, _color, _active);

};

} // namespace rpc
} // namespace carla
