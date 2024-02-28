// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/sensor/SensorData.h"
#include "carla/sensor/Deserializer.h"

#include <exception>
#include <iterator>
#include <type_traits>

namespace carla {
namespace sensor {
namespace data {

  template <typename T>
  class Array;

  /// Base class for all the sensor data consisting of an array of items.
  /// This class provides const access to the data, which is the normal use-case
  /// If non-const access is required, use the derived Array class which casts away the constiness
  template <typename T>
  class ArrayConst : public SensorData {
  public:

    using value_type = T;
    using const_iterator = typename std::add_const<value_type>::type *;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using const_pointer = typename std::add_const<value_type>::type *;
    using const_reference = typename std::add_const<value_type>::type &;

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

    bool empty() const {
      return begin() == end();
    }

    size_type size() const {
      DEBUG_ASSERT(std::distance(begin(), end()) >= 0);
      return static_cast<size_type>(std::distance(begin(), end()));
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

  protected:

    template <typename FuncT>
    explicit ArrayConst(RawData DESERIALIZE_DECL_DATA(data), FuncT get_offset)
      : SensorData(data),
        _data(DESERIALIZE_MOVE_DATA(data)),
        _offset(get_offset(_data)) {
      DEBUG_ASSERT(_data.size() >= _offset);
      DEBUG_ASSERT((_data.size() - _offset) % sizeof(T) == 0u);
      DEBUG_ASSERT(begin() <= end());
    }

    explicit ArrayConst(size_t offset, RawData DESERIALIZE_DECL_DATA(data))
      : ArrayConst(DESERIALIZE_MOVE_DATA(data), [offset](RawData const &) { return offset; }) {}

    const RawData &GetRawData() const {
      return _data;
    }

  private:
    template <typename U>
    friend class Array; 

#if defined(CARLA_SERVER_BUILD)
    const RawData &_data;
#else
    RawData _data;
#endif

    const size_t _offset;
  };

} // namespace data
} // namespace sensor
} // namespace carla
