// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Transform.h"

#include <string>
#include <utility>

namespace carla {
namespace rpc {

  using BoneTransformDataIn = std::pair<std::string, geom::Transform>;

} // namespace rpc
} // namespace carla
