// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "EpisodeTestServer.h"

#include <carla/ThreadGroup.h>

#include <atomic>
#include <latch>
#include <string>

using namespace std::chrono_literals;

TEST(episode_concurrency, receives_real_streamed_world_state) {
  test::ClientEpisode fixture;
  ASSERT_TRUE(fixture.Bootstrap());
  EXPECT_EQ(fixture.episode->GetState()->GetEpisodeId(), 1u);
}

TEST(episode_concurrency, map_change_has_one_consumer) {
  test::ClientEpisode fixture;
  EXPECT_TRUE(fixture.episode->HasMapChangedSinceLastCall());
  EXPECT_FALSE(fixture.episode->HasMapChangedSinceLastCall());

  ASSERT_TRUE(fixture.Bootstrap());
  fixture.episode->HasMapChangedSinceLastCall();

  const auto frame = fixture.server.Publish(true);
  ASSERT_TRUE(fixture.WaitForFrame(frame));
  constexpr size_t readers = 16u;
  std::latch ready(readers);
  std::latch release(1u);
  std::atomic_size_t consumed{0u};
  carla::ThreadGroup threads;
  threads.CreateThreads(readers, [&]() {
    ready.count_down();
    release.wait();
    if (fixture.episode->HasMapChangedSinceLastCall()) {
      ++consumed;
    }
  });
  ready.wait();
  release.count_down();
  threads.JoinAll();
  EXPECT_EQ(consumed.load(), 1u);
  EXPECT_FALSE(fixture.episode->HasMapChangedSinceLastCall());
}

TEST(episode_concurrency, new_episode_marks_map_dirty_before_state_is_visible) {
  test::ClientEpisode fixture;
  ASSERT_TRUE(fixture.Bootstrap());
  fixture.episode->HasMapChangedSinceLastCall();

  const auto ordinary_frame = fixture.server.Publish();
  ASSERT_TRUE(fixture.WaitForFrame(ordinary_frame));
  EXPECT_FALSE(fixture.episode->HasMapChangedSinceLastCall());

  const auto changed_frame = fixture.server.Publish(false, 2u);
  ASSERT_TRUE(test::WaitForCondition([&]() { return fixture.episode->GetId() == 2u; }));
  EXPECT_TRUE(fixture.episode->HasMapChangedSinceLastCall());
  EXPECT_FALSE(fixture.episode->HasMapChangedSinceLastCall());
  ASSERT_TRUE(fixture.WaitForFrame(changed_frame));
}

TEST(episode_concurrency, map_change_publication_races_consumers) {
  test::ClientEpisode fixture;
  ASSERT_TRUE(fixture.Bootstrap());
  fixture.episode->HasMapChangedSinceLastCall();
  std::atomic_bool done{false};
  std::atomic_size_t consumed{0u};
  carla::ThreadGroup threads;
  threads.CreateThreads(4u, [&]() {
    while (!done.load()) {
      if (fixture.episode->HasMapChangedSinceLastCall()) {
        ++consumed;
      }
      std::this_thread::yield();
    }
  });
  for (size_t i = 0u; i < 300u; ++i) {
    fixture.server.Publish(true);
    std::this_thread::sleep_for(1ms);
  }
  done = true;
  threads.JoinAll();
  EXPECT_GT(consumed.load(), 0u);
}

#ifndef LIBCARLA_NO_EXCEPTIONS
namespace {

  void ExpectPendingMessage(const std::string &message) {
    test::ClientEpisode fixture;
    ASSERT_TRUE(fixture.Bootstrap());
    fixture.episode->AddPendingException("previous pending message");
    fixture.episode->AddPendingException(message);
    std::atomic_bool received{false};
    std::thread waiter([&]() {
      const auto deadline = std::chrono::steady_clock::now() + 5s;
      while (!received.load() && std::chrono::steady_clock::now() < deadline) {
        try {
          fixture.episode->WaitForState(10ms);
        } catch (const carla::detail::SharedException &error) {
          EXPECT_STREQ(error.what(), message.c_str());
          received = true;
        } catch (const std::exception &error) {
          ADD_FAILURE() << error.what();
          break;
        }
      }
    });
    EXPECT_TRUE(test::WaitForCondition([&]() {
      if (received.load()) {
        return true;
      }
      fixture.episode->AddPendingException(message);
      fixture.server.Publish();
      return false;
    }));
    waiter.join();
  }

} // namespace

TEST(episode_concurrency, most_recent_pending_exception_is_delivered) {
  ExpectPendingMessage("latest pending message");
}

TEST(episode_concurrency, empty_pending_exception_is_delivered) {
  ExpectPendingMessage("");
}

TEST(episode_concurrency, pending_exception_publication_races_streaming) {
  test::ClientEpisode fixture;
  ASSERT_TRUE(fixture.Bootstrap());
  std::atomic_bool done{false};
  std::atomic_size_t received{0u};
  std::thread publisher([&]() {
    while (!done.load()) {
      fixture.server.Publish();
      std::this_thread::sleep_for(1ms);
    }
  });
  std::thread waiter([&]() {
    while (!done.load()) {
      try {
        fixture.episode->WaitForState(10ms);
      } catch (const carla::detail::SharedException &error) {
        EXPECT_EQ(std::string(error.what()), std::string(2048u, 'x'));
        ++received;
      }
    }
  });
  carla::ThreadGroup producers;
  producers.CreateThreads(4u, [&]() {
    for (size_t i = 0u; i < 1000u; ++i) {
      fixture.episode->AddPendingException(std::string(2048u, 'x'));
      std::this_thread::yield();
    }
  });
  producers.JoinAll();
  EXPECT_TRUE(test::WaitForCondition([&]() {
    if (received.load() > 0u) {
      return true;
    }
    fixture.episode->AddPendingException(std::string(2048u, 'x'));
    fixture.server.Publish();
    return false;
  }));
  done = true;
  publisher.join();
  waiter.join();
}
#endif // LIBCARLA_NO_EXCEPTIONS
