// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/RecurrentSharedFuture.h>
#include <carla/ThreadGroup.h>

#include <atomic>
#include <latch>
#include <memory>
#include <thread>
#include <variant>

using namespace std::chrono_literals;

namespace {

  struct CopyTrackedValue {
    explicit CopyTrackedValue(std::shared_ptr<std::atomic_size_t> count)
      : copies(std::move(count)) {}

    CopyTrackedValue(const CopyTrackedValue &other) : copies(other.copies) {
      ++*copies;
    }

    CopyTrackedValue &operator=(const CopyTrackedValue &other) {
      copies = other.copies;
      ++*copies;
      return *this;
    }

    CopyTrackedValue(CopyTrackedValue &&) = default;
    CopyTrackedValue &operator=(CopyTrackedValue &&) = default;

    std::shared_ptr<std::atomic_size_t> copies;
  };

#ifndef LIBCARLA_NO_EXCEPTIONS
  struct LifetimeException : std::runtime_error {
    explicit LifetimeException(std::shared_ptr<int> value)
      : std::runtime_error("expected exception"), value(std::move(value)) {}

    std::shared_ptr<int> value;
  };

  struct ThrowingMoveValue {
    explicit ThrowingMoveValue(std::shared_ptr<int> value)
      : value(std::move(value)) {}

    ThrowingMoveValue(const ThrowingMoveValue &) = default;
    ThrowingMoveValue &operator=(const ThrowingMoveValue &) = default;

    ThrowingMoveValue(ThrowingMoveValue &&other) : value(other.value) {
      carla::throw_exception(std::runtime_error("expected move exception"));
    }

    std::shared_ptr<int> value;
  };

  struct CopyFailureState {
    std::atomic_bool fail{true};
  };

  struct ThrowingCopyValue {
    explicit ThrowingCopyValue(std::shared_ptr<CopyFailureState> state)
      : state(std::move(state)) {}

    ThrowingCopyValue(const ThrowingCopyValue &other) : state(other.state) {
      if (state->fail.load()) {
        carla::throw_exception(std::runtime_error("expected copy exception"));
      }
    }

    ThrowingCopyValue &operator=(const ThrowingCopyValue &) = default;
    ThrowingCopyValue(ThrowingCopyValue &&) = default;
    ThrowingCopyValue &operator=(ThrowingCopyValue &&) = default;

    std::shared_ptr<CopyFailureState> state;
  };

  template <typename ExceptionT>
  void ExpectLvalueExceptionDelivery(ExceptionT &exception) {
    carla::RecurrentSharedFuture<int> future;
    std::atomic_bool completed{false};
    std::thread waiter([&]() {
      try {
        future.WaitFor(2s);
        ADD_FAILURE() << "Expected the published exception";
      } catch (const carla::detail::SharedException &received) {
        EXPECT_STREQ(received.what(), exception.what());
        EXPECT_NE(received.GetException().get(), &exception);
      } catch (const std::exception &unexpected) {
        ADD_FAILURE() << unexpected.what();
      }
      completed = true;
    });

    const auto deadline = std::chrono::steady_clock::now() + 2s;
    while (!completed.load() && std::chrono::steady_clock::now() < deadline) {
      future.SetException(exception);
      std::this_thread::sleep_for(1ms);
    }
    waiter.join();
  }
#endif // LIBCARLA_NO_EXCEPTIONS

} // namespace

TEST(recurrent_shared_future, timed_out_waiter_does_not_copy_later_values) {
  carla::RecurrentSharedFuture<CopyTrackedValue> future;
  EXPECT_FALSE(future.WaitFor(0ms).has_value());

  auto copies = std::make_shared<std::atomic_size_t>(0u);
  future.SetValue(CopyTrackedValue(copies));
  EXPECT_EQ(copies->load(), 0u);
}

TEST(recurrent_shared_future, timed_out_waiter_does_not_retain_later_values) {
  carla::RecurrentSharedFuture<std::shared_ptr<int>> future;
  EXPECT_FALSE(future.WaitFor(0ms).has_value());

  auto value = std::make_shared<int>(42);
  std::weak_ptr<int> weak = value;
  future.SetValue(value);
  value.reset();
  EXPECT_TRUE(weak.expired());
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST(recurrent_shared_future, timed_out_waiter_does_not_retain_exceptions) {
  carla::RecurrentSharedFuture<int> future;
  EXPECT_FALSE(future.WaitFor(0ms).has_value());

  auto value = std::make_shared<int>(42);
  std::weak_ptr<int> weak = value;
  future.SetException(LifetimeException(value));
  value.reset();
  EXPECT_TRUE(weak.expired());
}
#endif // LIBCARLA_NO_EXCEPTIONS

TEST(recurrent_shared_future, inactive_threads_do_not_copy_later_values) {
  constexpr size_t count = 32u;
  carla::RecurrentSharedFuture<CopyTrackedValue> future;
  carla::ThreadGroup threads;
  std::latch timed_out(count);
  std::latch release(1u);

  threads.CreateThreads(count, [&]() {
    EXPECT_FALSE(future.WaitFor(0ms).has_value());
    timed_out.count_down();
    release.wait();
  });
  timed_out.wait();

  auto copies = std::make_shared<std::atomic_size_t>(0u);
  future.SetValue(CopyTrackedValue(copies));
  EXPECT_EQ(copies->load(), 0u);

  release.count_down();
  threads.JoinAll();
}

TEST(recurrent_shared_future, supports_monostate_values) {
  carla::RecurrentSharedFuture<std::monostate> future;
  EXPECT_FALSE(future.WaitFor(0ms).has_value());
  std::atomic_bool completed{false};
  std::thread waiter([&]() {
    auto result = future.WaitFor(2s);
    EXPECT_TRUE(result.has_value());
    completed = true;
  });

  const auto deadline = std::chrono::steady_clock::now() + 2s;
  while (!completed.load() && std::chrono::steady_clock::now() < deadline) {
    future.SetValue(std::monostate{});
    std::this_thread::sleep_for(1ms);
  }
  waiter.join();
}

TEST(recurrent_shared_future, concurrent_delivery_releases_completed_waiters) {
  constexpr size_t count = 16u;
  carla::RecurrentSharedFuture<std::shared_ptr<int>> future;
  carla::ThreadGroup threads;
  std::latch ready(count);
  std::atomic_size_t completed{0u};
  auto value = std::make_shared<int>(42);

  threads.CreateThreads(count, [&]() {
    ready.count_down();
    auto result = future.WaitFor(2s);
    EXPECT_TRUE(result.has_value());
    if (result.has_value()) {
      EXPECT_EQ(**result, 42);
    }
    ++completed;
  });
  ready.wait();

  const auto deadline = std::chrono::steady_clock::now() + 2s;
  while (completed.load() != count && std::chrono::steady_clock::now() < deadline) {
    future.SetValue(value);
    std::this_thread::sleep_for(1ms);
  }
  threads.JoinAll();
  EXPECT_EQ(completed.load(), count);

  future.SetValue(value);
  EXPECT_EQ(value.use_count(), 1);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST(recurrent_shared_future, delivered_exception_releases_waiter) {
  carla::RecurrentSharedFuture<std::shared_ptr<int>> future;
  std::atomic_bool completed{false};
  std::thread waiter([&]() {
    EXPECT_THROW(future.WaitFor(2s), carla::detail::SharedException);
    completed = true;
  });

  const auto deadline = std::chrono::steady_clock::now() + 2s;
  while (!completed.load() && std::chrono::steady_clock::now() < deadline) {
    future.SetException(std::runtime_error("expected exception"));
    std::this_thread::sleep_for(1ms);
  }
  waiter.join();

  auto value = std::make_shared<int>(42);
  future.SetValue(value);
  EXPECT_EQ(value.use_count(), 1);
}
#endif // LIBCARLA_NO_EXCEPTIONS

TEST(recurrent_shared_future, repeated_waits_race_delivery_and_timeout) {
  constexpr size_t count = 16u;
  constexpr size_t repetitions = 100u;
  carla::RecurrentSharedFuture<std::shared_ptr<int>> future;
  carla::ThreadGroup threads;
  auto value = std::make_shared<int>(42);
  std::atomic_bool done{false};
  std::atomic_size_t delivered{0u};
  std::atomic_size_t timed_out{0u};

  std::thread publisher([&]() {
    while (!done.load()) {
      future.SetValue(value);
      std::this_thread::yield();
    }
  });
  threads.CreateThreads(count, [&]() {
    for (size_t i = 0u; i < repetitions; ++i) {
      auto result = future.WaitFor(i % 2u == 0u ? 0ms : 1ms);
      if (result.has_value()) {
        EXPECT_EQ(**result, 42);
        ++delivered;
      } else {
        ++timed_out;
      }
    }
  });
  threads.JoinAll();
  done = true;
  publisher.join();

  EXPECT_EQ(delivered.load() + timed_out.load(), count * repetitions);
  future.SetValue(value);
  EXPECT_EQ(value.use_count(), 1);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST(recurrent_shared_future, accepts_mutable_lvalue_exception) {
  std::runtime_error exception("mutable lvalue exception");
  ExpectLvalueExceptionDelivery(exception);
}

TEST(recurrent_shared_future, accepts_const_lvalue_exception) {
  const std::runtime_error exception("const lvalue exception");
  ExpectLvalueExceptionDelivery(exception);
}

TEST(recurrent_shared_future, failed_result_move_releases_waiter) {
  carla::RecurrentSharedFuture<ThrowingMoveValue> future;
  std::atomic_bool completed{false};
  std::thread waiter([&]() {
    EXPECT_THROW(future.WaitFor(2s), std::runtime_error);
    completed = true;
  });

  auto payload = std::make_shared<int>(42);
  const ThrowingMoveValue value(payload);
  const auto deadline = std::chrono::steady_clock::now() + 2s;
  while (!completed.load() && std::chrono::steady_clock::now() < deadline) {
    future.SetValue(value);
    std::this_thread::sleep_for(1ms);
  }
  waiter.join();

  future.SetValue(value);
  EXPECT_EQ(payload.use_count(), 2);
}

TEST(recurrent_shared_future, failed_value_copy_does_not_complete_waiter) {
  carla::RecurrentSharedFuture<ThrowingCopyValue> future;
  std::atomic_bool completed{false};
  std::thread waiter([&]() {
    EXPECT_NO_THROW({
      auto result = future.WaitFor(500ms);
      EXPECT_FALSE(result.has_value());
    });
    completed = true;
  });

  auto state = std::make_shared<CopyFailureState>();
  const ThrowingCopyValue value(state);
  bool failed = false;
  const auto deadline = std::chrono::steady_clock::now() + 500ms;
  while (!completed.load() && !failed && std::chrono::steady_clock::now() < deadline) {
    try {
      future.SetValue(value);
    } catch (const std::runtime_error &) {
      failed = true;
    }
    std::this_thread::yield();
  }
  waiter.join();
  EXPECT_TRUE(failed);

  state->fail = false;
  future.SetValue(value);
  EXPECT_EQ(state.use_count(), 2);
}
#endif // LIBCARLA_NO_EXCEPTIONS
