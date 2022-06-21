// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#include <array>

namespace carla {
namespace streaming {

  /// A token that uniquely identify a stream.
  class Token {
  public:

    std::array<unsigned char, 24u> data;

    MSGPACK_DEFINE_ARRAY(data);
  };

} // namespace streaming
} // namespace carla
