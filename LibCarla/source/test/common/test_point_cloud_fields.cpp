// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/PointCloudFieldsLayout.h>

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
  check_monotonic(carla::ros2::kSemanticLidarFields);
  check_monotonic(carla::ros2::kRadarFields);
  check_monotonic(carla::ros2::kDvsFields);
}
