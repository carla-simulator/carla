// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/PointCloudFieldsLayout.h>

#include <cmath>

namespace {

constexpr std::uint32_t SizeOf(carla::ros2::PointFieldDataType datatype) {
  switch (datatype) {
    case carla::ros2::PointFieldDataType::Int8:    return 1u;
    case carla::ros2::PointFieldDataType::UInt8:   return 1u;
    case carla::ros2::PointFieldDataType::Int16:   return 2u;
    case carla::ros2::PointFieldDataType::UInt16:  return 2u;
    case carla::ros2::PointFieldDataType::Int32:   return 4u;
    case carla::ros2::PointFieldDataType::UInt32:  return 4u;
    case carla::ros2::PointFieldDataType::Float32: return 4u;
    case carla::ros2::PointFieldDataType::Float64: return 8u;
  }
  return 0u;
}

// Compile-time check that within a layout the fields are tightly packed
// (each field's offset equals the running sum of previous field sizes).
template <std::size_t N>
constexpr std::uint32_t LastFieldEnd(
    const std::array<carla::ros2::PointFieldDescriptor, N> &layout) {
  return layout.back().offset + SizeOf(layout.back().datatype) * layout.back().count;
}

}  // namespace

TEST(PointCloudFields, lidar_layout_matches_lidar_detection_size) {
  const auto &fields = carla::ros2::kLidarFields;
  ASSERT_EQ(fields.size(), 4u);
  EXPECT_EQ(fields[0].name, "x");
  EXPECT_EQ(fields[1].name, "y");
  EXPECT_EQ(fields[2].name, "z");
  EXPECT_EQ(fields[3].name, "intensity");
  EXPECT_EQ(LastFieldEnd(fields), 16u);
}

TEST(PointCloudFields, extended_lidar_layout_is_autoware_xyzircaedt) {
  const auto &fields = carla::ros2::kLidarFieldsEx;
  ASSERT_EQ(fields.size(), 10u);
  EXPECT_EQ(fields[0].name, "x");
  EXPECT_EQ(fields[1].name, "y");
  EXPECT_EQ(fields[2].name, "z");
  EXPECT_EQ(fields[3].name, "intensity");
  EXPECT_EQ(fields[4].name, "return_type");
  EXPECT_EQ(fields[5].name, "channel");
  EXPECT_EQ(fields[6].name, "azimuth");
  EXPECT_EQ(fields[7].name, "elevation");
  EXPECT_EQ(fields[8].name, "distance");
  EXPECT_EQ(fields[9].name, "time_stamp");
  EXPECT_EQ(fields[3].datatype, carla::ros2::PointFieldDataType::UInt8);
  EXPECT_EQ(fields[4].datatype, carla::ros2::PointFieldDataType::UInt8);
  EXPECT_EQ(fields[5].datatype, carla::ros2::PointFieldDataType::UInt16);
  EXPECT_EQ(fields[9].datatype, carla::ros2::PointFieldDataType::UInt32);
  // Tightly packed 32 B stride: matches both the Autoware PointXYZIRCAEDT
  // struct and the naturally aligned LidarPointEx POD in the publisher.
  EXPECT_EQ(LastFieldEnd(fields), 32u);
}

TEST(PointCloudFields, semantic_lidar_layout_matches_semantic_lidar_detection_size) {
  const auto &fields = carla::ros2::kSemanticLidarFields;
  ASSERT_EQ(fields.size(), 6u);
  EXPECT_EQ(fields[0].name, "x");
  EXPECT_EQ(fields[3].name, "cos_inc_angle");
  EXPECT_EQ(fields[4].name, "object_idx");
  EXPECT_EQ(fields[5].name, "object_tag");
  EXPECT_EQ(fields[4].datatype, carla::ros2::PointFieldDataType::UInt32);
  EXPECT_EQ(fields[5].datatype, carla::ros2::PointFieldDataType::UInt32);
  EXPECT_EQ(LastFieldEnd(fields), 24u);
}

TEST(PointCloudFields, radar_layout_carries_seven_float_fields) {
  const auto &fields = carla::ros2::kRadarFields;
  ASSERT_EQ(fields.size(), 7u);
  EXPECT_EQ(fields[3].name, "velocity");
  EXPECT_EQ(fields[4].name, "azimuth");
  EXPECT_EQ(fields[5].name, "altitude");
  EXPECT_EQ(fields[6].name, "depth");
  for (const auto &field : fields) {
    EXPECT_EQ(field.datatype, carla::ros2::PointFieldDataType::Float32);
    EXPECT_EQ(field.count, 1u);
  }
  EXPECT_EQ(LastFieldEnd(fields), 28u);
}

TEST(PointCloudFields, dvs_layout_matches_packed_dvs_event_size) {
  const auto &fields = carla::ros2::kDvsFields;
  ASSERT_EQ(fields.size(), 4u);
  EXPECT_EQ(fields[0].name, "x");
  EXPECT_EQ(fields[1].name, "y");
  EXPECT_EQ(fields[2].name, "t");
  EXPECT_EQ(fields[3].name, "pol");
  EXPECT_EQ(fields[0].datatype, carla::ros2::PointFieldDataType::UInt16);
  EXPECT_EQ(fields[2].datatype, carla::ros2::PointFieldDataType::Float64);
  EXPECT_EQ(fields[3].datatype, carla::ros2::PointFieldDataType::Int8);
  // sensor::data::DVSEvent is #pragma pack(push, 1): 2 + 2 + 8 + 1 = 13.
  EXPECT_EQ(LastFieldEnd(fields), 13u);
}

TEST(PointCloudFields, layouts_offsets_are_monotonically_increasing) {
  auto check_monotonic = [](auto &layout) {
    for (std::size_t i = 1; i < layout.size(); ++i) {
      EXPECT_GT(layout[i].offset, layout[i - 1].offset);
    }
  };
  check_monotonic(carla::ros2::kLidarFields);
  check_monotonic(carla::ros2::kLidarFieldsEx);
  check_monotonic(carla::ros2::kSemanticLidarFields);
  check_monotonic(carla::ros2::kRadarFields);
  check_monotonic(carla::ros2::kDvsFields);
}

namespace {

constexpr float kPi = 3.14159265358979323846f;

}  // namespace

// The default azimuth range is atan2(y, x) in the ROS frame, unchanged.
TEST(PointCloudFields, lidar_azimuth_default_is_atan2) {
  const carla::ros2::LidarAzimuthRange range =
      carla::ros2::LidarAzimuthRange::FromDegrees(false, -180.0f, 180.0f);
  for (int i = 0; i < 3600; ++i) {
    const float angle = static_cast<float>(i) * 0.1f * kPi / 180.0f;
    const float x = std::cos(angle);
    const float y = std::sin(angle);
    EXPECT_EQ(carla::ros2::ComputeLidarAzimuth(x, y, range), std::atan2(y, x));
  }
  // Both edges directly behind the sensor stay where atan2 puts them.
  EXPECT_EQ(carla::ros2::ComputeLidarAzimuth(-1.0f, 0.0f, range), std::atan2(0.0f, -1.0f));
  EXPECT_EQ(carla::ros2::ComputeLidarAzimuth(-1.0f, -0.0f, range), std::atan2(-0.0f, -1.0f));
  const carla::ros2::LidarAzimuthRange default_range{};
  EXPECT_EQ(default_range.start, range.start);
  EXPECT_EQ(default_range.end, range.end);
  EXPECT_FALSE(default_range.clockwise);
}

// Clockwise from forward in [0, 360) is the rotation angle Autoware's LiDAR
// drivers report: forward, right, behind and left are 0, 90, 180 and 270.
TEST(PointCloudFields, lidar_azimuth_clockwise_zero_to_two_pi) {
  const carla::ros2::LidarAzimuthRange range =
      carla::ros2::LidarAzimuthRange::FromDegrees(true, 0.0f, 360.0f);
  EXPECT_NEAR(carla::ros2::ComputeLidarAzimuth(1.0f, 0.0f, range), 0.0f, 1e-6f);
  EXPECT_NEAR(carla::ros2::ComputeLidarAzimuth(0.0f, -1.0f, range), 0.5f * kPi, 1e-6f);
  EXPECT_NEAR(carla::ros2::ComputeLidarAzimuth(-1.0f, 0.0f, range), kPi, 1e-6f);
  EXPECT_NEAR(carla::ros2::ComputeLidarAzimuth(0.0f, 1.0f, range), 1.5f * kPi, 1e-6f);
  for (int i = 0; i < 3600; ++i) {
    const float angle = static_cast<float>(i) * 0.1f * kPi / 180.0f;
    const float azimuth =
        carla::ros2::ComputeLidarAzimuth(std::cos(angle), std::sin(angle), range);
    EXPECT_GE(azimuth, range.start);
    EXPECT_LT(azimuth, range.end);
  }
  // A point just left of forward wraps to just under 2*pi, never onto it.
  EXPECT_LT(carla::ros2::ComputeLidarAzimuth(1.0f, 1e-30f, range), range.end);
  // CARLA's LiDAR sweeps clockwise seen from above (towards -y in the ROS
  // frame), so along a ring the azimuth grows by the horizontal step.
  const float step = 0.2f * kPi / 180.0f;
  for (int i = 1; i < 1800; ++i) {
    const float previous = -static_cast<float>(i - 1) * step;
    const float current = -static_cast<float>(i) * step;
    const float a = carla::ros2::ComputeLidarAzimuth(std::cos(previous), std::sin(previous), range);
    const float b = carla::ros2::ComputeLidarAzimuth(std::cos(current), std::sin(current), range);
    EXPECT_NEAR(b - a, step, 1e-4f) << "at step " << i;
  }
}

// Any full-turn interval and either direction: the result lies in the
// interval and is the same angle as atan2 in the chosen direction.
TEST(PointCloudFields, lidar_azimuth_arbitrary_range_and_direction) {
  for (const bool clockwise : {false, true}) {
    for (const float start_deg : {-180.0f, -90.0f, 0.0f, 45.0f, 180.0f}) {
      const carla::ros2::LidarAzimuthRange range =
          carla::ros2::LidarAzimuthRange::FromDegrees(clockwise, start_deg, start_deg + 360.0f);
      for (int i = 0; i < 360; ++i) {
        const float angle = static_cast<float>(i) * kPi / 180.0f;
        const float x = std::cos(angle);
        const float y = std::sin(angle);
        const float azimuth = carla::ros2::ComputeLidarAzimuth(x, y, range);
        EXPECT_GE(azimuth, range.start);
        EXPECT_LE(azimuth, range.end);
        const float expected = std::atan2(clockwise ? -y : y, x);
        EXPECT_NEAR(std::cos(azimuth), std::cos(expected), 1e-5f);
        EXPECT_NEAR(std::sin(azimuth), std::sin(expected), 1e-5f);
      }
    }
  }
}
