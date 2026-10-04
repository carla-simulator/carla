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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace carla {
namespace multigpu {

// using session = std::shared_ptr<Primary>;
// using callback_response = std::function<void(std::shared_ptr<Primary>, carla::Buffer)>;
using token_type = carla::streaming::detail::token_type;
using stream_id = carla::streaming::detail::stream_id_type;
using actor_id = uint32_t;

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

    void SendPublishTF(bool publish_tf);

    // send to know if a connection is alive
    void SendIsAlive();

    /// Returns std::nullopt if no secondary could provide the token (e.g. the
    /// selected secondary disconnected with the request in flight, or has no
    /// episode loaded). Nothing is cached on failure, so a later call for the
    /// same sensor retries. A sensor whose secondary disconnected is routed
    /// again here. @a sensor_actor_id is the primary's id of the sensor actor,
    /// which the secondary resolves to its own sensor stream.
    [[nodiscard]]
    std::optional<token_type> GetToken(stream_id sensor_id, actor_id sensor_actor_id);

    /// Returns false if no secondary accepted this sensor or the secondary
    /// holding it did not reply; ROS enablement has not happened in that case.
    [[nodiscard]]
    bool EnableForROS(stream_id sensor_id, actor_id sensor_actor_id);

    void DisableForROS(stream_id sensor_id);

    bool IsEnabledForROS(stream_id sensor_id);

    /// Routes every sensor whose secondary disconnected to the secondaries
    /// connected since the previous call, under the same primary stream id,
    /// so clients already subscribed reconnect to it without a new listen().
    /// The first new secondary that stays connected takes every lost sensor.
    /// A sensor the new secondary refuses is forgotten and routed lazily
    /// later; if it disconnects instead, the sensor stays lost for the next
    /// one. ROS publishing enabled through EnableForROS() is enabled again.
    /// Blocks on round trips; never call it from a Router callback. Returns
    /// how many sensors were routed again.
    std::size_t RerouteLostSensors();

    /// Drops the routing of a destroyed sensor so it is never routed again.
    void ForgetSensor(stream_id sensor_id);

    /// Whether @a sensor_id is routed to a secondary that is still connected.
    /// Never blocks on a round trip; callable from any thread.
    [[nodiscard]]
    bool IsRouted(stream_id sensor_id) const;

  private:

    struct Route {
      std::weak_ptr<Primary> session;
      actor_id actor;
      bool ros_enabled = false;
    };

    enum class TokenError { None, NoReply, Refused };

    // send to one secondary (@a target, or round-robin if null) to get the
    // token of a sensor; also reports which secondary session actually
    // answered, so the caller never has to guess it from a separate (and
    // racy) round-robin lookup. On failure @a error says whether a secondary
    // answered (Refused) or not (NoReply).
    std::optional<token_type> SendGetToken(
        stream_id sensor_id,
        actor_id sensor_actor_id,
        const std::weak_ptr<Primary> *target,
        std::weak_ptr<Primary> &out_session,
        TokenError &error);

    // manage ROS enable/disable of sensor
    bool SendEnableForROS(stream_id sensor_id);
    void SendDisableForROS(stream_id sensor_id);
    bool SendIsEnabledForROS(stream_id sensor_id);

    /// Whether @a sensor_id is routed to a secondary that is still connected.
    /// Caller must hold _mutex.
    bool IsRoutedToLiveSecondary(stream_id sensor_id);

    /// Enables ROS again on the new secondary of a re-routed sensor if it was
    /// enabled before. Caller must hold _mutex.
    void RestoreRosAfterReroute(stream_id sensor_id);

    void SetRoute(stream_id sensor_id, std::weak_ptr<Primary> server);
    void EraseRoute(stream_id sensor_id);
    void ClearRoutes();

    /// Reads a one-bool reply; @a buffer must not be empty.
    static bool ReadBoolReply(const carla::Buffer &buffer);

    std::shared_ptr<Router> _router;
    std::unordered_map<stream_id, token_type> _tokens;
    // Keeps the routes of disconnected secondaries until they are routed
    // again, forgotten, or a map load clears them.
    std::unordered_map<stream_id, Route> _servers;

    // Serializes every round trip through _router (GetToken/EnableForROS/...).
    // The router keeps at most one in-flight promise per secondary session
    // (see Router::_promises), so two overlapping requests to the same
    // session would otherwise clobber each other's promise.
    std::mutex _mutex;

    // Mirror of _servers for IsRouted(), which must not wait on _mutex.
    mutable std::mutex _routes_mutex;
    std::unordered_map<stream_id, std::weak_ptr<Primary>> _routes;
};

} // namespace multigpu
} // namespace carla
