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

#include <thread>
#include <future>

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

  std::promise<void> completed;
  auto completion = completed.get_future();
  unsigned int completed_calls = 0u;

  carla::ThreadGroup threads;
  threads.CreateThread([&]() {
    try {
      Client client("localhost", port);
      client.set_timeout(5000);
      for (auto i = 0; i < 300; ++i) {
        auto result = client.call("do_the_thing", i, 1).as<int>();
        EXPECT_EQ(result, i + 1);
        ++completed_calls;
      }
      completed.set_value();
    } catch (...) {
      completed.set_exception(std::current_exception());
    }
  });

  const auto deadline = std::chrono::steady_clock::now() + 30s;
  auto i = 0u;
  while (completion.wait_for(0s) != std::future_status::ready &&
         std::chrono::steady_clock::now() < deadline) {
    server.SyncRunFor(2ms);
    std::this_thread::yield();
    ++i;
  }
  std::cout << "game thread: run " << i << " slices.\n";
  EXPECT_EQ(completion.wait_for(0s), std::future_status::ready);
  server.Stop();
  threads.JoinAll();
  EXPECT_NO_THROW(completion.get());
  EXPECT_EQ(completed_calls, 300u);
}
