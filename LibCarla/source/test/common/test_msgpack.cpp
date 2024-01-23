// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/MsgPackAdaptors.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/Response.h>

#include <thread>

using namespace carla::rpc;

TEST(msgpack, response) {
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

TEST(msgpack, actor) {
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

TEST(msgpack, variant) {
  using mp = carla::MsgPack;

  std::variant<bool, float, std::string> var;

  var = true;
  auto result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.index(), 0);
  ASSERT_EQ(std::get<bool>(result), true);

  var = 42.0f;
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.index(), 1);
  ASSERT_EQ(std::get<float>(result), 42.0f);

  var = std::string("hola!");
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.index(), 2);
  ASSERT_EQ(std::get<std::string>(result), "hola!");
}

TEST(msgpack, optional) {
  using mp = carla::MsgPack;

  std::optional<float> var;

  auto result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_FALSE(result.has_value());

  var = 42.0f;
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(*result, 42.0f);
}
