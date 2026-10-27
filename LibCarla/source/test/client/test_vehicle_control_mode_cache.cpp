// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Version.h>
#include <carla/client/Vehicle.h>
#include <carla/client/detail/Simulator.h>
#include <carla/rpc/Metadata.h>
#include <carla/rpc/Response.h>
#include <carla/streaming/Server.h>

#include <rpc/server.h>

#include <mutex>
#include <vector>

namespace {

  // Real Simulator, actor factory, Vehicle and RPC transport. The endpoints
  // record commands; this component fixture does not simulate vehicle dynamics.
  class ControlServer {
  public:
    ControlServer()
      : _streaming("127.0.0.1", 0u), _stream(_streaming.MakeStream()), _rpc("127.0.0.1", 0u) {
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
      _rpc.bind("apply_control_to_vehicle", [this](carla::rpc::Metadata, uint32_t id,
          carla::rpc::VehicleControl control) {
        std::lock_guard<std::mutex> lock(_mutex);
        _manual.push_back(control);
        _actor_ids.push_back(id);
        return carla::rpc::Response<void>::Success();
      });
      _rpc.bind("apply_ackermann_control_to_vehicle", [this](carla::rpc::Metadata, uint32_t id,
          carla::rpc::VehicleAckermannControl control) {
        std::lock_guard<std::mutex> lock(_mutex);
        _ackermann.push_back(control);
        _actor_ids.push_back(id);
        return carla::rpc::Response<void>::Success();
      });
      _streaming.AsyncRun(1u);
      _rpc.async_run(1u);
    }

    ~ControlServer() {
      _rpc.stop();
    }

    std::shared_ptr<carla::client::detail::Simulator> MakeSimulator() const {
      auto simulator = std::make_shared<carla::client::detail::Simulator>(
          "127.0.0.1", _rpc.port(), 1u, false);
      simulator->GetCurrentEpisode();
      return simulator;
    }

    auto MakeVehicle(const std::shared_ptr<carla::client::detail::Simulator> &simulator, bool sticky = true) {
      carla::rpc::Actor actor{};
      actor.id = 42u;
      actor.description.id = "vehicle.control-cache-test";
      carla::rpc::ActorAttributeValue attribute;
      attribute.id = "sticky_control";
      attribute.type = carla::rpc::ActorAttributeType::Bool;
      attribute.value = sticky ? "true" : "false";
      actor.description.attributes.push_back(attribute);
      return std::static_pointer_cast<carla::client::Vehicle>(simulator->MakeActor(
          actor, carla::client::GarbageCollectionPolicy::Disabled));
    }

    auto Manual() {
      std::lock_guard<std::mutex> lock(_mutex);
      return _manual;
    }

    auto Ackermann() {
      std::lock_guard<std::mutex> lock(_mutex);
      return _ackermann;
    }

  private:
    std::mutex _mutex;
    std::vector<carla::rpc::VehicleControl> _manual;
    std::vector<carla::rpc::VehicleAckermannControl> _ackermann;
    std::vector<uint32_t> _actor_ids;
    carla::streaming::Server _streaming;
    carla::streaming::Stream _stream;
    ::rpc::server _rpc;
  };

  carla::rpc::VehicleControl ManualControl(float throttle = 0.5f) {
    carla::rpc::VehicleControl control;
    control.throttle = throttle;
    control.steer = 0.2f;
    return control;
  }

  carla::rpc::VehicleAckermannControl AckermannControl() {
    carla::rpc::VehicleAckermannControl control;
    control.speed = 10.0f;
    control.steer = 0.1f;
    return control;
  }

  class VehicleControlCache : public testing::Test {
  protected:
    ControlServer server;
    std::shared_ptr<carla::client::detail::Simulator> simulator = server.MakeSimulator();
    carla::SharedPtr<carla::client::Vehicle> vehicle = server.MakeVehicle(simulator);

    void Barrier() {
      // The same RPC connection and one server worker process this request
      // after preceding control requests, without sleeps or simulation frames.
      EXPECT_EQ(simulator->GetServerVersion(), carla::version());
    }
  };

  TEST_F(VehicleControlCache, repeated_manual_command_reenters_after_ackermann) {
    const auto control = ManualControl();
    vehicle->ApplyControl(control);
    vehicle->ApplyAckermannControl(AckermannControl());
    vehicle->ApplyControl(control);
    Barrier();
    ASSERT_EQ(server.Manual().size(), 2u);
    EXPECT_EQ(server.Manual().back(), control);
    ASSERT_EQ(server.Ackermann().size(), 1u);
    EXPECT_EQ(server.Ackermann().front(), AckermannControl());
  }

  TEST_F(VehicleControlCache, default_manual_command_reenters_after_ackermann) {
    const carla::rpc::VehicleControl control{};
    vehicle->ApplyControl(control);
    vehicle->ApplyAckermannControl(AckermannControl());
    vehicle->ApplyControl(control);
    Barrier();
    ASSERT_EQ(server.Manual().size(), 2u);
    EXPECT_EQ(server.Manual().back(), control);
  }

  TEST_F(VehicleControlCache, first_default_manual_command_after_ackermann_is_sent) {
    vehicle->ApplyAckermannControl(AckermannControl());
    vehicle->ApplyControl(carla::rpc::VehicleControl{});
    Barrier();
    EXPECT_EQ(server.Manual().size(), 1u);
  }

  TEST_F(VehicleControlCache, ordinary_repeated_manual_commands_remain_sticky) {
    vehicle->ApplyControl(ManualControl());
    vehicle->ApplyControl(ManualControl());
    Barrier();
    EXPECT_EQ(server.Manual().size(), 1u);
  }

  TEST_F(VehicleControlCache, nonsticky_manual_commands_are_all_sent) {
    auto nonsticky = server.MakeVehicle(simulator, false);
    nonsticky->ApplyControl(ManualControl());
    nonsticky->ApplyControl(ManualControl());
    nonsticky->ApplyAckermannControl(AckermannControl());
    nonsticky->ApplyControl(ManualControl());
    Barrier();
    EXPECT_EQ(server.Manual().size(), 3u);
  }

  TEST_F(VehicleControlCache, distinct_manual_commands_are_sent) {
    vehicle->ApplyControl(ManualControl(0.25f));
    vehicle->ApplyControl(ManualControl(0.75f));
    Barrier();
    EXPECT_EQ(server.Manual().size(), 2u);
  }

  TEST_F(VehicleControlCache, repeated_mode_transitions_keep_reengaging_manual_control) {
    for (size_t i = 0u; i < 3u; ++i) {
      vehicle->ApplyControl(ManualControl());
      vehicle->ApplyAckermannControl(AckermannControl());
    }
    vehicle->ApplyControl(ManualControl());
    Barrier();
    EXPECT_EQ(server.Manual().size(), 4u);
    EXPECT_EQ(server.Ackermann().size(), 3u);
  }

} // namespace
