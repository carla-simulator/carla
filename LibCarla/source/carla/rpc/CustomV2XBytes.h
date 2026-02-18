// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <array>

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

class CustomV2XBytes
{
  public:
    uint8_t max_data_size() const {
        return static_cast<uint8_t>(bytes.max_size());
    }
    uint8_t data_size{0u};
    std::array<unsigned char, 100> bytes;
    MSGPACK_DEFINE_ARRAY(data_size, bytes);
};

}     // namespace rpc
} // namespace carla
