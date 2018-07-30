// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/streaming/detail/Types.h"

#include <boost/asio/buffer.hpp>

#include <array>
#include <exception>
#include <limits>
#include <memory>
#include <string>

namespace carla {
namespace streaming {

  /// A message owns a buffer with raw data.
  class Message {

    // =========================================================================
    // -- Typedefs -------------------------------------------------------------
    // =========================================================================

  public:

    using value_type = unsigned char;

    using size_type = detail::message_size_type;

    // =========================================================================
    // -- Construction and assignment ------------------------------------------
    // =========================================================================

  public:

    Message() = default;

    explicit Message(size_type size)
      : _size(size),
        _data(std::make_unique<value_type[]>(_size)) {}

    explicit Message(uint64_t size)
      : Message([size]() {
#ifndef LIBCARLA_INCLUDED_FROM_UE4
          /// @todo What to do with exceptions?
          if (size > std::numeric_limits<size_type>::max()) {
            throw std::invalid_argument("message size too big");
          }
#endif // LIBCARLA_INCLUDED_FROM_UE4
          return static_cast<size_type>(size);
        } ()) {}

    template <typename ConstBufferSequence>
    explicit Message(ConstBufferSequence source)
      : Message(boost::asio::buffer_size(source)) {
      DEBUG_ONLY(auto bytes_copied = )
      boost::asio::buffer_copy(buffer(), source);
      DEBUG_ASSERT(bytes_copied == size());
    }

    Message(const Message &) = delete;
    Message &operator=(const Message &) = delete;

    Message(Message &&rhs) noexcept
      : _size(rhs._size),
        _data(std::move(rhs._data)) {
      rhs._size = 0u;
    }

    Message &operator=(Message &&rhs) noexcept {
      _size = rhs._size;
      _data = std::move(rhs._data);
      rhs._size = 0u;
      return *this;
    }

    // =========================================================================
    // -- Data access ----------------------------------------------------------
    // =========================================================================

  public:

    bool empty() const {
      return _size == 0u;
    }

    size_type size() const {
      return _size;
    }

    const value_type *data() const {
      return _data.get();
    }

    value_type *data() {
      return _data.get();
    }

    // =========================================================================
    // -- Conversions ----------------------------------------------------------
    // =========================================================================

  public:

    boost::asio::const_buffer buffer() const {
      return {data(), size()};
    }

    boost::asio::mutable_buffer buffer() {
      return {data(), size()};
    }

    std::array<boost::asio::const_buffer, 2u> encode() const {
      DEBUG_ASSERT(!empty());
      return {boost::asio::buffer(&_size, sizeof(_size)), buffer()};
    }

    // =========================================================================
    // -- Private members ------------------------------------------------------
    // =========================================================================

  private:

    size_type _size = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

} // namespace streaming
} // namespace carla
