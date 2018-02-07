// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/NonCopyable.h"
#include "carla/server/DoubleBuffer.h"
#include "carla/server/SensorDataMessage.h"

#include <type_traits>
#include <unordered_map>
#include <vector>

struct carla_sensor_data;
struct carla_sensor_definition;

namespace carla {
namespace server {

namespace detail {

  template <typename IT>
  class value_iterator {
  public:

    value_iterator(IT original_iterator) : _it(original_iterator) {}

    value_iterator &operator++() {
      ++_it;
      return *this;
    }

    auto &operator*() {
      return _it->second;
    }

    bool operator!=(value_iterator rhs) {
      return _it != rhs._it;
    }

  private:

    IT _it;
  };

} // detail

  /// Stores the data received from the sensors (asynchronously) to be sent next
  /// on next tick.
  ///
  /// Each sensor has a double-buffer for one producer and one consumer per
  /// sensor. Several threads can simultaneously write as long as they write to
  /// different buffers, i.e. each sensor can have its own producer and consumer
  /// threads.
  class SensorDataInbox : private NonCopyable {

    using DataBuffer = DoubleBuffer<SensorDataMessage>;

    using Map = std::unordered_map<uint32_t, DataBuffer>;

  public:

    using Sensors = std::vector<carla_sensor_definition>;

    using buffer_iterator = detail::value_iterator<Map::iterator>;

    explicit SensorDataInbox(const Sensors &sensors) {
      // We need to initialize the map before hand so it remains constant and
      // doesn't need a lock.
      for (auto &sensor : sensors)
        _buffers[sensor.id];
    }

    void Write(const carla_sensor_data &data) {
      auto writer = _buffers.at(data.id).MakeWriter();
      writer->Write(data);
    }

    /// Tries to acquire a reader on the buffer of the given sensor. See
    /// DoubleBuffer.
    auto TryMakeReader(uint32_t sensor_id) {
      return _buffers.at(sensor_id).TryMakeReader();
    }

    buffer_iterator begin() {
      return _buffers.begin();
    }

    buffer_iterator end() {
      return _buffers.end();
    }

  private:

    Map _buffers;
  };

} // namespace server
} // namespace carla
