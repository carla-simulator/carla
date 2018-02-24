// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include "carla/server/AsyncServer.h"
#include "carla/server/CarlaEncoder.h"
#include "carla/server/CarlaSceneDescription.h"
#include "carla/server/EncoderServer.h"
#include "carla/server/RequestNewEpisode.h"
#include "carla/server/TCPServer.h"

namespace carla {
namespace server {

  class AgentServer;

  class WorldServer : private NonCopyable {
  public:

    WorldServer();

    ~WorldServer();

    void Disconnect() {
      KillAgentServer();
      _world_server.Disconnect();
    }

    std::future<error_code> Connect(uint32_t port, time_duration timeout);

    error_code TryRead(carla_request_new_episode &request_new_episode, timeout_t timeout);

    std::future<error_code> Write(const carla_scene_description &scene_description);

    error_code TryRead(carla_episode_start &episode_start, timeout_t timeout);

    std::future<error_code> Write(const carla_episode_ready &episode_ready);

    /// This assumes you have entered the loop of write measurements, read
    /// control.
    void StartAgentServer();

    AgentServer *GetAgentServer() {
      return _agent_server.get();
    }

    void KillAgentServer();

    void ResetProtocol();

  private:

    struct Protocol {
      Protocol() = default;
      explicit Protocol(time_duration timeout);

      ReadTask<RequestNewEpisode> request_new_episode;
      WriteTask<CarlaSceneDescription> scene_description;
      ReadTask<carla_episode_start> episode_start;
      WriteTask<carla_episode_ready> episode_ready;
    };

    void ExecuteProtocol(Protocol &&protocol);

    uint32_t _port;

    time_duration _timeout;

    CarlaEncoder _encoder;

    Protocol _protocol;

    AsyncServer<EncoderServer<TCPServer>> _world_server;

    std::vector<carla_sensor_definition> _sensor_definitions;

    std::unique_ptr<AgentServer> _agent_server;

    RequestNewEpisode _new_episode_data;
  };

} // namespace server
} // namespace carla
