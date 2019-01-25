// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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

using namespace carla::rpc;

TEST(rpc, compilation_tests) {
  Server server(TESTING_PORT);
  server.BindSync("bind00", []() { return 2.0f; });
  server.BindSync("bind01", [](int x) { return x; });
  server.BindSync("bind02", [](int, float) { return 0.0; });
  server.BindSync("bind03", [](int, float, double, char) {});
}

TEST(rpc, server_bind_sync_run_on_game_thread) {
  const auto main_thread_id = std::this_thread::get_id();

  Server server(TESTING_PORT);

  server.BindSync("do_the_thing", [=](int x, int y) -> int {
    EXPECT_EQ(std::this_thread::get_id(), main_thread_id);
    return x + y;
  });

  server.AsyncRun(1u);

  std::atomic_bool done{false};

  carla::ThreadGroup threads;
  threads.CreateThread([&]() {
    Client client("localhost", TESTING_PORT);
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

TEST(rpc, response) {
  using mp = carla::MsgPack;
  const std::string error = "Uh ah an error!";
  Response<int> r = ResponseError(error);
  auto s = mp::UnPack<decltype(r)>(mp::Pack(r));
  ASSERT_TRUE(r.HasError());
  ASSERT_EQ(r.GetError().What(), error);
  ASSERT_TRUE(s.HasError());
  ASSERT_EQ(s.GetError().What(), error);
  r.Reset(42);
  s = mp::UnPack<decltype(r)>(mp::Pack(r));
  ASSERT_FALSE(r.HasError());
  ASSERT_EQ(r.Get(), 42);
  ASSERT_FALSE(s.HasError());
  ASSERT_EQ(s.Get(), 42);
  r.SetError(error);
  s = mp::UnPack<decltype(r)>(mp::Pack(r));
  ASSERT_FALSE(r);
  ASSERT_EQ(r.GetError().What(), error);
  ASSERT_FALSE(s);
  ASSERT_EQ(s.GetError().What(), error);
  Response<std::vector<float>> rv;
  auto sv = mp::UnPack<decltype(rv)>(mp::Pack(rv));
  ASSERT_TRUE(rv.HasError());
  ASSERT_TRUE(sv.HasError());
  Response<void> er;
  ASSERT_TRUE(er.HasError());
  er = Response<void>::Success();
  auto es = mp::UnPack<decltype(er)>(mp::Pack(er));
  ASSERT_FALSE(er.HasError());
  ASSERT_FALSE(es.HasError());
  er.SetError(error);
  es = mp::UnPack<decltype(er)>(mp::Pack(er));
  ASSERT_FALSE(er);
  ASSERT_EQ(er.GetError().What(), error);
  ASSERT_FALSE(es);
  ASSERT_EQ(es.GetError().What(), error);
}

TEST(rpc, msgpack) {
  namespace c = carla;
  namespace cg = carla::geom;
  Actor actor;
  actor.id = 42u;
  actor.description.uid = 2u;
  actor.description.id = "actor.random.whatever";
  actor.bounding_box = cg::BoundingBox{cg::Vector3D{1.0f, 2.0f, 3.0f}};

  auto buffer = c::MsgPack::Pack(actor);
  auto result = c::MsgPack::UnPack<Actor>(buffer);

  ASSERT_EQ(result.id, actor.id);
  ASSERT_EQ(result.description.uid, actor.description.uid);
  ASSERT_EQ(result.description.id, actor.description.id);
  ASSERT_EQ(result.bounding_box, actor.bounding_box);
}

TEST(rpc, msgpack_variant) {
  using mp = carla::MsgPack;

  boost::variant<bool, float, std::string> var;

  var = true;
  auto result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.which(), 0);
  ASSERT_EQ(boost::get<bool>(result), true);

  var = 42.0f;
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.which(), 1);
  ASSERT_EQ(boost::get<float>(result), 42.0f);

  var = std::string("hola!");
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.which(), 2);
  ASSERT_EQ(boost::get<std::string>(result), "hola!");
}
