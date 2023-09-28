// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/StreamStateBase.h"

#include "carla/BufferPool.h"

namespace carla {
namespace streaming {
namespace detail {

  StreamStateBase::StreamStateBase(const token_type &token)
    : _token(token),
      _buffer_pool(std::make_shared<BufferPool>()) {}

  StreamStateBase::~StreamStateBase() = default;

  Buffer StreamStateBase::MakeBuffer() {
    auto pool = _buffer_pool;
    return pool->Pop();
  }

} // namespace detail
} // namespace streaming
} // namespace carla
