// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/MsgPackAdaptors.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/Command.h>
#include <carla/rpc/CommandResponse.h>
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

namespace {
  // rpc::Actor as it was before spawn_frame was appended.
  struct ActorWithoutSpawnFrame {
    carla::rpc::ActorId id = 0u;
    carla::rpc::ActorId parent_id = 0u;
    ActorDescription description;
    carla::geom::BoundingBox bounding_box;
    std::vector<uint8_t> semantic_tags;
    std::vector<unsigned char> stream_token;
    MSGPACK_DEFINE_ARRAY(id, parent_id, description, bounding_box, semantic_tags, stream_token);
  };
} // namespace

TEST(msgpack, actor_spawn_frame_is_compatible_both_ways) {
  namespace c = carla;
  Actor actor;
  actor.id = 42u;
  actor.parent_id = 7u;
  actor.description.id = "vehicle.tesla.model3";
  actor.spawn_frame = 123456u;

  auto round_trip = c::MsgPack::UnPack<Actor>(c::MsgPack::Pack(actor));
  ASSERT_EQ(round_trip.spawn_frame, actor.spawn_frame);

  // A new client reading an old server's reply: no spawn_frame, defaults to 0.
  ActorWithoutSpawnFrame old_actor;
  old_actor.id = 42u;
  old_actor.parent_id = 7u;
  old_actor.description.id = "vehicle.tesla.model3";
  auto from_old = c::MsgPack::UnPack<Actor>(c::MsgPack::Pack(old_actor));
  ASSERT_EQ(from_old.id, old_actor.id);
  ASSERT_EQ(from_old.parent_id, old_actor.parent_id);
  ASSERT_EQ(from_old.description.id, old_actor.description.id);
  ASSERT_EQ(from_old.spawn_frame, 0u);

  // An old client reading a new server's reply ignores the extra field.
  auto from_new = c::MsgPack::UnPack<ActorWithoutSpawnFrame>(c::MsgPack::Pack(actor));
  ASSERT_EQ(from_new.id, actor.id);
  ASSERT_EQ(from_new.parent_id, actor.parent_id);
  ASSERT_EQ(from_new.description.id, actor.description.id);
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

// Every alternative of rpc::Command decodes back into the same alternative,
// with the same contents (packing it again gives the same bytes).
template <size_t I>
static void CheckCommandAlternative() {
  using mp = carla::MsgPack;
  using CommandType = Command::CommandType;
  Command command;
  command.command.template emplace<I>();
  const auto bytes = mp::Pack(command);
  const auto decoded = mp::UnPack<Command>(bytes);
  ASSERT_EQ(decoded.command.index(), I);
  const auto again = mp::Pack(decoded);
  ASSERT_EQ(std::string(reinterpret_cast<const char *>(again.data()), again.size()),
            std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size()));
  static_assert(I < std::variant_size_v<CommandType>);
}

template <size_t... Is>
static void CheckCommandAlternatives(std::index_sequence<Is...>) {
  (CheckCommandAlternative<Is>(), ...);
}

TEST(msgpack, command_variant_round_trip) {
  using mp = carla::MsgPack;
  CheckCommandAlternatives(
      std::make_index_sequence<std::variant_size_v<Command::CommandType>>());

  // Alternatives with contents, including a nested batch.
  const carla::geom::Transform transform{
      carla::geom::Location{1.5f, -2.25f, 3.0f}, carla::geom::Rotation{10.0f, -20.0f, 30.5f}};
  ActorDescription description;
  description.uid = 7u;
  description.id = "vehicle.test.car";
  Command::SpawnActor spawn(description, transform, 42u);
  spawn.do_after.push_back(Command{Command::SetSimulatePhysics(0u, false)});
  spawn.do_after.push_back(Command{Command::SetAutopilot(0u, true, 8001u)});
  const std::vector<Command> batch = {
      Command{spawn},
      Command{Command::ApplyTransform(5u, transform)},
      Command{Command::ConsoleCommand("stat fps")},
  };
  const auto bytes = mp::Pack(batch);
  const auto decoded = mp::UnPack<std::vector<Command>>(bytes);
  ASSERT_EQ(decoded.size(), batch.size());
  const auto &decoded_spawn = std::get<Command::SpawnActor>(decoded[0].command);
  ASSERT_EQ(decoded_spawn.description.id, "vehicle.test.car");
  ASSERT_EQ(decoded_spawn.description.uid, 7u);
  ASSERT_TRUE(decoded_spawn.parent.has_value());
  ASSERT_EQ(*decoded_spawn.parent, 42u);
  ASSERT_EQ(decoded_spawn.do_after.size(), 2u);
  ASSERT_TRUE(std::get<Command::SetAutopilot>(decoded_spawn.do_after[1].command).enabled);
  const auto &decoded_transform = std::get<Command::ApplyTransform>(decoded[1].command);
  ASSERT_EQ(decoded_transform.actor, 5u);
  ASSERT_EQ(decoded_transform.transform, transform);
  ASSERT_EQ(std::get<Command::ConsoleCommand>(decoded[2].command).cmd, "stat fps");
}

// Copying a Response from a non-const lvalue keeps its value. It used to go
// through the forwarding constructor and Response's operator bool, so every
// copied value became 1 (or 0 for an error).
TEST(msgpack, response_copy_keeps_value) {
  CommandResponse value(ActorId(328u));
  CommandResponse copy(value);
  ASSERT_FALSE(copy.HasError());
  ASSERT_EQ(copy.Get(), 328u);
  CommandResponse error(ResponseError("collision"));
  CommandResponse error_copy(error);
  ASSERT_TRUE(error_copy.HasError());
  ASSERT_EQ(error_copy.GetError().What(), std::string("collision"));
  std::vector<CommandResponse> responses{value, error};
  std::vector<CommandResponse> responses_copy(responses.begin(), responses.end());
  ASSERT_EQ(responses_copy[0].Get(), 328u);
  ASSERT_TRUE(responses_copy[1].HasError());
}

// What apply_batch_and_tick sends back: responses nested in a pair nested in
// a response. Every level is a variant whose index must not leak into a value.
TEST(msgpack, batch_and_tick_response_round_trip) {
  using mp = carla::MsgPack;
  using Result = Response<std::pair<std::vector<CommandResponse>, uint64_t>>;
  std::vector<CommandResponse> responses;
  responses.emplace_back(ActorId(328u));
  responses.emplace_back(ResponseError("collision"));
  responses.emplace_back(ActorId(4242u));
  const Result sent{std::make_pair(responses, uint64_t(10129u))};
  const auto received = mp::UnPack<Result>(mp::Pack(sent));
  ASSERT_FALSE(received.HasError());
  const auto &[decoded, frame] = received.Get();
  ASSERT_EQ(frame, 10129u);
  ASSERT_EQ(decoded.size(), 3u);
  ASSERT_FALSE(decoded[0].HasError());
  ASSERT_EQ(decoded[0].Get(), 328u);
  ASSERT_TRUE(decoded[1].HasError());
  ASSERT_EQ(decoded[1].GetError().What(), std::string("collision"));
  ASSERT_FALSE(decoded[2].HasError());
  ASSERT_EQ(decoded[2].Get(), 4242u);
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
