// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/Buffer.h>

#include <array>
#include <limits>

TEST(buffer_resize, growth_preserves_only_the_initialized_prefix) {
  carla::Buffer buffer;
  const std::array<uint8_t, 3u> prefix{{11u, 22u, 33u}};
  buffer.copy_from(prefix);
  buffer.resize(4096u);
  ASSERT_EQ(buffer.size(), 4096u);
  ASSERT_GE(buffer.capacity(), buffer.size());
  EXPECT_EQ(buffer[0u], 11u);
  EXPECT_EQ(buffer[1u], 22u);
  EXPECT_EQ(buffer[2u], 33u);
}

TEST(buffer_resize, empty_buffer_can_grow_without_reading_null_memory) {
  carla::Buffer buffer;
  buffer.resize(16u);
  EXPECT_EQ(buffer.size(), 16u);
  EXPECT_GE(buffer.capacity(), buffer.size());
  EXPECT_NE(buffer.data(), nullptr);
}

TEST(buffer_resize, rejected_size_retains_existing_allocation_and_contents) {
  carla::Buffer buffer;
  buffer.copy_from(std::array<uint8_t, 3u>{{11u, 22u, 33u}});
  const auto *data = buffer.data();
  const auto capacity = buffer.capacity();
  EXPECT_THROW(buffer.resize(static_cast<uint64_t>(carla::Buffer::max_size()) + 1u),
      std::invalid_argument);
  ASSERT_EQ(buffer.data(), data);
  EXPECT_EQ(buffer.size(), 3u);
  EXPECT_EQ(buffer.capacity(), capacity);
  EXPECT_EQ(buffer[0u], 11u);
  EXPECT_EQ(buffer[2u], 33u);
}

TEST(buffer_resize, shrink_and_regrow_within_capacity_preserve_storage_control) {
  carla::Buffer buffer(16u);
  buffer[0u] = 11u;
  buffer[1u] = 22u;
  const auto *data = buffer.data();
  buffer.resize(1u);
  EXPECT_EQ(buffer.size(), 1u);
  buffer.resize(16u);
  EXPECT_EQ(buffer.size(), 16u);
  EXPECT_EQ(buffer.data(), data);
  EXPECT_EQ(buffer[0u], 11u);
  EXPECT_EQ(buffer[1u], 22u);
}

TEST(buffer_resize, zero_size_keeps_reusable_capacity_control) {
  carla::Buffer buffer(16u);
  const auto *data = buffer.data();
  buffer.resize(0u);
  EXPECT_TRUE(buffer.empty());
  EXPECT_EQ(buffer.capacity(), 16u);
  EXPECT_EQ(buffer.data(), data);
}
