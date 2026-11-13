// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

namespace carla {
namespace ros2 {

// PointField datatype constants mirror the numeric constants in
// carla/ros2/types/msg/PointField.h (carla::ros2::msg::PointField::*). They are
// kept here as an enum class so this header does not pull the message types into
// the Build-Tests/ compile unit. Producers in publishers/Carla*Publisher.cpp map
// these values onto carla::ros2::msg::PointField; this enum mirrors the same
// numeric values so layout tests can verify field offsets / sizes.
enum class PointFieldDataType : std::uint8_t {
  Int8    = 1,
  UInt8   = 2,
  Int16   = 3,
  UInt16  = 4,
  Int32   = 5,
  UInt32  = 6,
  Float32 = 7,
  Float64 = 8,
};

struct PointFieldDescriptor {
  std::string_view name;
  std::uint32_t offset;
  PointFieldDataType datatype;
  std::uint32_t count;
};

// Lidar: 4 FLOAT32 fields. Point stride = 16 B = sizeof(sensor::data::LidarDetection).
inline constexpr std::array<PointFieldDescriptor, 4> kLidarFields = {{
    {"x",         0u,  PointFieldDataType::Float32, 1u},
    {"y",         4u,  PointFieldDataType::Float32, 1u},
    {"z",         8u,  PointFieldDataType::Float32, 1u},
    {"intensity", 12u, PointFieldDataType::Float32, 1u},
}};

// Extended lidar (Autoware PointXYZIRCAEDT, tier4 SetDataEx port): x/y/z
// FLOAT32, intensity + return_type UINT8, channel UINT16, azimuth/elevation/
// distance FLOAT32, time_stamp UINT32. Point stride = 32 B, matching both the
// tightly packed field layout and the naturally aligned LidarPointEx POD in
// CarlaLidarPublisher.cpp (the uint16 lands on offset 14, so no padding).
inline constexpr std::array<PointFieldDescriptor, 10> kLidarFieldsEx = {{
    {"x",           0u,  PointFieldDataType::Float32, 1u},
    {"y",           4u,  PointFieldDataType::Float32, 1u},
    {"z",           8u,  PointFieldDataType::Float32, 1u},
    {"intensity",   12u, PointFieldDataType::UInt8,   1u},
    {"return_type", 13u, PointFieldDataType::UInt8,   1u},
    {"channel",     14u, PointFieldDataType::UInt16,  1u},
    {"azimuth",     16u, PointFieldDataType::Float32, 1u},
    {"elevation",   20u, PointFieldDataType::Float32, 1u},
    {"distance",    24u, PointFieldDataType::Float32, 1u},
    {"time_stamp",  28u, PointFieldDataType::UInt32,  1u},
}};

inline constexpr double kLidarAzimuthPi = 3.14159265358979323846;

// How the extended layout's `azimuth` is measured, set per sensor through the
// `azimuth_direction`, `azimuth_range_start` and `azimuth_range_end`
// blueprint attributes. Angles are in the ROS frame (x forward, y left) and
// zero at +x. The default, counter-clockwise in [-pi, pi], is atan2(y, x).
// Autoware's LiDAR drivers instead report the rotation angle, clockwise in
// [0, 2*pi), which is CARLA's sweep direction.
struct LidarAzimuthRange {
  bool clockwise = false;
  // Radians. `end` - `start` is a full turn.
  float start = static_cast<float>(-kLidarAzimuthPi);
  float end = static_cast<float>(kLidarAzimuthPi);

  // From the blueprint attributes' degrees. The conversion runs in double so
  // that -180/180 land on the same floats atan2 returns for -pi/pi.
  static LidarAzimuthRange FromDegrees(bool clockwise, float start_deg, float end_deg) {
    LidarAzimuthRange range;
    range.clockwise = clockwise;
    range.start = static_cast<float>(static_cast<double>(start_deg) * kLidarAzimuthPi / 180.0);
    range.end = static_cast<float>(static_cast<double>(end_deg) * kLidarAzimuthPi / 180.0);
    return range;
  }
};

inline float ComputeLidarAzimuth(float x, float y, const LidarAzimuthRange &range) {
  constexpr float kTwoPi = static_cast<float>(2.0 * kLidarAzimuthPi);
  float azimuth = std::atan2(range.clockwise ? -y : y, x);  // [-pi, pi]
  // Only angles outside the range are wrapped, so the default range returns
  // atan2 unchanged, its +-pi edges included.
  if (azimuth < range.start || azimuth > range.end) {
    azimuth = range.start + std::fmod(azimuth - range.start, kTwoPi);
    if (azimuth < range.start) {
      azimuth += kTwoPi;
    }
    // A full turn in float can round onto `end`; report it as `start`.
    if (azimuth >= range.end) {
      azimuth = range.start;
    }
  }
  return azimuth;
}

// Semantic lidar: 6 fields. 4 FLOAT32 + 2 UINT32. Point stride = 24 B =
// sizeof(sensor::data::SemanticLidarDetection).
inline constexpr std::array<PointFieldDescriptor, 6> kSemanticLidarFields = {{
    {"x",             0u,  PointFieldDataType::Float32, 1u},
    {"y",             4u,  PointFieldDataType::Float32, 1u},
    {"z",             8u,  PointFieldDataType::Float32, 1u},
    {"cos_inc_angle", 12u, PointFieldDataType::Float32, 1u},
    {"object_idx",    16u, PointFieldDataType::UInt32,  1u},
    {"object_tag",    20u, PointFieldDataType::UInt32,  1u},
}};

// Radar: 7 FLOAT32 fields. Point stride = 28 B =
// sizeof(RadarDetectionWithPosition) — 3 Cartesian floats prepended to the
// raw RadarDetection payload (velocity, azimuth, altitude, depth).
inline constexpr std::array<PointFieldDescriptor, 7> kRadarFields = {{
    {"x",        0u,  PointFieldDataType::Float32, 1u},
    {"y",        4u,  PointFieldDataType::Float32, 1u},
    {"z",        8u,  PointFieldDataType::Float32, 1u},
    {"velocity", 12u, PointFieldDataType::Float32, 1u},
    {"azimuth",  16u, PointFieldDataType::Float32, 1u},
    {"altitude", 20u, PointFieldDataType::Float32, 1u},
    {"depth",    24u, PointFieldDataType::Float32, 1u},
}};

// DVS events: x/y UINT16, t FLOAT64, pol INT8. The producer converts the
// underlying int64 event timestamp to a real double in the 8-byte t slot
// (sensor_msgs PointField has no 8-byte integer datatype). Per-point stride
// matches the packed sensor::data::DVSEvent (13 B with #pragma pack(push, 1)).
inline constexpr std::array<PointFieldDescriptor, 4> kDvsFields = {{
    {"x",   0u,  PointFieldDataType::UInt16,  1u},
    {"y",   2u,  PointFieldDataType::UInt16,  1u},
    {"t",   4u,  PointFieldDataType::Float64, 1u},
    {"pol", 12u, PointFieldDataType::Int8,    1u},
}};

// Each table's last field's offset + datatype size must equal the
// corresponding sensor's point stride. The producers check that with a
// runtime sizeof() assertion against the underlying POD; layout tests in
// test_point_cloud_fields.cpp verify the same property at compile time.

}  // namespace ros2
}  // namespace carla
