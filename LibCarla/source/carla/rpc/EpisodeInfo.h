// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/streaming/Token.h"

#include <cstdint>
#include <string>

namespace carla {
namespace rpc {

  class EpisodeInfo {
  public:

    using id_type = uint32_t;

    id_type id;

    std::string map_name;

    streaming::Token token;

    MSGPACK_DEFINE_ARRAY(id, map_name, token);
  };

} // namespace rpc
} // namespace carla
