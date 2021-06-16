// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace rpc {

  enum class ActorState : uint8_t {
    Invalid,
    Active,
    Dormant,
    PendingKill,
  };

} // namespace rpc
} // namespace carla
