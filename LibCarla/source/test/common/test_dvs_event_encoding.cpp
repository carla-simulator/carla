// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ros2/publishers/DvsEventEncoding.h>

#include <array>

TEST(DvsEventEncoding, empty_event_stream_produces_all_zeros) {
  const auto buffer = carla::ros2::EncodeDvsEventsToBgr(
      4u, 4u, static_cast<const carla::ros2::DvsEvent *>(nullptr), 0u);
  ASSERT_EQ(buffer.size(), 4u * 4u * 3u);
  for (auto byte : buffer) {
    EXPECT_EQ(byte, 0u);
  }
}

TEST(DvsEventEncoding, buffer_length_is_height_times_width_times_three) {
  const std::array<carla::ros2::DvsEvent, 0> events = {};
  const auto buffer = carla::ros2::EncodeDvsEventsToBgr(
      8u, 5u, events.data(), events.size());
  EXPECT_EQ(buffer.size(), 8u * 5u * 3u);
}

TEST(DvsEventEncoding, positive_polarity_lights_red_channel) {
  // Positive polarity: index offset 2 (red). At pixel (x=1, y=0) in a
  // 4x2 image the red byte sits at (0*4 + 1)*3 + 2 = 5.
  const std::array<carla::ros2::DvsEvent, 1> events = {
      carla::ros2::DvsEvent{1, 0, 0, true},
  };
  const auto buffer = carla::ros2::EncodeDvsEventsToBgr(
      2u, 4u, events.data(), events.size());
  EXPECT_EQ(buffer[5], 255u);
  EXPECT_EQ(buffer[3], 0u);
  EXPECT_EQ(buffer[4], 0u);
}

TEST(DvsEventEncoding, negative_polarity_lights_blue_channel) {
  // Negative polarity: index offset 0 (blue). At pixel (x=2, y=1) in a
  // 4x2 image the blue byte sits at (1*4 + 2)*3 + 0 = 18.
  const std::array<carla::ros2::DvsEvent, 1> events = {
      carla::ros2::DvsEvent{2, 1, 0, false},
  };
  const auto buffer = carla::ros2::EncodeDvsEventsToBgr(
      2u, 4u, events.data(), events.size());
  EXPECT_EQ(buffer[18], 255u);
  EXPECT_EQ(buffer[19], 0u);
  EXPECT_EQ(buffer[20], 0u);
}

TEST(DvsEventEncoding, mixed_polarities_at_distinct_pixels_dont_collide) {
  const std::array<carla::ros2::DvsEvent, 2> events = {
      carla::ros2::DvsEvent{0, 0, 0, true},
      carla::ros2::DvsEvent{1, 0, 0, false},
  };
  const auto buffer = carla::ros2::EncodeDvsEventsToBgr(
      1u, 2u, events.data(), events.size());
  // pixel (0,0): red byte = 2.
  EXPECT_EQ(buffer[2], 255u);
  // pixel (1,0): blue byte = 3.
  EXPECT_EQ(buffer[3], 255u);
}

TEST(DvsEventEncoding, out_of_bounds_events_are_dropped) {
  // x or y >= width / height must be silently skipped to mirror the legacy
  // publisher behaviour (subscribers were tolerant of stray events on the
  // pre-PR-4 wire format).
  const std::array<carla::ros2::DvsEvent, 3> events = {
      carla::ros2::DvsEvent{99, 0,  0, true},
      carla::ros2::DvsEvent{0,  99, 0, true},
      carla::ros2::DvsEvent{2,  2,  0, true},
  };
  const auto buffer = carla::ros2::EncodeDvsEventsToBgr(
      4u, 4u, events.data(), events.size());
  ASSERT_EQ(buffer.size(), 4u * 4u * 3u);
  // Only the in-bounds event lit a pixel; sum equals 255.
  std::uint32_t sum = 0u;
  for (auto byte : buffer) {
    sum += byte;
  }
  EXPECT_EQ(sum, 255u);
}

TEST(DvsEventEncoding, raw_byte_overload_matches_struct_overload) {
  // Packed wire layout for sensor::data::DVSEvent: 13 bytes (2 + 2 + 8 + 1).
  // Build a stride-13 byte stream and confirm both overloads agree.
  struct PackedEvent {
    std::uint16_t x;
    std::uint16_t y;
    std::int64_t t;
    bool pol;
  } __attribute__((packed));

  PackedEvent raw_events[2] = {
      {2u, 1u, 100, true},
      {0u, 0u, 200, false},
  };
  const carla::ros2::DvsEvent typed_events[2] = {
      {2u, 1u, 100, true},
      {0u, 0u, 200, false},
  };

  const auto from_raw = carla::ros2::EncodeDvsEventsToBgr(
      4u, 4u,
      reinterpret_cast<const std::uint8_t *>(raw_events),
      2u,
      sizeof(PackedEvent));
  const auto from_typed = carla::ros2::EncodeDvsEventsToBgr(
      4u, 4u, typed_events, 2u);
  EXPECT_EQ(from_raw, from_typed);
}
