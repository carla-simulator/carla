// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// #include "carla/Logging.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

#include <mutex>
#include <optional>

namespace carla {
namespace multigpu {

// using session = std::shared_ptr<Primary>;
// using callback_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;
using token_type = carla::streaming::detail::token_type;
using stream_id = carla::streaming::detail::stream_id_type;

class Router;

class PrimaryCommands {
  public:


    PrimaryCommands();
    PrimaryCommands(std::shared_ptr<Router> router);

    void set_router(std::shared_ptr<Router> router);

    // broadcast to all secondary servers the frame data
    void SendFrameData(carla::Buffer buffer);

    // broadcast to all secondary servers the map to load
    void SendLoadMap(std::string map);

    // send to know if a connection is alive
    void SendIsAlive();

    /// Returns std::nullopt if no secondary could provide the token (e.g. the
    /// selected secondary disconnected with the request in flight). Nothing
    /// is cached on failure, so a later call for the same sensor retries.
    [[nodiscard]]
    std::optional<token_type> GetToken(stream_id sensor_id);

    /// Returns false if no secondary ever accepted this sensor (GetToken()
    /// routing failed) or if the secondary holding it gave no reply (the
    /// null-session path from SendEnableForROS). The caller must treat that
    /// as ROS enablement not having happened.
    [[nodiscard]]
    bool EnableForROS(stream_id sensor_id);

    void DisableForROS(stream_id sensor_id);

    bool IsEnabledForROS(stream_id sensor_id);

  private:

    // send to one secondary to get the token of a sensor; also reports which
    // secondary session actually answered, so the caller never has to guess
    // it from a separate (and racy) round-robin lookup.
    std::optional<token_type> SendGetToken(
        carla::streaming::detail::stream_id_type sensor_id,
        std::weak_ptr<Primary> &out_session);

    // manage ROS enable/disable of sensor
    bool SendEnableForROS(stream_id sensor_id);
    void SendDisableForROS(stream_id sensor_id);
    bool SendIsEnabledForROS(stream_id sensor_id);


    std::shared_ptr<Router> _router;
    std::unordered_map<stream_id, token_type> _tokens;
    std::unordered_map<stream_id, std::weak_ptr<Primary>> _servers;

    // Serializes every round trip through _router (GetToken/EnableForROS/...).
    // The router keeps at most one in-flight promise per secondary session
    // (see Router::_promises), so two overlapping requests to the same
    // session would otherwise clobber each other's promise.
    std::mutex _mutex;
};

} // namespace multigpu
} // namespace carla
