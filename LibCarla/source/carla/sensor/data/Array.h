// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/ArrayConst.h"

namespace carla {
namespace sensor {
namespace data {

  /// Base class for all the sensor data consisting of an array of items.
  template <typename T>
  class Array : public ArrayConst<T> {
  private:
    using ArrayConst<T>::_data;
    using ArrayConst<T>::_offset;

  public:

    using value_type = typename ArrayConst<T>::value_type;
    using iterator = value_type *;
    using const_iterator = typename ArrayConst<T>::const_iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = typename ArrayConst<T>::const_reverse_iterator;
    using size_type = typename ArrayConst<T>::size_type;
    using pointer = value_type *;
    using const_pointer = typename ArrayConst<T>::const_pointer;
    using reference = value_type &;
    using const_reference = typename ArrayConst<T>::const_reference;

    const_iterator cbegin() const {
      return reinterpret_cast<const_iterator>(_data.begin() + _offset);
    }

    const_iterator begin() const {
      return cbegin();
    }

    const_iterator cend() const {
      return reinterpret_cast<const_iterator>(_data.end());
    }

    const_iterator end() const {
      return cend();
    }
    const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator rbegin() const {
      return crbegin();
    }

    const_reverse_iterator crend() const {
      return const_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const {
      return crend();
    }

    iterator begin() {
      return reinterpret_cast<iterator>(const_cast<RawData*>(&_data)->begin() + _offset);
    }

    iterator end() {
      return reinterpret_cast<iterator>(const_cast<RawData*>(&_data)->end());
    }

    reverse_iterator rbegin() {
      return reverse_iterator(begin());
    }

    reverse_iterator rend() {
      return reverse_iterator(end());
    }

    bool empty() const {
      return begin() == end();
    }

    value_type *data() {
      return begin();
    }

    const value_type *data() const {
      return begin();
    }

    const_reference operator[](size_type pos) const {
      return data()[pos];
    }

    const_reference at(size_type pos) const {
      if (!(pos < size())) {
        throw_exception(std::out_of_range("Array index out of range"));
      }
      return operator[](pos);
    }

    reference operator[](size_type pos) {
      return data()[pos];
    }

    reference at(size_type pos) {
      if (!(pos < size())) {
        throw_exception(std::out_of_range("Array index out of range"));
      }
      return operator[](pos);
    }

    using ArrayConst<T>::size;

  protected:

    template <typename FuncT>
    explicit Array(RawData DESERIALIZE_DECL_DATA(data), FuncT get_offset)
      : ArrayConst<T>(DESERIALIZE_MOVE_DATA(data), get_offset) {}
    
    explicit Array(size_t offset, RawData DESERIALIZE_DECL_DATA(data))
      : ArrayConst<T>(offset, DESERIALIZE_MOVE_DATA(data)) {}

    using ArrayConst<T>::GetRawData;

  };

} // namespace data
} // namespace sensor
} // namespace carla
