// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

/// @file BufferView.h
/// @brief Immutable view over a buffer's data
///
/// Provides read-only access to buffer data without ownership.

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

  /// @brief Creating a constant view from an existing buffer
  ///
  /// BufferView provides read-only access to buffer data without taking
  /// ownership. The underlying buffer is moved into the view and cannot be
  /// modified through the view.
  ///
  /// @example
  /// ```cpp
  /// auto view = BufferView::CreateFrom(std::move(buffer));
  /// const auto* data = view->data();
  /// const auto size = view->size();
  /// ```
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

    /// @brief Create a BufferView from a buffer.
    /// @param buffer Buffer to view (moved into view).
    /// @return Shared pointer to BufferView.
    static std::shared_ptr<BufferView> CreateFrom(Buffer &&buffer) {
      return std::shared_ptr<BufferView>(new BufferView(std::move(buffer)));
    }

  private:

    /// @brief Private constructor from buffer.
    /// @param rhs Buffer to move into view.
    BufferView(Buffer &&rhs) noexcept
      : _buffer(std::move(rhs)) {}

    /// @}
    // =========================================================================
    /// @name Data access
    // =========================================================================
    /// @{

  public:

    /// @brief Access the byte at position @a i.
    /// @param i Index to access.
    /// @return Reference to byte at index.
    const value_type &operator[](size_t i) const {
      return _buffer.data()[i];
    }

    /// @brief Direct access to the allocated memory.
    /// @return Pointer to data or nullptr if empty.
    const value_type *data() const noexcept {
      return _buffer.data();
    }

    /// @brief Make a boost::asio::buffer from this buffer.
    /// @return Boost ASIO const buffer.
    /// @warning Boost.Asio buffers do not own the data. Caller must ensure
    ///          the memory is not deleted while the asio buffer is in use.
    boost::asio::const_buffer cbuffer() const noexcept {
      return {_buffer.data(), _buffer.size()};
    }

    /// @brief Make a boost::asio::buffer from this buffer.
    /// @return Boost ASIO const buffer.
    /// @copydetails cbuffer()
    boost::asio::const_buffer buffer() const noexcept {
      return cbuffer();
    }

    /// @}
    // =========================================================================
    /// @name Capacity
    // =========================================================================
    /// @{

  public:

    /// @brief Check if buffer is empty.
    /// @return true if size is 0.
    bool empty() const noexcept {
      return _buffer.size() == 0u;
    }

    /// @brief Get the number of bytes in the buffer.
    /// @return Size in bytes.
    size_type size() const noexcept {
      return _buffer.size();
    }

    /// @brief Get the maximum possible size.
    /// @return Maximum size for size_type.
    static constexpr size_type max_size() noexcept {
      return (std::numeric_limits<size_type>::max)();
    }

    /// @brief Get the capacity of the underlying buffer.
    /// @return Capacity in bytes.
    size_type capacity() const noexcept {
      return _buffer.capacity();
    }

    /// @}
    // =========================================================================
    /// @name Iterators
    // =========================================================================
    /// @{

  public:

    /// @brief Get const iterator to beginning.
    /// @return Iterator to first element.
    const_iterator cbegin() const noexcept {
      return _buffer.data();
    }

    /// @brief Get const iterator to beginning.
    /// @return Iterator to first element.
    const_iterator begin() const noexcept {
      return _buffer.cbegin();
    }

    /// @brief Get const iterator to end.
    /// @return Iterator past last element.
    const_iterator cend() const noexcept {
      return _buffer.cbegin() + _buffer.size();
    }

    /// @brief Get const iterator to end.
    /// @return Iterator past last element.
    const_iterator end() const noexcept {
      return _buffer.cend();
    }

  private:

    const Buffer _buffer;  ///< Underlying buffer (immutable)
  };

  using SharedBufferView = std::shared_ptr<BufferView>;

} // namespace carla
