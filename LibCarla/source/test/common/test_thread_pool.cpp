// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/ThreadPool.h"

#include <memory>
#include <stdexcept>
#include <type_traits>

TEST(ThreadPool, PostReturnsTaskResult) {
  carla::ThreadPool pool;
  auto result = pool.Post([]() { return 42; });
  static_assert(std::is_same_v<decltype(result), std::future<int>>);
  pool.io_context().poll();
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  EXPECT_EQ(result.get(), 42);
}

TEST(ThreadPool, PostVoidLvalueTask) {
  carla::ThreadPool pool;
  int count = 0;
  auto task = [&count]() { ++count; };
  auto result = pool.Post(task);
  static_assert(std::is_same_v<decltype(result), std::future<void>>);
  pool.io_context().poll();
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  result.get();
  EXPECT_EQ(count, 1);
}

TEST(ThreadPool, PostMoveOnlyTaskAndResult) {
  carla::ThreadPool pool;
  auto result = pool.Post([value = std::make_unique<int>(17)]() mutable {
    return std::move(value);
  });
  static_assert(std::is_same_v<decltype(result), std::future<std::unique_ptr<int>>>);
  pool.io_context().poll();
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  auto value = result.get();
  ASSERT_NE(value, nullptr);
  EXPECT_EQ(*value, 17);
}

TEST(ThreadPool, PostPreservesResultReference) {
  carla::ThreadPool pool;
  int value = 23;
  auto result = pool.Post([&value]() -> int & { return value; });
  static_assert(std::is_same_v<decltype(result), std::future<int &>>);
  pool.io_context().poll();
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  EXPECT_EQ(&result.get(), &value);
}

TEST(ThreadPool, PostDeducesStoredCallableLvalueOverload) {
  struct Task {
    int operator()() & { return 31; }
    long operator()() && { return 99; }
  };
  carla::ThreadPool pool;
  auto result = pool.Post(Task{});
  static_assert(std::is_same_v<decltype(result), std::future<int>>);
  pool.io_context().poll();
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  EXPECT_EQ(result.get(), 31);
}

TEST(ThreadPool, PostFunctionPointer) {
  carla::ThreadPool pool;
  auto result = pool.Post(+[]() { return 53; });
  static_assert(std::is_same_v<decltype(result), std::future<int>>);
  pool.io_context().poll();
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  EXPECT_EQ(result.get(), 53);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST(ThreadPool, PostPropagatesTaskExceptionThroughFuture) {
  carla::ThreadPool pool;
  auto result = pool.Post([]() -> int {
    carla::throw_exception(std::runtime_error("task failed"));
  });
  static_assert(std::is_same_v<decltype(result), std::future<int>>);
  EXPECT_NO_THROW(pool.io_context().poll());
  ASSERT_EQ(result.wait_for(std::chrono::seconds(0)), std::future_status::ready);
  EXPECT_THROW(result.get(), std::runtime_error);
}
#endif // LIBCARLA_NO_EXCEPTIONS
