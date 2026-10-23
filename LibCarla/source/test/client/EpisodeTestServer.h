// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <carla/BufferView.h>
#include <carla/client/detail/Client.h>
#include <carla/client/detail/Episode.h>
#include <carla/rpc/Metadata.h>
#include <carla/rpc/Response.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/s11n/EpisodeStateSerializer.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>
#include <carla/streaming/Server.h>

#include <rpc/server.h>

#include <atomic>
#include <chrono>
#include <exception>
#include <future>
#include <mutex>
#include <optional>
#include <thread>

namespace test {

  template <typename Predicate>
  bool WaitForCondition(Predicate predicate,
      std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (!predicate() && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return predicate();
  }

  // Supplies real RPC and streaming transports to the production client.
  class EpisodeTestServer {
  public:
    EpisodeTestServer()
      : _streaming("127.0.0.1", 0u), _stream(_streaming.MakeStream()),
        _rpc("127.0.0.1", 0u) {
      _streaming.SetSynchronousMode(true);
      _rpc.bind("get_episode_info", [this](carla::rpc::Metadata) {
        carla::rpc::EpisodeInfo info{};
        info.id = _episode_id.load();
        info.token = _stream.token();
        return carla::rpc::Response<carla::rpc::EpisodeInfo>{info};
      });
      _rpc.bind("get_sensor_token", [this](carla::rpc::Metadata, uint32_t) {
        return carla::rpc::Response<carla::streaming::Token>{_stream.token()};
      });
    }

    ~EpisodeTestServer() {
      _rpc.stop();
    }

    auto RpcPort() const { return _rpc.port(); }

    void Start() {
      _streaming.AsyncRun(1u);
      _rpc.async_run(1u);
    }

    uint64_t Publish(bool map_changed = false,
        std::optional<uint64_t> episode_id = std::nullopt) {
      std::scoped_lock<std::mutex> lock(_publish_mutex);
      if (episode_id.has_value()) {
        _episode_id = *episode_id;
      }
      const auto frame = ++_frame;
      using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
      Serializer::Header state{};
      state.episode_id = _episode_id.load();
      state.delta_seconds = 0.02f;
      state.simulation_state = map_changed ? Serializer::MapChange : Serializer::None;
      auto payload = _stream.MakeBuffer();
      payload.copy_from(boost::asio::buffer(&state, sizeof(state)));
      constexpr auto index = carla::sensor::SensorRegistry::get<FWorldObserver *>::index;
      auto header = carla::sensor::s11n::SensorHeaderSerializer::Serialize(
          index, frame, static_cast<double>(frame) * 0.02, {});
      _stream.Write(carla::BufferView::CreateFrom(std::move(header)),
          carla::BufferView::CreateFrom(std::move(payload)));
      return frame;
    }

    auto Frame() {
      std::scoped_lock<std::mutex> lock(_publish_mutex);
      return _frame;
    }

    template <typename Predicate>
    bool PublishUntil(Predicate predicate, bool map_changed = false) {
      return WaitForCondition([&]() {
        if (predicate()) {
          return true;
        }
        Publish(map_changed);
        return false;
      });
    }

  private:
    carla::streaming::Server _streaming;
    carla::streaming::Stream _stream;
    std::atomic_uint64_t _episode_id{1u};
    std::mutex _publish_mutex;
    uint64_t _frame = 0u;
    ::rpc::server _rpc;
  };

  class ClientEpisode {
  public:
    ClientEpisode() {
      server.Start();
      client = std::make_unique<carla::client::detail::Client>(
          "127.0.0.1", server.RpcPort(), 1u);
      auto destroyed = std::make_shared<std::promise<void>>();
      _destroyed = destroyed->get_future();
      episode = std::shared_ptr<carla::client::detail::Episode>(
          new carla::client::detail::Episode(*client, {}),
          [destroyed](auto *value) {
            delete value;
            destroyed->set_value();
          });
      episode->Listen();
      episode->RegisterOnTickEvent([this](carla::client::WorldSnapshot state) {
        _received_frame = state.GetFrame();
      });
    }

    ~ClientEpisode() {
      episode.reset();
      // The last callback may own the episode until it finishes. Keep its
      // borrowed client alive through the episode destructor's unsubscribe.
      if (_destroyed.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
        std::terminate();
      }
    }

    bool Bootstrap() {
      if (!server.PublishUntil([&]() { return _received_frame.load() > 0u; })) {
        return false;
      }
      const auto sent = server.Frame();
      return WaitForFrame(sent);
    }

    bool WaitForFrame(uint64_t frame) {
      return WaitForCondition([&]() { return _received_frame.load() >= frame; });
    }

    EpisodeTestServer server;
    std::unique_ptr<carla::client::detail::Client> client;
    std::shared_ptr<carla::client::detail::Episode> episode;

  private:
    std::future<void> _destroyed;
    std::atomic_uint64_t _received_frame{0u};
  };

} // namespace test
