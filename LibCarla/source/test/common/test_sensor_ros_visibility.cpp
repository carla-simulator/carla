// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"
#include <carla/streaming/Server.h>
#include <carla/streaming/detail/Token.h>

TEST(streaming, sensor_ros_visibility_follows_shared_stream_state) {
  using namespace carla::streaming;
  Server server(TESTING_PORT);
  auto stream = server.MakeStream();
  const auto id = detail::token_type(stream.token()).get_stream_id();
  const auto copy = stream;
  EXPECT_FALSE(stream.IsEnabledForROS());
  EXPECT_FALSE(copy.IsEnabledForROS());
  server.EnableForROS(id);
  EXPECT_TRUE(stream.IsEnabledForROS());
  EXPECT_TRUE(copy.IsEnabledForROS());
  server.DisableForROS(id);
  EXPECT_FALSE(stream.IsEnabledForROS());
  EXPECT_FALSE(copy.IsEnabledForROS());
  server.EnableForROS(id);
  EXPECT_TRUE(stream.IsEnabledForROS());
}

TEST(streaming, sensor_ros_visibility_respects_startup_default) {
  using namespace carla::streaming;
  Server server(TESTING_PORT);
  server.SetROS2TopicVisibilityDefaultEnabled(true);
  auto stream = server.MakeStream();
  const auto id = detail::token_type(stream.token()).get_stream_id();
  EXPECT_TRUE(stream.IsEnabledForROS());
  server.DisableForROS(id);
  EXPECT_FALSE(stream.IsEnabledForROS());
}

TEST(streaming, sensor_ros_visibility_moved_from_stream_is_disabled) {
  using namespace carla::streaming;
  Server server(TESTING_PORT);
  server.SetROS2TopicVisibilityDefaultEnabled(true);
  auto stream = server.MakeStream();
  const auto moved = std::move(stream);
  EXPECT_FALSE(stream.IsEnabledForROS());
  EXPECT_TRUE(moved.IsEnabledForROS());
}
