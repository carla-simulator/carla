// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace client {

  enum class GarbageCollectionPolicy {
    Disabled,
    Enabled,
    Inherit
  };

} // namespace client
} // namespace carla
