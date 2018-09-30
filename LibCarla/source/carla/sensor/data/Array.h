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
     return reinterpret_cast<iterator>(_message.begin() + _offset);
    }

    const_iterator cbegin() const {
     return reinterpret_cast<const_iterator>(_message.begin() + _offset);
    }

    const_iterator begin() const {
     return cbegin();
    }

    iterator end() {
     return reinterpret_cast<iterator>(_message.end());
    }

    const_iterator cend() const {
     return reinterpret_cast<const_iterator>(_message.end());
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

    explicit Array(size_t offset, DataMessage message)
      : SensorData(message),
        _message(std::move(message)) {
      SetOffset(offset);
    }

    explicit Array(DataMessage message)
      : Array(0u, std::move(message)) {}

    void SetOffset(size_t offset) {
      DEBUG_ASSERT(_message.size() >= _offset);
      DEBUG_ASSERT((_message.size() - _offset) % sizeof(T) == 0u);
      _offset = offset;
    }

    const DataMessage &GetMessage() const {
      return _message;
    }

  private:

    size_t _offset;

    DataMessage _message;
  };

} // namespace data
} // namespace sensor
} // namespace carla
