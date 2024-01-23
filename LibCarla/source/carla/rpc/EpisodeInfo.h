// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  /// @todo Rename, does not represent the episode info anymore.
  class EpisodeInfo {
  public:

    using id_type = uint64_t;

    id_type id;

    streaming::Token token;

    MSGPACK_DEFINE_ARRAY(id, token);
  };

} // namespace rpc
} // namespace carla
