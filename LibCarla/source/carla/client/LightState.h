// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/rpc/Color.h"

namespace carla {
namespace client {

using Color = rpc::Color;
using LightId = uint32_t;

enum class LightGroup : uint8_t {
  None = 0,
  Vehicle,
  Street,
  Building,
  Other
};

struct LightState {

  void Reset () {
    _intensity = 0.0f;
    _color.r = 0;
    _color.g = 0;
    _color.b = 0;
    _group = LightGroup::None;
  }

  float _intensity = 0.0f;
  Color _color;
  LightGroup _group = LightGroup::None;
};

} // namespace client
} // namespace carla