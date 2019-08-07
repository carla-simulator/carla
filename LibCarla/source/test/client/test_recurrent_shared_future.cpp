// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/RecurrentSharedFuture.h>
#include <carla/ThreadGroup.h>

using namespace std::chrono_literals;

TEST(recurrent_shared_future, use_case) {
  using namespace carla;
  ThreadGroup threads;
  RecurrentSharedFuture<int> future;

  constexpr size_t number_of_threads = 12u;
  constexpr size_t number_of_openings = 40u;

  std::atomic_size_t count{0u};
  std::atomic_bool done{false};

  threads.CreateThreads(number_of_threads, [&]() {
    while (!done) {
      auto result = future.WaitFor(1s);
      ASSERT_TRUE(result.has_value());
      ASSERT_EQ(*result, 42);
      ++count;
    }
  });

  std::this_thread::sleep_for(100ms);
  for (auto i = 0u; i < number_of_openings - 1u; ++i) {
    future.SetValue(42);
    std::this_thread::sleep_for(10ms);
  }
  done = true;
  future.SetValue(42);
  threads.JoinAll();
  ASSERT_EQ(count, number_of_openings * number_of_threads);
}

TEST(recurrent_shared_future, timeout) {
  using namespace carla;
  RecurrentSharedFuture<int> future;
  auto result = future.WaitFor(1ns);
  ASSERT_FALSE(result.has_value());
}

TEST(recurrent_shared_future, exception) {
  using namespace carla;
  ThreadGroup threads;
  RecurrentSharedFuture<int> future;
  const std::string message = "Uh oh an exception!";

  threads.CreateThread([&]() {
    std::this_thread::sleep_for(10ms);
    future.SetException(std::runtime_error(message));
  });

  try {
    future.WaitFor(1s);
  } catch (const std::exception &e) {
    ASSERT_STREQ(e.what(), message.c_str());
  }
}
