// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include "carla/BufferPool.h"

namespace carla {
namespace sensor {
namespace s11n {

  static_assert(
      SensorHeaderSerializer::header_offset == 3u * 8u + 6u * 4u,
      "Header size missmatch");

  static Buffer PopBufferFromPool() {
    static auto pool = std::make_shared<BufferPool>();
    return pool->Pop();
  }

  Buffer SensorHeaderSerializer::Serialize(
      const uint64_t index,
      const uint64_t frame,
      double timestamp,
      const rpc::Transform transform) {
    Header h;
    h.sensor_type = index;
    h.frame = frame;
    h.timestamp = timestamp;
    h.sensor_transform = transform;
    auto buffer = PopBufferFromPool();
    buffer.copy_from(reinterpret_cast<const unsigned char *>(&h), sizeof(h));
    return buffer;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
