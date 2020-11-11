// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <cstdint>
#include <string>

namespace carla {
namespace rpc {

  class BenchmarkQueryValue {
  public:

    std::string query;

    float value;

    MSGPACK_DEFINE_ARRAY(query, value);
  };

} // namespace rpc
} // namespace carla
