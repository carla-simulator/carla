// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/sensor/RawData.h"

#include <cstdint>
#include <cstring>

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

  /// Serializes the single-channel float32 distance images produced by the
  /// path-traced distance camera. The header is layout-identical to
  /// ImageSerializer::ImageHeader (every image serializer shares it, see
  /// data::ImageTmpl); only the pixel type differs.
  class DistanceImageSerializer {
  public:

#pragma pack(push, 1)
    struct ImageHeader {
      uint32_t width;
      uint32_t height;
      float fov_angle;
    };
#pragma pack(pop)

    constexpr static auto header_offset = sizeof(ImageHeader);

    static const ImageHeader &DeserializeHeader(const RawData &data) {
      return *reinterpret_cast<const ImageHeader *>(data.begin());
    }

    template <typename Sensor>
    static Buffer Serialize(const Sensor &sensor, Buffer &&bitmap);

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

  template <typename Sensor>
  inline Buffer DistanceImageSerializer::Serialize(const Sensor &sensor, Buffer &&bitmap) {
    DEBUG_ASSERT(bitmap.size() > sizeof(ImageHeader));
    ImageHeader header = {
        sensor.GetImageWidth(),
        sensor.GetImageHeight(),
        sensor.GetFOVAngle()
    };
    std::memcpy(bitmap.data(), reinterpret_cast<const void *>(&header), sizeof(header));
    return std::move(bitmap);
  }

} // namespace s11n
} // namespace sensor
} // namespace carla
