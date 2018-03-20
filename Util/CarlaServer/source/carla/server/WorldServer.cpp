// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/server/WorldServer.h"

#include "carla/Debug.h"
#include "carla/server/AgentServer.h"

namespace carla {
namespace server {

  // ===========================================================================
  // -- Static local functions -------------------------------------------------
  // ===========================================================================

  static bool IsPortValid(const uint32_t port) {
    constexpr uint32_t MIN = 1023u;
    return (port > MIN) && (port + 1u > MIN) && (port + 2u > MIN);
  }

  static std::future<error_code> GetInvalidPortResult(const uint32_t port) {
    log_error("invalid port", port);
    std::promise<error_code> promise;
    promise.set_value(errc::invalid_argument());
    return promise.get_future();
  }

  template <typename T>
  static error_code TryRead(ReadTask<T> &task, T &value, timeout_t timeout) {
    DEBUG_ASSERT(task.valid());
    Reading<T> reading;
    reading.error_code = errc::try_again();
    if (task.TryGetResult(reading, timeout) && !reading.error_code) {
      value = std::move(reading.message);
    }
    return reading.error_code;
  }

  template <typename T, typename E = typename std::remove_reference<T>::type>
  static std::future<error_code> Write(WriteTask<E> &task, T &&value) {
    DEBUG_ASSERT(task.valid());
    task.set_message(std::forward<T>(value));
    return task.ReleaseResult();
  }

  // ===========================================================================
  // --  -------------------------------------------------
  // ===========================================================================

  WorldServer::Protocol::Protocol(const time_duration timeout)
      : request_new_episode(timeout),
        scene_description(timeout),
        episode_start(timeout),
        episode_ready(timeout) {}

  // ===========================================================================
  // --  -------------------------------------------------
  // ===========================================================================

  WorldServer::WorldServer()
      : _encoder(),
        _world_server(_encoder) {}

  WorldServer::~WorldServer() {}

  std::future<error_code> WorldServer::Connect(
      const uint32_t port,
      const time_duration timeout) {
    if (!IsPortValid(port)) {
      return GetInvalidPortResult(port);
    }
    _port = port;
    _timeout = timeout;
    auto result = _world_server.Connect(_port, _timeout);
    ExecuteProtocol(Protocol(_timeout));
    return result;
  }

  error_code WorldServer::TryRead(
      carla_request_new_episode &request_new_episode,
      const timeout_t timeout) {
    auto ec = carla::server::TryRead(_protocol.request_new_episode, _new_episode_data, timeout);
    if (!ec) {
      request_new_episode = _new_episode_data.values;
    }
    return ec;
  }

  std::future<error_code> WorldServer::Write(
      const carla_scene_description &scene_description) {
    /// @todo Here sensor names are not copied and will be invalidated.
    decltype(_sensor_definitions) defs(
        scene_description.sensors,
        scene_description.sensors + scene_description.number_of_sensors);
    _sensor_definitions = std::move(defs);
    CarlaSceneDescription scene(_encoder.Encode(scene_description));
    return carla::server::Write(_protocol.scene_description, std::move(scene));
  }

  error_code WorldServer::TryRead(
      carla_episode_start &episode_start,
      const timeout_t timeout) {
    return carla::server::TryRead(_protocol.episode_start, episode_start, timeout);
  }

  std::future<error_code> WorldServer::Write(
      const carla_episode_ready &episode_ready) {
    return carla::server::Write(_protocol.episode_ready, episode_ready);
  }

  void WorldServer::StartAgentServer() {
    _agent_server = std::make_unique<AgentServer>(
        _encoder,
        _port + 1u,
        _port + 2u,
        _sensor_definitions,
        _timeout);
  }

  void WorldServer::KillAgentServer() {
    _agent_server = nullptr;
    _sensor_definitions.clear();
  }

  void WorldServer::ResetProtocol() {
    Protocol protocol(_timeout);
    // Here we need to wait forever for the new episode, as it will take as long
    // as the current episode lasts.
    protocol.request_new_episode.set_timeout(boost::posix_time::pos_infin);
    ExecuteProtocol(std::move(protocol));
  }

  void WorldServer::ExecuteProtocol(Protocol &&protocol) {
    _protocol = std::move(protocol);
    _world_server.Execute(_protocol.request_new_episode);
    _world_server.Execute(_protocol.scene_description);
    _world_server.Execute(_protocol.episode_start);
    _world_server.Execute(_protocol.episode_ready);
  }

} // namespace server
} // namespace carla
