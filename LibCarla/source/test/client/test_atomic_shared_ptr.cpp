// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/AtomicSharedPtr.h>
#include <carla/ThreadGroup.h>

#include <memory>

TEST(atomic_shared_ptr, successful_exchange_replaces_value) {
  auto original = std::make_shared<int>(1);
  auto replacement = std::make_shared<int>(2);
  carla::AtomicSharedPtr<int> value(original);
  auto expected = original;

  EXPECT_TRUE(value.compare_exchange(&expected, replacement));
  EXPECT_EQ(expected, original);
  EXPECT_EQ(value.load(), replacement);
}

TEST(atomic_shared_ptr, failed_exchange_acquires_current_value) {
  const auto current = std::make_shared<int>(42);
  carla::AtomicSharedPtr<int> value(current);
  auto expected = std::make_shared<int>(7);
  std::weak_ptr<int> released = expected;

  EXPECT_FALSE(value.compare_exchange(&expected, std::make_shared<int>(99)));
  EXPECT_TRUE(released.expired());
  EXPECT_EQ(expected, current);
  EXPECT_EQ(*expected, 42);
  EXPECT_EQ(value.load(), current);
}

TEST(atomic_shared_ptr, exchanges_null_value) {
  carla::AtomicSharedPtr<int> value;
  std::shared_ptr<int> expected;
  auto replacement = std::make_shared<int>(42);

  EXPECT_TRUE(value.compare_exchange(&expected, replacement));
  EXPECT_EQ(expected, nullptr);
  EXPECT_TRUE(value.compare_exchange(&replacement, nullptr));
  EXPECT_EQ(value.load(), nullptr);
}

TEST(atomic_shared_ptr, concurrent_compare_exchange_publishes_values) {
  constexpr size_t threads_count = 8u;
  constexpr size_t repetitions = 1000u;
  carla::AtomicSharedPtr<const size_t> value(std::make_shared<const size_t>(0u));
  carla::ThreadGroup threads;

  threads.CreateThreads(threads_count, [&]() {
    auto expected = value.load();
    for (size_t i = 0u; i < repetitions; ++i) {
      while (!value.compare_exchange(
          &expected, std::make_shared<const size_t>(*expected + 1u))) {}
      expected = value.load();
    }
  });
  threads.JoinAll();

  EXPECT_EQ(*value.load(), threads_count * repetitions);
}
