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

#include <unordered_map>

struct carla_sensor_data;
struct carla_sensor_definition;

namespace carla {
namespace server {

  /// Stores the data received from the sensors (asynchronously) to be sent next
  /// on next tick.
  ///
  /// Each sensor has a double-buffer for one producer and one consumer per
  /// sensor. Several threads can simultaneously write as long as they write to
  /// different buffers, i.e. each sensor can have its own producer and consumer
  /// threads.
  class SensorDataInbox : private NonCopyable {
  public:

    /// We need to initialize the map before hand so it remains constant and
    /// doesn't need a lock.
    ///
    /// @warning This function is not thread-safe.
    void RegisterSensor(const carla_sensor_definition &sensor) {
      _buffers[sensor.id];
    }

    void Write(const carla_sensor_data &data) {
      auto message = _buffers.at(data.id).MakeWriter();
      message->Write(data);
    }

    /// Tries to acquire a reader on the buffer of the given sensor. See
    /// DoubleBuffer.
    auto TryMakeReader(uint32_t sensor_id) {
      return _buffers.at(sensor_id).TryMakeReader(timeout_t::milliseconds(0u));
    }

  private:

    using DataBuffer = DoubleBuffer<SensorDataMessage>;

    std::unordered_map<uint32_t, DataBuffer> _buffers;
  };

} // namespace server
} // namespace carla
