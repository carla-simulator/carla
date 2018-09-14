// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/StreamState.h"

#include "carla/BufferPool.h"

namespace carla {
namespace streaming {
namespace detail {

  StreamState::StreamState(const token_type &token)
    : _token(token),
      _buffer_pool(std::make_shared<BufferPool>()) {}

  StreamState::~StreamState() = default;

  Buffer StreamState::MakeBuffer() {
    return _buffer_pool->Pop();
  }

} // namespace detail
} // namespace streaming
} // namespace carla
