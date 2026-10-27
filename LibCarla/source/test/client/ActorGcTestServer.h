// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/Version.h>
#include <carla/client/Actor.h>
#include <carla/client/ActorBlueprint.h>
#include <carla/client/detail/Simulator.h>
#include <carla/rpc/Metadata.h>
#include <carla/rpc/Response.h>
#include <carla/streaming/Server.h>

#include <rpc/server.h>

#include <atomic>
#include <string>
#include <memory>

namespace actor_gc_test {

  class ActorServer {
  public:
    explicit ActorServer(bool destroy_result = true)
      : _destroy_result(destroy_result), _streaming("127.0.0.1", 0u),
        _stream(_streaming.MakeStream()), _rpc("127.0.0.1", 0u) {
      _rpc.bind("version", [](carla::rpc::Metadata) {
        return carla::rpc::Response<std::string>{std::string(carla::version())};
      });
      _rpc.bind("get_episode_info", [this](carla::rpc::Metadata) {
        carla::rpc::EpisodeInfo info{};
        info.id = 1u;
        info.token = _stream.token();
        return carla::rpc::Response<carla::rpc::EpisodeInfo>{info};
      });
      _rpc.bind("get_sensor_token", [this](carla::rpc::Metadata, uint32_t) {
        return carla::rpc::Response<carla::streaming::Token>{_stream.token()};
      });
      _rpc.bind("get_episode_settings", [](carla::rpc::Metadata) {
        carla::rpc::EpisodeSettings settings{};
        settings.synchronous_mode = true;
        return carla::rpc::Response<carla::rpc::EpisodeSettings>{settings};
      });
      _rpc.bind("query_lights_state", [](carla::rpc::Metadata, std::string) {
        return carla::rpc::Response<std::vector<carla::rpc::LightState>>{
            std::vector<carla::rpc::LightState>{}};
      });
      _rpc.bind("spawn_actor", [this](carla::rpc::Metadata, carla::rpc::ActorDescription description,
          carla::geom::Transform) {
        carla::rpc::Actor actor;
        actor.id = _next_actor_id++;
        actor.description = std::move(description);
        return carla::rpc::Response<carla::rpc::Actor>{std::move(actor)};
      });
      _rpc.bind("destroy_actor", [this](carla::rpc::Metadata, uint32_t) {
        ++_destroy_count;
        return carla::rpc::Response<bool>{bool(_destroy_result)};
      });
      _streaming.AsyncRun(1u);
      _rpc.async_run(1u);
    }

    ~ActorServer() {
      _rpc.stop();
    }

    auto MakeSimulator() const {
      auto simulator = std::make_shared<carla::client::detail::Simulator>(
          "127.0.0.1", _rpc.port(), 1u, false);
      simulator->GetCurrentEpisode();
      return simulator;
    }

    size_t DestroyCount() const {
      return _destroy_count.load();
    }

  private:
    bool _destroy_result;
    std::atomic<uint32_t> _next_actor_id{42u};
    std::atomic<size_t> _destroy_count{0u};
    carla::streaming::Server _streaming;
    carla::streaming::Stream _stream;
    ::rpc::server _rpc;
  };

} // namespace actor_gc_test
