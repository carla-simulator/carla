// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/Logging.h"

#include <boost/asio/buffer.hpp>

#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Containers/Array.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {

  class BufferPool;

  /// A piece of raw data.
  ///
  /// Note that if more capacity is needed, a new memory block is allocated and
  /// the old one is deleted. This means that by default the buffer can only
  /// grow. To release the memory use `clear` or `pop`.
  ///
  /// This is a move-only type, meant to be cheap to pass by value. If the
  /// buffer is retrieved from a BufferPool, the memory is automatically pushed
  /// back to the pool on destruction.
  ///
  /// @warning Creating a buffer bigger than max_size() is undefined.
  class Buffer {

    // =========================================================================
    /// @name Member types
    // =========================================================================
    /// @{

  public:

    using value_type = unsigned char;

    using size_type = uint32_t;

    using iterator = value_type *;

    using const_iterator = const value_type *;

    /// @}
    // =========================================================================
    /// @name Construction and destruction
    // =========================================================================
    /// @{

  public:

    /// Create an empty buffer.
    Buffer() = default;

    /// Create a buffer with @a size bytes allocated.
    explicit Buffer(size_type size)
      : _size(size),
        _capacity(size),
        _data(std::make_unique<value_type[]>(size)) {}

    /// @copydoc Buffer(size_type)
    explicit Buffer(uint64_t size)
      : Buffer([size]() {
          if (size > max_size()) {
            throw_exception(std::invalid_argument("message size too big"));
          }
          return static_cast<size_type>(size);
        } ()) {}

    /// Copy @a source into this buffer. Allocates the necessary memory.
    template <typename T>
    explicit Buffer(const T &source) {
      copy_from(source);
    }

    explicit Buffer(const value_type *data, size_type size) {
      copy_from(data, size);
    }

    /// @copydoc Buffer(size_type)
    explicit Buffer(const value_type *data, uint64_t size)
      : Buffer(data, [size]() {
          if (size > max_size()) {
            throw_exception(std::invalid_argument("message size too big"));
          }
          return static_cast<size_type>(size);
        } ()) {}

    Buffer(const Buffer &) = delete;

    Buffer(Buffer &&rhs) noexcept
      : _parent_pool(std::move(rhs._parent_pool)),
        _size(rhs._size),
        _capacity(rhs._capacity),
        _data(rhs.pop()) {}

    ~Buffer() {
      if (_capacity > 0u) {
        ReuseThisBuffer();
      }
    }

    /// @}
    // =========================================================================
    /// @name Assignment
    // =========================================================================
    /// @{

  public:

    Buffer &operator=(const Buffer &) = delete;

    Buffer &operator=(Buffer &&rhs) noexcept {
      _parent_pool = std::move(rhs._parent_pool);
      _size = rhs._size;
      _capacity = rhs._capacity;
      _data = rhs.pop();
      return *this;
    }

    /// @}
    // =========================================================================
    /// @name Data access
    // =========================================================================
    /// @{

  public:

    /// Access the byte at position @a i.
    const value_type &operator[](size_t i) const {
      return _data[i];
    }

    /// Access the byte at position @a i.
    value_type &operator[](size_t i) {
      return _data[i];
    }

    /// Direct access to the allocated memory or nullptr if no memory is
    /// allocated.
    const value_type *data() const noexcept {
      return _data.get();
    }

    /// Direct access to the allocated memory or nullptr if no memory is
    /// allocated.
    value_type *data() noexcept {
      return _data.get();
    }

    /// Make a boost::asio::buffer from this buffer.
    ///
    /// @warning Boost.Asio buffers do not own the data, it's up to the caller
    /// to not delete the memory that this buffer holds until the asio buffer is
    /// no longer used.
    boost::asio::const_buffer cbuffer() const noexcept {
      return {data(), size()};
    }

    /// @copydoc cbuffer()
    boost::asio::const_buffer buffer() const noexcept {
      return cbuffer();
    }

    /// @copydoc cbuffer()
    boost::asio::mutable_buffer buffer() noexcept {
      return {data(), size()};
    }

    /// @}
    // =========================================================================
    /// @name Capacity
    // =========================================================================
    /// @{

  public:

    bool empty() const noexcept {
      return _size == 0u;
    }

    size_type size() const noexcept {
      return _size;
    }

    static constexpr size_type max_size() noexcept {
      return (std::numeric_limits<size_type>::max)();
    }

    size_type capacity() const noexcept {
      return _capacity;
    }

    /// @}
    // =========================================================================
    /// @name Iterators
    // =========================================================================
    /// @{

  public:

    const_iterator cbegin() const noexcept {
      return _data.get();
    }

    const_iterator begin() const noexcept {
      return cbegin();
    }

    iterator begin() noexcept {
      return _data.get();
    }

    const_iterator cend() const noexcept {
      return cbegin() + size();
    }

    const_iterator end() const noexcept {
      return cend();
    }

    iterator end() noexcept {
      return begin() + size();
    }

    /// @}
    // =========================================================================
    /// @name Modifiers
    // =========================================================================
    /// @{

  public:

    /// Reset the size of this buffer. If the capacity is not enough, the
    /// current memory is discarded and a new block of size @a size is
    /// allocated.
    void reset(size_type size) {
      if (_capacity < size) {
        log_debug("allocating buffer of", size, "bytes");
        _data = std::make_unique<value_type[]>(size);
        _capacity = size;
      }
      _size = size;
    }

    /// @copydoc reset(size_type)
    void reset(uint64_t size) {
      if (size > max_size()) {
        throw_exception(std::invalid_argument("message size too big"));
      }
      reset(static_cast<size_type>(size));
    }

    /// Resize the buffer, a new block of size @a size is
    /// allocated if the capacity is not enough and the data is copied.
    void resize(uint64_t size) {
      if(_capacity < size) {
        std::unique_ptr<value_type[]> data = std::move(_data);
        uint64_t old_size = size;
        reset(size);
        copy_from(data.get(), static_cast<size_type>(old_size));
      }
      _size = static_cast<size_type>(size);
    }

    /// Release the contents of this buffer and set its size and capacity to
    /// zero.
    std::unique_ptr<value_type[]> pop() noexcept {
      _size = 0u;
      _capacity = 0u;
      return std::move(_data);
    }

    /// Clear the contents of this buffer and set its size and capacity to zero.
    /// Deletes allocated memory.
    void clear() noexcept {
      pop();
    }

    /// @}
    // =========================================================================
    /// @name copy_from
    // =========================================================================
    /// @{

  public:

    /// Copy @a source into this buffer. Allocates memory if necessary.
    template <typename T>
    void copy_from(const T &source) {
      copy_from(0u, source);
    }

    /// Copy @a size bytes of the memory pointed by @a data into this buffer.
    /// Allocates memory if necessary.
    void copy_from(const value_type *data, size_type size) {
      copy_from(0u, data, size);
    }

    /// Copy @a source into this buffer leaving at the front an offset of @a
    /// offset bytes uninitialized. Allocates memory if necessary.
    void copy_from(size_type offset, const Buffer &rhs) {
      copy_from(offset, rhs.buffer());
    }

    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    typename std::enable_if<boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(size_type offset, const T &source) {
      reset(boost::asio::buffer_size(source) + offset);
      DEBUG_ASSERT(boost::asio::buffer_size(source) == size() - offset);
      DEBUG_ONLY(auto bytes_copied = )
      boost::asio::buffer_copy(buffer() + offset, source);
      DEBUG_ASSERT(bytes_copied == size() - offset);
    }

    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    typename std::enable_if<!boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(size_type offset, const T &source) {
      copy_from(offset, boost::asio::buffer(source));
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    /// @copydoc copy_from(size_type, const Buffer &)
    template <typename T>
    void copy_from(size_type offset, const TArray<T> &source) {
      copy_from(
          offset,
          reinterpret_cast<const value_type *>(source.GetData()),
          sizeof(T) * source.Num());
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4

    /// Copy @a size bytes of the memory pointed by @a data into this buffer,
    /// leaving at the front an offset of @a offset bytes uninitialized.
    /// Allocates memory if necessary.
    void copy_from(size_type offset, const value_type *data, size_type size) {
      copy_from(offset, boost::asio::buffer(data, size));
    }

    /// @}

  private:

    void ReuseThisBuffer();

    friend class BufferPool;

    std::weak_ptr<BufferPool> _parent_pool;

    size_type _size = 0u;

    size_type _capacity = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

} // namespace carla
