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
#include <utility>

using namespace carla::rpc;
using namespace std::chrono_literals;

TEST(rpc, mutable_response_copy_preserves_actor_id) {
  Response<ActorId> original{42u};
  const auto &constant = original;
  const Response<ActorId> constant_copy(constant);
  ASSERT_FALSE(constant_copy.HasError());
  EXPECT_EQ(constant_copy.Get(), 42u);

  Response<ActorId> copy(original);
  ASSERT_FALSE(copy.HasError());
  EXPECT_EQ(copy.Get(), 42u);
  EXPECT_EQ(original.Get(), 42u);

  Response<ActorId> moved(std::move(copy));
  ASSERT_FALSE(moved.HasError());
  EXPECT_EQ(moved.Get(), 42u);
}

TEST(rpc, mutable_response_copy_preserves_error) {
  Response<ActorId> original{ResponseError{"spawn failed"}};
  const auto &constant = original;
  const Response<ActorId> constant_copy(constant);
  ASSERT_TRUE(constant_copy.HasError());
  EXPECT_EQ(constant_copy.GetError().What(), "spawn failed");

  Response<ActorId> copy(original);
  ASSERT_TRUE(copy.HasError());
  EXPECT_EQ(copy.GetError().What(), "spawn failed");
  EXPECT_EQ(original.GetError().What(), "spawn failed");

  Response<ActorId> assigned{7u};
  assigned = original;
  ASSERT_TRUE(assigned.HasError());
  EXPECT_EQ(assigned.GetError().What(), "spawn failed");
}

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
