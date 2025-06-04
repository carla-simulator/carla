// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"

#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <util/enable-ue4-macros.h>
#include "Containers/Array.h"
#include <util/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

#include <boost/asio/buffer.hpp>

namespace carla {

  class BufferPool;

  /// Creating a constant view from an existing buffer
  class BufferView : public std::enable_shared_from_this<BufferView> {

    // =========================================================================
    /// @name Member types
    // =========================================================================
    /// @{

  public:

    using value_type = unsigned char;
    using size_type = uint32_t;
    using const_iterator = const value_type *;

    /// @}
    // =========================================================================
    /// @name Construction and destruction
    // =========================================================================
    /// @{

  public:

    BufferView() = delete;
    BufferView(const BufferView &) = delete;

    static std::shared_ptr<BufferView> CreateFrom(Buffer &&buffer) {
      return std::shared_ptr<BufferView>(new BufferView(std::move(buffer)));
    }

  private:

    BufferView(Buffer &&rhs) noexcept
      : _buffer(std::move(rhs)) {}

    /// @}
    // =========================================================================
    /// @name Data access
    // =========================================================================
    /// @{

  public:

    /// Access the byte at position @a i.
    const value_type &operator[](size_t i) const {
      return _buffer.data()[i];
    }

    /// Direct access to the allocated memory or nullptr if no memory is
    /// allocated.
    const value_type *data() const noexcept {
      return _buffer.data();
    }

    /// Make a boost::asio::buffer from this buffer.
    ///
    /// @warning Boost.Asio buffers do not own the data, it's up to the caller
    /// to not delete the memory that this buffer holds until the asio buffer is
    /// no longer used.
    boost::asio::const_buffer cbuffer() const noexcept {
      return {_buffer.data(), _buffer.size()};
    }

    /// @copydoc cbuffer()
    boost::asio::const_buffer buffer() const noexcept {
      return cbuffer();
    }

    /// @}
    // =========================================================================
    /// @name Capacity
    // =========================================================================
    /// @{

  public:

    bool empty() const noexcept {
      return _buffer.size() == 0u;
    }

    size_type size() const noexcept {
      return _buffer.size();
    }

    static constexpr size_type max_size() noexcept {
      return (std::numeric_limits<size_type>::max)();
    }

    size_type capacity() const noexcept {
      return _buffer.capacity();
    }

    /// @}
    // =========================================================================
    /// @name Iterators
    // =========================================================================
    /// @{

  public:

    const_iterator cbegin() const noexcept {
      return _buffer.data();
    }

    const_iterator begin() const noexcept {
      return _buffer.cbegin();
    }

    const_iterator cend() const noexcept {
      return _buffer.cbegin() + _buffer.size();
    }

    const_iterator end() const noexcept {
      return _buffer.cend();
    }

  private:

    const Buffer _buffer;
  };

  using SharedBufferView = std::shared_ptr<BufferView>;

} // namespace carla
