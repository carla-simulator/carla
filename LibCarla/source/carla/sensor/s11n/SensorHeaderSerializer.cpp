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

  static Buffer PopBufferFromPool() {
    static BufferPool pool;
    return pool.Pop();
  }

#pragma pack(push, 1)
    struct Header {
      uint64_t sensor_type;
      uint64_t frame_counter;
      float sensor_transform[6u];
    };
#pragma pack(pop)

  Buffer SensorHeaderSerializer::Serialize(
      const uint64_t index,
      const uint64_t frame,
      const rpc::Transform transform) {
    Header h;
    h.sensor_type = index;
    h.frame_counter = frame;
    h.sensor_transform[0u] = transform.location.x;
    h.sensor_transform[1u] = transform.location.y;
    h.sensor_transform[2u] = transform.location.z;
    h.sensor_transform[3u] = transform.rotation.pitch;
    h.sensor_transform[4u] = transform.rotation.yaw;
    h.sensor_transform[5u] = transform.rotation.roll;
    auto buffer = PopBufferFromPool();
    buffer.copy_from(reinterpret_cast<const unsigned char *>(&h), sizeof(h));
    return buffer;
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
