// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/client/Client.h>
#include <carla/rpc/Metadata.h>
#include <carla/rpc/Response.h>
#include <carla/streaming/Server.h>

#include <rpc/server.h>

#include <map>
#include <mutex>

namespace {

using Command = carla::rpc::Command;
using ActorId = carla::rpc::ActorId;
using Response = carla::rpc::CommandResponse;

// Real public Client, actor lookup and remote TM transports, with inert server
// endpoints. No Unreal actors, simulation or TrafficManagerLocal are run here.
class BatchAutopilotServer {
public:
  BatchAutopilotServer()
    : streaming("127.0.0.1", 0u), stream(streaming.MakeStream()),
      server("127.0.0.1", 0u), first_tm("127.0.0.1", 0u), second_tm("127.0.0.1", 0u) {
    BindTM(first_tm);
    BindTM(second_tm);
    server.bind("version", [](carla::rpc::Metadata) {
      return carla::rpc::Response<std::string>{"batch-test"};
    });
    server.bind("get_episode_info", [this](carla::rpc::Metadata) {
      carla::rpc::EpisodeInfo info{};
      info.id = 1u;
      info.token = stream.token();
      return carla::rpc::Response<carla::rpc::EpisodeInfo>{info};
    });
    server.bind("get_sensor_token", [this](carla::rpc::Metadata, uint32_t) {
      return carla::rpc::Response<carla::streaming::Token>{stream.token()};
    });
    server.bind("get_episode_settings", [](carla::rpc::Metadata) {
      carla::rpc::EpisodeSettings settings{};
      settings.synchronous_mode = true;
      return carla::rpc::Response<carla::rpc::EpisodeSettings>{settings};
    });
    server.bind("query_lights_state", [](carla::rpc::Metadata, std::string) {
      return carla::rpc::Response<std::vector<carla::rpc::LightState>>{
          std::vector<carla::rpc::LightState>{}};
    });
    server.bind("update_lights_state", [](carla::rpc::Metadata, std::string,
        std::vector<carla::rpc::LightState>, bool) {});
    server.bind("get_actors_by_id", [this](carla::rpc::Metadata, std::vector<ActorId> ids) {
      ++actor_queries;
      std::vector<carla::rpc::Actor> actors;
      for (const auto id : ids) {
        if (id != missing_actor) {
          carla::rpc::Actor actor{};
          actor.id = id;
          actor.description.id = "vehicle.batch-test";
          actors.push_back(actor);
        }
      }
      return carla::rpc::Response<std::vector<carla::rpc::Actor>>{actors};
    });
    server.bind("is_traffic_manager_running", [this](carla::rpc::Metadata, uint16_t port) {
      return carla::rpc::Response<bool>{port == FirstPort() || port == SecondPort()};
    });
    server.bind("get_traffic_manager_running", [](carla::rpc::Metadata, uint16_t port) {
      return carla::rpc::Response<std::pair<std::string, uint16_t>>{
          std::make_pair(std::string("127.0.0.1"), port)};
    });
    server.bind("apply_batch", [this](carla::rpc::Metadata, std::vector<Command>, bool tick) {
      batch_tick = tick;
      return std::vector<Response>(responses.cbegin(), responses.cend());
    });
    streaming.AsyncRun(1u);
    server.async_run(1u);
    first_tm.async_run(1u);
    second_tm.async_run(1u);
  }

  ~BatchAutopilotServer() {
    // Join the production remote-TM keep-alive workers before their endpoints
    // and the client's borrowed episode go away.
    carla::traffic_manager::TrafficManager::Release();
    second_tm.stop();
    first_tm.stop();
    server.stop();
  }

  uint16_t FirstPort() const { return first_tm.port(); }
  uint16_t SecondPort() const { return second_tm.port(); }
  uint16_t RpcPort() const { return server.port(); }

  carla::client::Client Client() {
    return carla::client::Client("127.0.0.1", server.port(), 1u);
  }

  std::vector<ActorId> Enabled(uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex);
    return enabled[port];
  }

  std::vector<ActorId> Disabled(uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex);
    return disabled[port];
  }

  std::vector<Response> responses;
  ActorId missing_actor = 0u;
  std::atomic_uint actor_queries{0u};
  bool batch_tick = true;

private:
  void BindTM(::rpc::server &tm) {
    const auto port = tm.port();
    tm.bind("health_check_remote_TM", []() {});
    tm.bind("register_vehicle", [this, port](std::vector<carla::rpc::Actor> actors) {
      std::lock_guard<std::mutex> lock(mutex);
      for (const auto &actor : actors) {
        enabled[port].push_back(actor.id);
      }
    });
    tm.bind("unregister_vehicle", [this, port](std::vector<carla::rpc::Actor> actors) {
      std::lock_guard<std::mutex> lock(mutex);
      for (const auto &actor : actors) {
        disabled[port].push_back(actor.id);
      }
    });
  }

  carla::streaming::Server streaming;
  carla::streaming::Stream stream;
  ::rpc::server server;
  ::rpc::server first_tm;
  ::rpc::server second_tm;
  std::mutex mutex;
  std::map<uint16_t, std::vector<ActorId>> enabled;
  std::map<uint16_t, std::vector<ActorId>> disabled;
};

} // namespace

TEST(batch_autopilot, cpp_spawn_followup_registers_returned_actor) {
  BatchAutopilotServer server;
  auto client = server.Client();
  server.responses = {Response{42u}};
  ASSERT_EQ(server.responses[0].Get(), 42u);
  Command::SpawnActor spawn({}, {});
  spawn.do_after.emplace_back(Command::SetAutopilot(0u, true, server.FirstPort()));
  const auto responses = client.ApplyBatchSync({spawn});
  ASSERT_EQ(responses.size(), 1u);
  EXPECT_EQ(responses[0].Get(), 42u);
  EXPECT_EQ(server.Enabled(server.FirstPort()), (std::vector<ActorId>{42u}));
  EXPECT_FALSE(server.batch_tick);
  // Prevent the fixture's release from outliving the client's episode.
  carla::traffic_manager::TrafficManager::Release();
}

TEST(batch_autopilot, standalone_commands_keep_their_own_tm_ports) {
  BatchAutopilotServer server;
  auto client = server.Client();
  server.responses = {Response{30u}, Response{10u}, Response{20u}};
  client.ApplyBatchSync({Command::SetAutopilot(30u, true, server.FirstPort()),
      Command::SetAutopilot(10u, true, server.SecondPort()),
      Command::SetAutopilot(20u, false, server.FirstPort())});
  EXPECT_EQ(server.Enabled(server.FirstPort()), (std::vector<ActorId>{30u}));
  EXPECT_EQ(server.Enabled(server.SecondPort()), (std::vector<ActorId>{10u}));
  EXPECT_EQ(server.Disabled(server.FirstPort()), (std::vector<ActorId>{20u}));
  EXPECT_EQ(server.actor_queries.load(), 1u);
  carla::traffic_manager::TrafficManager::Release();
}

TEST(batch_autopilot, last_state_per_actor_and_port_wins_and_ids_are_sorted) {
  BatchAutopilotServer server;
  auto client = server.Client();
  server.responses = {Response{30u}, Response{10u}, Response{30u}, Response{20u}};
  client.ApplyBatchSync({Command::SetAutopilot(30u, false, server.FirstPort()),
      Command::SetAutopilot(10u, true, server.FirstPort()),
      Command::SetAutopilot(30u, true, server.FirstPort()),
      Command::SetAutopilot(20u, true, server.FirstPort())});
  EXPECT_EQ(server.Enabled(server.FirstPort()), (std::vector<ActorId>{10u, 20u, 30u}));
  EXPECT_TRUE(server.Disabled(server.FirstPort()).empty());
  carla::traffic_manager::TrafficManager::Release();
}

TEST(batch_autopilot, spawn_followups_preserve_each_tm_port_and_final_state) {
  BatchAutopilotServer server;
  auto client = server.Client();
  server.responses = {Response{42u}};
  Command::SpawnActor spawn({}, {});
  spawn.do_after.emplace_back(Command::SetAutopilot(0u, true, server.FirstPort()));
  spawn.do_after.emplace_back(Command::SetAutopilot(0u, true, server.SecondPort()));
  spawn.do_after.emplace_back(Command::SetAutopilot(0u, false, server.FirstPort()));
  client.ApplyBatchSync({spawn});
  EXPECT_TRUE(server.Enabled(server.FirstPort()).empty());
  EXPECT_EQ(server.Disabled(server.FirstPort()), (std::vector<ActorId>{42u}));
  EXPECT_EQ(server.Enabled(server.SecondPort()), (std::vector<ActorId>{42u}));
  carla::traffic_manager::TrafficManager::Release();
}

TEST(batch_autopilot, large_mixed_port_batch_fetches_actors_once_and_sorts_each_group) {
  BatchAutopilotServer server;
  auto client = server.Client();
  std::vector<Command> commands;
  std::vector<ActorId> first, second;
  for (ActorId id = 151u; id != 0u; --id) {
    const auto port = id % 2u ? server.FirstPort() : server.SecondPort();
    commands.emplace_back(Command::SetAutopilot(id, true, port));
    server.responses.emplace_back(id);
    (id % 2u ? first : second).insert((id % 2u ? first : second).begin(), id);
  }
  client.ApplyBatchSync(std::move(commands));
  EXPECT_EQ(server.Enabled(server.FirstPort()), first);
  EXPECT_EQ(server.Enabled(server.SecondPort()), second);
  EXPECT_EQ(server.actor_queries.load(), 1u);
  carla::traffic_manager::TrafficManager::Release();
}

TEST(batch_autopilot, failed_responses_and_absent_actors_are_ignored) {
  BatchAutopilotServer server;
  auto client = server.Client();
  server.responses = {Response{carla::rpc::ResponseError{"spawn failed"}}, Response{99u}};
  server.missing_actor = 99u;
  const auto responses = client.ApplyBatchSync({Command::SetAutopilot(1u, true, server.FirstPort()),
      Command::SetAutopilot(99u, false, server.SecondPort())});
  ASSERT_TRUE(responses[0].HasError());
  EXPECT_EQ(responses[0].GetError().What(), "spawn failed");
  EXPECT_TRUE(server.Enabled(server.FirstPort()).empty());
  EXPECT_TRUE(server.Disabled(server.SecondPort()).empty());
}

TEST(batch_autopilot, non_autopilot_batch_does_not_bootstrap_an_episode) {
  BatchAutopilotServer server;
  auto client = server.Client();
  server.responses = {Response{5u}};
  const auto responses = client.ApplyBatchSync({Command::DestroyActor(5u)});
  EXPECT_EQ(responses[0].Get(), 5u);
  EXPECT_EQ(server.actor_queries.load(), 0u);
}
