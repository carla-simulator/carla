// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/rpc/Command.h>

#include <type_traits>
#include <utility>

namespace {

using Command = carla::rpc::Command;

Command MakeSpawn(std::optional<carla::rpc::ActorId> parent) {
  Command::SpawnActor spawn;
  spawn.description.uid = 7u;
  spawn.description.id = "sensor.other.gnss";
  spawn.transform.location.x = 1.0f;
  spawn.parent = parent;
  return Command{spawn};
}

template <size_t Index>
void CheckActorCommand() {
  using Type = std::variant_alternative_t<Index, Command::CommandType>;
  if constexpr (!std::is_same_v<Type, Command::SpawnActor> &&
                !std::is_same_v<Type, Command::ConsoleCommand>) {
    Type value{};
    value.actor = 123u;
    Command command{value};
    command.ApplySpawnedActorId(42u);
    EXPECT_EQ(std::get<Type>(command.command).actor, 42u);
  }
}

template <size_t... Indices>
void CheckActorCommands(std::index_sequence<Indices...>) {
  (CheckActorCommand<Indices>(), ...);
}

} // namespace

TEST(command_spawn_actor, future_parent_is_replaced) {
  auto command = MakeSpawn(0u); // Python's command.FutureActor sentinel.
  command.ApplySpawnedActorId(42u);
  const auto &spawn = std::get<Command::SpawnActor>(command.command);
  ASSERT_TRUE(spawn.parent.has_value());
  EXPECT_EQ(*spawn.parent, 42u);
  EXPECT_EQ(spawn.description.uid, 7u);
  EXPECT_EQ(spawn.description.id, "sensor.other.gnss");
  EXPECT_EQ(spawn.transform.location.x, 1.0f);
}

TEST(command_spawn_actor, missing_parent_is_preserved) {
  auto command = MakeSpawn(std::nullopt);
  command.ApplySpawnedActorId(42u);
  EXPECT_FALSE(std::get<Command::SpawnActor>(command.command).parent.has_value());
}

TEST(command_spawn_actor, explicit_parent_is_preserved) {
  auto command = MakeSpawn(123u);
  command.ApplySpawnedActorId(42u);
  EXPECT_EQ(std::get<Command::SpawnActor>(command.command).parent, 123u);
}

TEST(command_spawn_actor, nested_future_parents_use_the_immediate_parent) {
  auto command = MakeSpawn(0u);
  auto &spawn = std::get<Command::SpawnActor>(command.command);
  spawn.do_after.push_back(MakeSpawn(0u));

  command.ApplySpawnedActorId(42u);
  EXPECT_EQ(spawn.parent, 42u);
  EXPECT_EQ(std::get<Command::SpawnActor>(spawn.do_after.front().command).parent, 0u);

  // The server resolves this command only after the intermediate actor spawns.
  spawn.do_after.front().ApplySpawnedActorId(63u);
  EXPECT_EQ(std::get<Command::SpawnActor>(spawn.do_after.front().command).parent, 63u);
}

TEST(command_spawn_actor, other_actor_commands_keep_the_replacement_contract) {
  CheckActorCommands(std::make_index_sequence<std::variant_size_v<Command::CommandType>>{});
}

TEST(command_spawn_actor, console_command_is_unchanged) {
  Command command{Command::ConsoleCommand{"stat fps"}};
  command.ApplySpawnedActorId(42u);
  EXPECT_EQ(std::get<Command::ConsoleCommand>(command.command).cmd, "stat fps");
}

TEST(command_spawn_actor, future_parent_survives_messagepack_and_then_resolves) {
  auto command = carla::MsgPack::UnPack<Command>(carla::MsgPack::Pack(MakeSpawn(0u)));
  EXPECT_EQ(std::get<Command::SpawnActor>(command.command).parent, 0u);
  command.ApplySpawnedActorId(42u);
  EXPECT_EQ(std::get<Command::SpawnActor>(command.command).parent, 42u);
}
