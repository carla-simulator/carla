// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"

#include <cstdint>

namespace carla {
namespace streaming {
namespace detail {

  using stream_id_type = uint32_t;

  using message_size_type = Buffer::size_type;

} // namespace detail
} // namespace streaming
} // namespace carla
