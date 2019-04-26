// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ListView.h"
#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/NonCopyable.h"
#include "carla/streaming/detail/Types.h"

#include <boost/asio/buffer.hpp>

#include <array>
#include <exception>
#include <limits>
#include <memory>
#include <string>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  /// Serialization of a set of buffers to be sent over a TCP socket as a single
  /// message. Template paramenter @a MaxNumberOfBuffers imposes a compile-time
  /// limit on the maximum number of buffers that can be included in a single
  /// message.
  template <size_t MaxNumberOfBuffers>
  class MessageTmpl
    : public std::enable_shared_from_this<MessageTmpl<MaxNumberOfBuffers>>,
      private NonCopyable {
  public:

    static constexpr size_t max_size() {
      return MaxNumberOfBuffers;
    }

  private:

    MessageTmpl(size_t) {}

    template <typename... Buffers>
    MessageTmpl(size_t size, Buffer &&buffer, Buffers &&... buffers)
      : MessageTmpl(size, std::move(buffers)...) {
      ++_number_of_buffers;
      _total_size += buffer.size();
      _buffer_views[1u + size - _number_of_buffers] = buffer.cbuffer();
      _buffers[size - _number_of_buffers] = std::move(buffer);
    }

  public:

    template <typename... Buffers>
    MessageTmpl(Buffer &&buf, Buffers &&... buffers)
      : MessageTmpl(sizeof...(Buffers) + 1u, std::move(buf), std::move(buffers)...) {
      static_assert(sizeof...(Buffers) < max_size(), "Too many buffers!");
      _buffer_views[0u] = boost::asio::buffer(&_total_size, sizeof(_total_size));
    }

    /// Size in bytes of the message excluding the header.
    auto size() const noexcept {
      return _total_size;
    }

    bool empty() const noexcept {
      return size() == 0u;
    }

    auto GetBufferSequence() const {
      auto begin = _buffer_views.begin();
      return MakeListView(begin, begin + _number_of_buffers + 1u);
    }

  private:

    message_size_type _number_of_buffers = 0u;

    message_size_type _total_size = 0u;

    std::array<Buffer, MaxNumberOfBuffers> _buffers;

    std::array<boost::asio::const_buffer, MaxNumberOfBuffers + 1u> _buffer_views;
  };

  /// A TCP message containing a maximum of 2 buffers. This is optimized for a
  /// header and body sort of messages.
  using Message = MessageTmpl<2u>;

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
