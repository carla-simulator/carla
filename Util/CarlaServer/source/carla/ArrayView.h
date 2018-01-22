// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <type_traits>

namespace carla {

namespace detail {

  /// A view over an C-style array. Encapsulates the array and its size, but
  /// does NOT own the data.
  ///
  /// Do not use ArrayView directly, use mutable_array_view or const_array_view.
  template <typename T>
  class ArrayView {
  public:

    using value_type = T;
    using mutable_value_type = std::remove_const_t<T>;
    using const_value_type = const std::remove_const_t<T>;
    using size_type = std::size_t;
    using iterator = value_type*;
    using const_iterator = const_value_type*;

    explicit ArrayView(T *data, size_type size)
        : _data(data),
          _size(size) {}

    ArrayView(ArrayView<mutable_value_type> &rhs)
        : _data(rhs.data()),
          _size(rhs.size()) {}

    ArrayView(const ArrayView<const_value_type> &rhs)
        : _data(rhs.data()),
          _size(rhs.size()) {}

    bool empty() const {
      return _size == 0u;
    }

    size_type size() const {
      return _size;
    }

    iterator begin() {
      return _data;
    }

    iterator end() {
      return _data + _size;
    }

    const_iterator begin() const {
      return _data;
    }

    const_iterator end() const {
      return _data + _size;
    }

    value_type *data() {
      return _data;
    }

    const_value_type *data() const {
      return _data;
    }

    value_type &operator[](size_type i) {
      return _data[i];
    }

    const_value_type &operator[](size_type i) const {
      return _data[i];
    }

  private:

    value_type *_data;

    size_type _size;
  };

} // namespace detail

  template <typename T>
  using mutable_array_view = detail::ArrayView<std::remove_const_t<T>>;

  template <typename T>
  using const_array_view = detail::ArrayView<const std::remove_const_t<T>>;

namespace array_view {

  template <typename T, typename V = mutable_array_view<T>>
  static inline auto make_mutable(T *data, typename V::size_type size) {
    return V(data, size);
  }

  template <typename T, typename V = const_array_view<T>>
  static inline auto make_const(const T *data, typename V::size_type size) {
    return V(data, size);
  }

} // namespace array_view

} // namespace carla
