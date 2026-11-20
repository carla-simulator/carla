// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/MsgPackAdaptors.h>
#include <carla/ThreadGroup.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/Client.h>
#include <carla/rpc/Response.h>
#include <carla/rpc/Server.h>

#include <chrono>
#include <iostream>
#include <thread>

using namespace carla::rpc;
using namespace std::chrono_literals;

TEST(rpc, compilation_tests) {
  Server server(TESTING_PORT);
  server.BindSync("bind00", []() { return 2.0f; });
  server.BindSync("bind01", [](int x) { return x; });
  server.BindSync("bind02", [](int, float) { return 0.0; });
  server.BindSync("bind03", [](int, float, double, char) {});
}

TEST(rpc, server_bind_sync_run_on_game_thread) {
  const auto main_thread_id = std::this_thread::get_id();

  const uint16_t port = (TESTING_PORT != 0u ? TESTING_PORT : 2017u);

  Server server(port);

  server.BindSync("do_the_thing", [=](int x, int y) -> int {
    EXPECT_EQ(std::this_thread::get_id(), main_thread_id);
    return x + y;
  });

  server.AsyncRun(1u);

  std::atomic_bool done{false};

  carla::ThreadGroup threads;
  threads.CreateThread([&]() {
    Client client("127.0.0.1", port);
    for (auto i = 0; i < 300; ++i) {
      auto result = client.call("do_the_thing", i, 1).as<int>();
      EXPECT_EQ(result, i + 1);
    }
    done = true;
  });

  auto i = 0u;
  for (; i < 1'000'000u; ++i) {
    server.SyncRunFor(2ms);
    if (done) {
      break;
    }
  }
  std::cout << "game thread: run " << i << " slices.\n";
  ASSERT_TRUE(done);
}

TEST(rpc, fence_orders_fire_and_forget_before_a_waiting_call) {
  const uint16_t port = (TESTING_PORT != 0u ? TESTING_PORT : 2017u);

  Server server(port);

  // Game thread only: no lock needed.
  size_t applied = 0u;
  server.BindSync("apply", [&](int) { ++applied; });
  server.BindSync("tick", [&]() -> uint64_t { return applied; });

  // Several RPC threads, so that one connection's messages are dispatched
  // concurrently, as on the CARLA server.
  server.AsyncRun(4u);

  constexpr size_t kTicks = 200u;
  constexpr size_t kCallsPerTick = 25u;

  std::atomic_bool done{false};
  std::atomic_bool failed{false};
  std::atomic_size_t late{0u};

  carla::ThreadGroup threads;
  threads.CreateThread([&]() {
    try {
      Client client("127.0.0.1", port);
      client.set_timeout(10000);
      for (size_t tick = 1u; tick <= kTicks; ++tick) {
        for (size_t i = 0u; i < kCallsPerTick; ++i) {
          client.async_call("apply", static_cast<int>(i));
        }
        // Every fire-and-forget call made so far is applied before the tick.
        const auto seen = client.call("tick").as<uint64_t>();
        if (seen != tick * kCallsPerTick) {
          ++late;
        }
      }
    } catch (const std::exception &e) {
      std::cerr << "client failed: " << e.what() << '\n';
      failed = true;
    }
    done = true;
  });

  // SyncRunFor returns at once when no task is queued, so bound the game
  // thread loop by time, not by iterations.
  const auto deadline = std::chrono::steady_clock::now() + 60s;
  while (!done && (std::chrono::steady_clock::now() < deadline)) {
    server.SyncRunFor(1ms);
  }
  ASSERT_TRUE(done);
  ASSERT_FALSE(failed);
  ASSERT_EQ(late.load(), 0u);
  ASSERT_EQ(applied, kTicks * kCallsPerTick);
}
