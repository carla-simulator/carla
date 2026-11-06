// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/StringUtil.h>
#include <carla/sensor/data/ActorDynamicState.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <type_traits>

namespace csd = carla::sensor::data;

TEST(fixed_signal_id, short_identifier_stops_at_null) {
  csd::detail::TrafficLightData data{};
  std::fill_n(data.sign_id, sizeof(data.sign_id), 'x');
  std::memcpy(data.sign_id, "signal-42", sizeof("signal-42"));
  EXPECT_EQ(carla::StringUtil::FromFixedBuffer(data.sign_id), "signal-42");
}

TEST(fixed_signal_id, full_width_identifier_does_not_read_timing_bytes) {
  csd::detail::TrafficLightData data{};
  std::fill_n(data.sign_id, sizeof(data.sign_id), 's');
  data.green_time = 42.42f;
  data.yellow_time = 3.0f;
  EXPECT_EQ(carla::StringUtil::FromFixedBuffer(data.sign_id), std::string(32u, 's'));
}

TEST(fixed_signal_id, full_width_sign_has_no_storage_beyond_its_identifier) {
  csd::detail::TrafficSignData data{};
  std::fill_n(data.sign_id, sizeof(data.sign_id), 's');
  EXPECT_EQ(carla::StringUtil::FromFixedBuffer(data.sign_id), std::string(32u, 's'));
}

TEST(fixed_signal_id, empty_identifier_remains_empty) {
  csd::detail::TrafficSignData data{};
  EXPECT_TRUE(carla::StringUtil::FromFixedBuffer(data.sign_id).empty());
}

TEST(fixed_signal_id, snapshot_wire_layout_is_unchanged) {
  static_assert(std::is_trivially_copyable_v<csd::ActorDynamicState>);
  EXPECT_EQ(sizeof(csd::ActorDynamicState), 119u);
  EXPECT_EQ(sizeof(csd::detail::TrafficSignData), 32u);
  EXPECT_EQ(sizeof(csd::detail::TrafficLightData::sign_id), 32u);
}
