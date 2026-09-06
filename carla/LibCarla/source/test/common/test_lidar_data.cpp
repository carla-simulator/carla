// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/sensor/data/LidarData.h>
#include <carla/sensor/data/SemanticLidarData.h>

#include <vector>

using carla::sensor::data::LidarData;
using carla::sensor::data::SemanticLidarData;

// {Lidar,SemanticLidar}Data::ResetMemory rejects callers that supply more
// points-per-channel entries than the configured channel count; the equal
// count case is valid. These tests pin the boundary so a future tightening
// to a strict `>` would be caught. DEBUG_ASSERT compiles away under NDEBUG,
// so the assertion only fires when the suite is built as Debug.

TEST(LidarData, reset_memory_accepts_one_entry_per_channel) {
  constexpr uint32_t channel_count{4u};
  LidarData data{channel_count};
  std::vector<uint32_t> points_per_channel(channel_count, 1u);
  data.ResetMemory(points_per_channel);
  EXPECT_EQ(data.GetChannelCount(), channel_count);
}

TEST(LidarData, reset_memory_accepts_fewer_entries_than_channels) {
  constexpr uint32_t channel_count{8u};
  LidarData data{channel_count};
  std::vector<uint32_t> points_per_channel(channel_count - 2u, 3u);
  data.ResetMemory(points_per_channel);
  EXPECT_EQ(data.GetChannelCount(), channel_count);
}

TEST(SemanticLidarData, reset_memory_accepts_one_entry_per_channel) {
  constexpr uint32_t channel_count{4u};
  SemanticLidarData data{channel_count};
  std::vector<uint32_t> points_per_channel(channel_count, 1u);
  data.ResetMemory(points_per_channel);
  EXPECT_EQ(data.GetChannelCount(), channel_count);
}

TEST(SemanticLidarData, reset_memory_accepts_fewer_entries_than_channels) {
  constexpr uint32_t channel_count{8u};
  SemanticLidarData data{channel_count};
  std::vector<uint32_t> points_per_channel(channel_count - 2u, 3u);
  data.ResetMemory(points_per_channel);
  EXPECT_EQ(data.GetChannelCount(), channel_count);
}
