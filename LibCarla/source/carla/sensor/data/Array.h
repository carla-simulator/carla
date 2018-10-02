// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/SensorData.h"

#include <type_traits>
#include <iterator>

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
    using size_type = typename std::iterator_traits<iterator>::difference_type;
    using pointer = typename std::iterator_traits<iterator>::pointer;
    using reference = typename std::iterator_traits<iterator>::reference;

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

    bool empty() const {
      return begin() == end();
    }

    size_type size() const {
     return std::distance(begin(), end());
    }

    value_type *data() {
      return begin();
    }

    const value_type *data() const {
      return begin();
    }

    reference operator[](size_type i) {
      return data()[i];
    }

    const reference operator[](size_type i) const {
      return data()[i];
    }

  protected:

    explicit Array(size_t offset, RawData data)
      : SensorData(data),
        _data(std::move(data)) {
      SetOffset(offset);
    }

    explicit Array(RawData data)
      : Array(0u, std::move(data)) {}

    void SetOffset(size_t offset) {
      DEBUG_ASSERT(_data.size() >= _offset);
      DEBUG_ASSERT((_data.size() - _offset) % sizeof(T) == 0u);
      _offset = offset;
    }

    const RawData &GetRawData() const {
      return _data;
    }

  private:

    size_t _offset;

    RawData _data;
  };

} // namespace data
} // namespace sensor
} // namespace carla
