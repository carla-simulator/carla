// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/sensor/SensorData.h"

#include <exception>
#include <iterator>
#include <type_traits>

namespace carla {
namespace sensor {
namespace data {

  /// Base class for all the sensor data consisting of an array of items.
  template <typename T>
  class Array : public SensorData {
  public:

    using value_type = T;
    using iterator = value_type *;
    using const_iterator = typename std::add_const<value_type>::type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using pointer = value_type *;
    using const_pointer = typename std::add_const<value_type>::type *;
    using reference = value_type &;
    using const_reference = typename std::add_const<value_type>::type &;

    iterator begin() {
      return reinterpret_cast<iterator>(_data.begin() + _offset);
    }

    const_iterator cbegin() const {
      return reinterpret_cast<const_iterator>(_data.begin() + _offset);
    }

    const_iterator begin() const {
      return cbegin();
    }

    iterator end() {
      return reinterpret_cast<iterator>(_data.end());
    }

    const_iterator cend() const {
      return reinterpret_cast<const_iterator>(_data.end());
    }

    const_iterator end() const {
      return cend();
    }

    reverse_iterator rbegin() {
      return reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cbegin());
    }

    const_reverse_iterator rbegin() const {
      return crbegin();
    }

    reverse_iterator rend() {
      return reverse_iterator(end());
    }

    const_reverse_iterator crend() const {
      return const_reverse_iterator(cend());
    }

    const_reverse_iterator rend() const {
      return crend();
    }

    bool empty() const {
      return begin() == end();
    }

    size_type size() const {
      DEBUG_ASSERT(std::distance(begin(), end()) >= 0);
      return static_cast<size_type>(std::distance(begin(), end()));
    }

    value_type *data() {
      return begin();
    }

    const value_type *data() const {
      return begin();
    }

    reference operator[](size_type pos) {
      return data()[pos];
    }

    const_reference operator[](size_type pos) const {
      return data()[pos];
    }

    reference at(size_type pos) {
      if (!(pos < size())) {
        throw_exception(std::out_of_range("Array index out of range"));
      }
      return operator[](pos);
    }

    const_reference at(size_type pos) const {
      if (!(pos < size())) {
        throw_exception(std::out_of_range("Array index out of range"));
      }
      return operator[](pos);
    }

  protected:

    template <typename FuncT>
    explicit Array(RawData &&data, FuncT get_offset)
      : SensorData(data),
        _data(std::move(data)),
        _offset(get_offset(_data)) {
      DEBUG_ASSERT(_data.size() >= _offset);
      DEBUG_ASSERT((_data.size() - _offset) % sizeof(T) == 0u);
      DEBUG_ASSERT(begin() <= end());
    }

    explicit Array(size_t offset, RawData &&data)
      : Array(std::move(data), [offset](const RawData &) { return offset; }) {}

    const RawData &GetRawData() const {
      return _data;
    }

  private:

    RawData _data;

    const size_t _offset;
  };

} // namespace data
} // namespace sensor
} // namespace carla
