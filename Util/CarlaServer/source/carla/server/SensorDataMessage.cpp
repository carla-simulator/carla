// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/server/SensorDataMessage.h"

#include "carla/Logging.h"
#include "carla/server/CarlaServerAPI.h"

namespace carla {
namespace server {

  void SensorDataMessage::Write(const carla_sensor_data &data) {
    // The buffer contains id + data-header + data.
    const uint32_t buffer_size =
        sizeof(uint32_t) +
        data.header_size +
        data.data_size;
    // The message is prepended by the size of the buffer.
    Reset(sizeof(uint32_t) + buffer_size);

    auto begin = _buffer.get();

    std::memcpy(begin, &buffer_size, sizeof(uint32_t));
    begin += sizeof(uint32_t);

    std::memcpy(begin, &data.id, sizeof(uint32_t));
    begin += sizeof(uint32_t);

    std::memcpy(begin, data.header, data.header_size);
    begin += data.header_size;

    std::memcpy(begin, data.data, data.data_size);
  }

  void SensorDataMessage::Reset(uint32_t count) {
    if (_capacity < count) {
      log_debug("allocating sensor buffer of", count, "bytes");
      _buffer = std::make_unique<unsigned char[]>(count);
      _capacity = count;
    }
    _size = count;
  }

} // namespace server
} // namespace carla
