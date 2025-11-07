// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/rpc/Transform.h"

namespace carla {
namespace sensor {
namespace s11n {

  /// Serializes the meta-information (header) sent with all the sensor data.
  class SensorHeaderSerializer {
  public:

#pragma pack(push, 1)
    struct Header {
      uint64_t sensor_type;
      uint64_t frame;
      double timestamp;
      rpc::Transform sensor_transform;
      rpc::Transform sensor_relative_transform;
      rpc::Quaternion sensor_relative_transform_quaternion;
    };
#pragma pack(pop)

    constexpr static auto header_offset = sizeof(Header);

    static Buffer Serialize(
        uint64_t index,
        uint64_t frame,
        double timestamp,
        const rpc::Transform &transform,
        const rpc::Transform &sensor_relative_transform,
        const rpc::Quaternion &sensor_relative_transform_quaternion
    );

    static const Header &Deserialize(const Buffer &message) {
      return *reinterpret_cast<const Header *>(message.data());
    }
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
