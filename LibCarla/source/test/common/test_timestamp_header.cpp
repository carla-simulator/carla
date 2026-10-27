// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Include the public header first to check its direct dependencies.
#include <carla/client/Timestamp.h>

#include <gtest/gtest.h>

#include <sstream>

TEST(timestamp_header, default_values) {
  const carla::client::Timestamp timestamp;
  EXPECT_EQ(timestamp.frame, 0u);
  EXPECT_DOUBLE_EQ(timestamp.elapsed_seconds, 0.0);
  EXPECT_DOUBLE_EQ(timestamp.delta_seconds, 0.0);
  EXPECT_DOUBLE_EQ(timestamp.platform_timestamp, 0.0);
}

TEST(timestamp_header, constructor_preserves_measurement_fields) {
  const carla::client::Timestamp timestamp(42u, 1.5, 0.05, 1000.0);
  EXPECT_EQ(timestamp.frame, 42u);
  EXPECT_DOUBLE_EQ(timestamp.elapsed_seconds, 1.5);
  EXPECT_DOUBLE_EQ(timestamp.delta_seconds, 0.05);
  EXPECT_DOUBLE_EQ(timestamp.platform_timestamp, 1000.0);
}

TEST(timestamp_header, equality_retains_frame_identity) {
  const carla::client::Timestamp timestamp(42u, 1.5, 0.05, 1000.0);
  const carla::client::Timestamp same_frame(42u, 9.0, 0.1, 2000.0);
  const carla::client::Timestamp next_frame(43u, 1.5, 0.05, 1000.0);
  EXPECT_EQ(timestamp, same_frame);
  EXPECT_FALSE(timestamp != same_frame);
  EXPECT_NE(timestamp, next_frame);
}

TEST(timestamp_header, stream_output_retains_format_and_chaining) {
  const carla::client::Timestamp timestamp(42u, 1.5, 0.05, 1000.0);
  std::ostringstream stream;
  std::ostream &result = stream << timestamp;
  EXPECT_EQ(&result, &stream);
  EXPECT_EQ(stream.str(), "Timestamp(frame=42,elapsed_seconds=1.500000,"
      "delta_seconds=0.050000,platform_timestamp=1000.000000)");
}
