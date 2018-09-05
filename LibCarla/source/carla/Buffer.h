// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Logging.h"

#include <boost/asio/buffer.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <type_traits>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include "Containers/Array.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {

  /// A piece of raw data.
  ///
  /// Creating a buffer bigger than max_size() is undefined.
  class Buffer {

    // =========================================================================
    // -- Typedefs -------------------------------------------------------------
    // =========================================================================

  public:

    using value_type = unsigned char;

    using size_type = uint32_t;

    using iterator = value_type *;

    using const_iterator = const value_type *;

    // =========================================================================
    // -- Construction and assignment ------------------------------------------
    // =========================================================================

  public:

    Buffer() = default;

    explicit Buffer(size_type size)
      : _size(size),
        _capacity(size),
        _data(std::make_unique<value_type[]>(size)) {}

    explicit Buffer(uint64_t size)
      : Buffer([size]() {
          DEBUG_ASSERT(size <= max_size());
          return static_cast<size_type>(size);
        } ()) {}

    template <typename T>
    explicit Buffer(const T &source) {
      copy_from(source);
    }

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&rhs) noexcept
      : _size(rhs._size),
        _capacity(rhs._capacity),
        _data(rhs.pop()) {}

    Buffer &operator=(Buffer &&rhs) noexcept {
      _size = rhs._size;
      _capacity = rhs._capacity;
      _data = rhs.pop();
      return *this;
    }

    // =========================================================================
    // -- Data access ----------------------------------------------------------
    // =========================================================================

  public:

    bool empty() const noexcept {
      return _size == 0u;
    }

    size_type size() const noexcept {
      return _size;
    }

    static constexpr size_type max_size() {
      return std::numeric_limits<size_type>::max();
    }

    size_type capacity() const noexcept {
      return _capacity;
    }

    const value_type &operator[](size_t i) const {
      return _data[i];
    }

    value_type &operator[](size_t i) {
      return _data[i];
    }

    const value_type *data() const noexcept {
      return _data.get();
    }

    value_type *data() noexcept {
      return _data.get();
    }

    // =========================================================================
    // -- Iterators ------------------------------------------------------------
    // =========================================================================

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

    // =========================================================================
    // -- Resizing -------------------------------------------------------------
    // =========================================================================

  public:

    void reset(size_type size) {
      if (_capacity < size) {
        log_debug("allocating sensor buffer of", size, "bytes");
        _data = std::make_unique<value_type[]>(size);
        _capacity = size;
      }
      _size = size;
    }

    void reset(uint64_t size) {
      DEBUG_ASSERT(size <= max_size());
      reset(static_cast<size_type>(size));
    }

    std::unique_ptr<value_type[]> pop() noexcept {
      _size = 0u;
      _capacity = 0u;
      return std::move(_data);
    }

    void clear() noexcept {
      pop();
    }

    // =========================================================================
    // -- Rewriting ------------------------------------------------------------
    // =========================================================================

  public:

    template <typename T>
    typename std::enable_if<boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(const T &source) {
      reset(boost::asio::buffer_size(source));
      DEBUG_ASSERT(boost::asio::buffer_size(source) == size());
      DEBUG_ONLY(auto bytes_copied = )
      boost::asio::buffer_copy(buffer(), source);
      DEBUG_ASSERT(bytes_copied == size());
    }

    template <typename T>
    typename std::enable_if<!boost::asio::is_const_buffer_sequence<T>::value>::type
    copy_from(const T &source) {
      copy_from(boost::asio::buffer(source));
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4
    template <typename T>
    void copy_from(const TArray<T> &source) {
      copy_from(boost::asio::buffer(source.GetData(), sizeof(T) * source.Num()));
    }
#endif // LIBCARLA_INCLUDED_FROM_UE4

    void copy_from(const Buffer &rhs) {
      copy_from(rhs.buffer());
    }

    // =========================================================================
    // -- Conversions ----------------------------------------------------------
    // =========================================================================

  public:

    boost::asio::const_buffer cbuffer() const {
      return {data(), size()};
    }

    boost::asio::const_buffer buffer() const {
      return cbuffer();
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

    size_type _capacity = 0u;

    std::unique_ptr<value_type[]> _data = nullptr;
  };

} // namespace carla
