// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/LightManager.h"
#include "carla/Version.h"
#include "carla/client/detail/Simulator.h"
#include "carla/rpc/Metadata.h"
#include "carla/rpc/Response.h"
#include "carla/streaming/Server.h"

#include <rpc/server.h>

namespace {

  // Real Simulator, LightManager and callback storage; no simulation ticks.
  class LightCallbackServer {
  public:
    LightCallbackServer()
      : _streaming("127.0.0.1", 0u),
        _stream(_streaming.MakeStream()),
        _rpc("127.0.0.1", 0u) {
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
      _streaming.AsyncRun(1u);
      _rpc.async_run(1u);
    }

    ~LightCallbackServer() {
      _rpc.stop();
    }

    std::shared_ptr<carla::client::detail::Simulator> Simulator() const {
      auto simulator = std::make_shared<carla::client::detail::Simulator>(
          "127.0.0.1", _rpc.port(), 1u, false);
      simulator->GetCurrentEpisode();
      return simulator;
    }

  private:
    carla::streaming::Server _streaming;
    carla::streaming::Stream _stream;
    ::rpc::server _rpc;
  };

  auto RetainedCallback(std::weak_ptr<int> &observed) {
    auto state = std::make_shared<int>(42);
    observed = state;
    return [state](carla::client::WorldSnapshot) { (void)state; };
  }

} // namespace

TEST(light_manager_callbacks, destruction_preserves_prior_user_tick_callback) {
  LightCallbackServer server;
  auto simulator = server.Simulator();
  std::weak_ptr<int> observed;
  const auto id = simulator->RegisterOnTickEvent(RetainedCallback(observed));
  {
    carla::client::LightManager manager;
    manager.SetEpisode(carla::client::detail::WeakEpisodeProxy{simulator});
  }
  // Inspect retained callback ownership without invoking destroyed objects.
  EXPECT_FALSE(observed.expired());
  simulator->RemoveOnTickEvent(id);
  EXPECT_TRUE(observed.expired());
}

TEST(light_manager_callbacks, later_user_tick_callback_control) {
  LightCallbackServer server;
  auto simulator = server.Simulator();
  std::weak_ptr<int> observed;
  size_t id;
  {
    carla::client::LightManager manager;
    manager.SetEpisode(carla::client::detail::WeakEpisodeProxy{simulator});
    id = simulator->RegisterOnTickEvent(RetainedCallback(observed));
  }
  EXPECT_FALSE(observed.expired());
  simulator->RemoveOnTickEvent(id);
  EXPECT_TRUE(observed.expired());
}

TEST(light_manager_callbacks, unrelated_light_update_callback_control) {
  LightCallbackServer server;
  auto simulator = server.Simulator();
  std::weak_ptr<int> observed;
  size_t id;
  {
    carla::client::LightManager manager;
    manager.SetEpisode(carla::client::detail::WeakEpisodeProxy{simulator});
    id = simulator->RegisterLightUpdateChangeEvent(RetainedCallback(observed));
  }
  EXPECT_FALSE(observed.expired());
  simulator->RemoveLightUpdateChangeEvent(id);
  EXPECT_TRUE(observed.expired());
}
