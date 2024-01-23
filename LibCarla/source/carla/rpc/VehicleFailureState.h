// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace rpc {

  enum class VehicleFailureState : uint8_t {
    None,
    Rollover,
    Engine,
    TirePuncture,
  };

} // namespace rpc
} // namespace carla
