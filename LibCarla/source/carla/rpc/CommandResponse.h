// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorId.h"
#include "carla/rpc/Response.h"

namespace carla {
namespace rpc {

  using CommandResponse = Response<ActorId>;

} // namespace rpc
} // namespace carla
