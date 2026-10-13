// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/primaryCommands.h"

// #include "carla/Logging.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/multigpu/router.h"
#include "carla/streaming/detail/tcp/Message.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

#include <algorithm>
#include <cstring>
#include <string_view>
#include <vector>

namespace carla {
namespace multigpu {

PrimaryCommands::PrimaryCommands() {
}

PrimaryCommands::PrimaryCommands(std::shared_ptr<Router> router) :
  _router(router) {
}

void PrimaryCommands::set_router(std::shared_ptr<Router> router) {
  _router = router;
}

// broadcast to all secondary servers the frame data
void PrimaryCommands::SendFrameData(carla::Buffer buffer) {
  _router->Write(MultiGPUCommand::SEND_FRAME, std::move(buffer));
  // log_info("sending frame command");
}

// broadcast to all secondary servers the map to load
void PrimaryCommands::SendLoadMap(std::string map) {
  // Held across the broadcast so no GetToken() can cache a token of the old
  // episode between the purge and the secondaries receiving LOAD_MAP.
  std::scoped_lock<std::mutex> lock(_mutex);
  _tokens.clear();
  _servers.clear();
  ClearRoutes();
  _router->WriteLoadMap(map);
}

// send to who the router wants the request for a token
std::optional<token_type> PrimaryCommands::SendGetToken(
    stream_id sensor_id,
    actor_id sensor_actor_id,
    const std::weak_ptr<Primary> *target,
    std::weak_ptr<Primary> &out_session,
    TokenError &error) {
  log_info("asking for a token");
  error = TokenError::None;
  const GetTokenRequest request{sensor_id, sensor_actor_id};
  carla::Buffer buf(reinterpret_cast<const carla::Buffer::value_type *>(&request), sizeof(request));
  auto fut = (target != nullptr) ?
      _router->WriteToOne(*target, MultiGPUCommand::GET_TOKEN, std::move(buf)) :
      _router->WriteToNext(MultiGPUCommand::GET_TOKEN, std::move(buf));

  auto response = fut.get();
  if (response.session == nullptr) {
    log_error("multigpu: no secondary provided a token for sensor ", sensor_id);
    error = TokenError::NoReply;
    return std::nullopt;
  }
  const std::string_view payload(reinterpret_cast<const char *>(response.buffer.data()), response.buffer.size());
  if (payload == kTokenNotReadyMarker) {
    log_error("multigpu: secondary cannot serve sensor ", sensor_id, " (actor ", sensor_actor_id, ")");
    error = TokenError::Refused;
    return std::nullopt;
  }
  if (response.buffer.size() < sizeof(carla::streaming::detail::token_data)) {
    log_error("multigpu: malformed token reply for sensor ", sensor_id, ": ", response.buffer.size(), " bytes");
    error = TokenError::Refused;
    return std::nullopt;
  }
  out_session = response.session;
  carla::streaming::detail::token_data data;
  std::memcpy(&data, response.buffer.data(), sizeof(data));
  token_type new_token(data);
  // Clients key and unsubscribe streams by the primary's stream id only.
  if (new_token.get_stream_id() != sensor_id) {
    log_warning("multigpu: secondary answered sensor ", sensor_id, " with stream id ", new_token.get_stream_id());
    new_token.set_stream_id(sensor_id);
  }
  log_info("got a token: ", new_token.get_stream_id(), ", ", new_token.get_port());
  return new_token;
}

void PrimaryCommands::SendPublishTF(bool publish_tf) {
  const uint8_t value = publish_tf ? 1u : 0u;
  _router->Write(MultiGPUCommand::SET_PUBLISH_TF, carla::Buffer(&value, sizeof(value)));
}

// send to know if a connection is alive
void PrimaryCommands::SendIsAlive() {
  std::scoped_lock<std::mutex> lock(_mutex);
  std::string msg("Are you alive?");
  carla::Buffer buf(reinterpret_cast<const unsigned char *>(msg.c_str()), static_cast<size_t>(msg.size()));
  log_info("sending is alive command");
  auto fut = _router->WriteToNext(MultiGPUCommand::YOU_ALIVE, std::move(buf));
  auto response = fut.get();
  if (response.session == nullptr) {
    log_error("multigpu: is-alive command got no reply");
    return;
  }
  log_info("response from alive command: ", response.buffer.data());
}

bool PrimaryCommands::SendEnableForROS(stream_id sensor_id) {
  // search if the sensor has been activated in any secondary server
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    carla::Buffer buf(reinterpret_cast<carla::Buffer::value_type *>(&sensor_id),
                      static_cast<size_t>(sizeof(stream_id)));
    auto fut = _router->WriteToOne(it->second.session, MultiGPUCommand::ENABLE_ROS, std::move(buf));

    auto response = fut.get();
    if ((response.session == nullptr) || response.buffer.empty()) {
      log_error("enable_for_ros for sensor ", sensor_id, " got no reply from its secondary server");
      return false;
    }
    if (!ReadBoolReply(response.buffer)) {
      log_error("enable_for_ros for sensor ", sensor_id, " was refused by its secondary server");
      return false;
    }
    return true;
  } else {
    log_error("enable_for_ros for sensor", sensor_id, " not found on any server");
    return false;
  }
}

void PrimaryCommands::SendDisableForROS(stream_id sensor_id) {
  // search if the sensor has been activated in any secondary server
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    carla::Buffer buf(reinterpret_cast<carla::Buffer::value_type *>(&sensor_id),
                      static_cast<size_t>(sizeof(stream_id)));
    auto fut = _router->WriteToOne(it->second.session, MultiGPUCommand::DISABLE_ROS, std::move(buf));

    auto response = fut.get();
    if ((response.session == nullptr) || response.buffer.empty()) {
      log_error("disable_for_ros for sensor ", sensor_id, " got no reply from its secondary server");
    } else if (!ReadBoolReply(response.buffer)) {
      log_error("disable_for_ros for sensor ", sensor_id, " was refused by its secondary server");
    }
  } else {
    log_error("disable_for_ros for sensor", sensor_id, " not found on any server");
  }
}

bool PrimaryCommands::SendIsEnabledForROS(stream_id sensor_id) {
  // search if the sensor has been activated in any secondary server
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    carla::Buffer buf(reinterpret_cast<carla::Buffer::value_type *>(&sensor_id),
                      static_cast<size_t>(sizeof(stream_id)));
    auto fut = _router->WriteToOne(it->second.session, MultiGPUCommand::IS_ENABLED_ROS, std::move(buf));

    auto response = fut.get();
    if ((response.session == nullptr) || response.buffer.empty()) {
      log_error("is_enabled_for_ros for sensor ", sensor_id, " got no reply from its secondary server");
      return false;
    }
    return ReadBoolReply(response.buffer);
  } else {
    log_error("is_enabled_for_ros for sensor", sensor_id, " not found on any server");
    return false;
  }
}

std::optional<token_type> PrimaryCommands::GetToken(stream_id sensor_id, actor_id sensor_actor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  if (IsRoutedToLiveSecondary(sensor_id)) {
    auto it = _tokens.find(sensor_id);
    if (it != _tokens.end()) {
      log_debug("Using token from already activated sensor: ", it->second.get_stream_id(), ", ", it->second.get_port());
      return it->second;
    }
  }
  const bool lost = _servers.contains(sensor_id);
  if (lost) {
    log_warning("multigpu: secondary of sensor ", sensor_id, " is gone, routing it again");
  }
  // enable the sensor on one secondary server; the session that actually
  // answers comes back from the round trip itself, not from a separate
  // (and potentially stale) "next server" lookup.
  std::weak_ptr<Primary> server;
  TokenError error = TokenError::None;
  auto token = SendGetToken(sensor_id, sensor_actor_id, nullptr, server, error);
  if (!token) {
    if (error == TokenError::Refused) {
      _servers.erase(sensor_id);
      _tokens.erase(sensor_id);
      EraseRoute(sensor_id);
    }
    return std::nullopt;
  }
  _tokens.erase(sensor_id);
  _tokens.emplace(sensor_id, *token);
  auto route = _servers.try_emplace(sensor_id, Route{server, sensor_actor_id}).first;
  route->second.session = server;
  route->second.actor = sensor_actor_id;
  SetRoute(sensor_id, server);
  if (lost) {
    RestoreRosAfterReroute(sensor_id);
  }
  log_debug("Using token from new activated sensor: ", token->get_stream_id(), ", ", token->get_port());
  return token;
}

bool PrimaryCommands::EnableForROS(stream_id sensor_id, actor_id sensor_actor_id) {
  {
    std::scoped_lock<std::mutex> lock(_mutex);
    if (IsRoutedToLiveSecondary(sensor_id)) {
      const bool enabled = SendEnableForROS(sensor_id);
      if (enabled) {
        _servers[sensor_id].ros_enabled = true;
      }
      return enabled;
    }
  }
  // The sensor has not been routed to a live secondary yet. GetToken()
  // performs that routing under its own critical section, so the lock above
  // must be released first to avoid a self-deadlock on the recursive call
  // below. On failure nothing was routed, so recursing would retry forever.
  if (!GetToken(sensor_id, sensor_actor_id)) {
    log_error("enable_for_ros for sensor ", sensor_id, " failed: no secondary server accepted it");
    return false;
  }
  return EnableForROS(sensor_id, sensor_actor_id);
}

void PrimaryCommands::DisableForROS(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  auto it = _servers.find(sensor_id);
  if (it == _servers.end()) {
    return;
  }
  it->second.ros_enabled = false;
  if (_router->IsConnected(it->second.session)) {
    SendDisableForROS(sensor_id);
  }
}

bool PrimaryCommands::IsEnabledForROS(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  if (IsRoutedToLiveSecondary(sensor_id)) {
    return SendIsEnabledForROS(sensor_id);
  }
  return false;
}

std::size_t PrimaryCommands::RerouteLostSensors() {
  std::scoped_lock<std::mutex> lock(_mutex);
  std::size_t rerouted = 0u;
  for (const auto &session : _router->TakeNewSessions()) {
    std::vector<stream_id> lost;
    for (const auto &[sensor_id, route] : _servers) {
      if (!_router->IsConnected(route.session)) {
        lost.push_back(sensor_id);
      }
    }
    std::sort(lost.begin(), lost.end());
    for (const stream_id sensor_id : lost) {
      if (!_router->IsConnected(session)) {
        break;
      }
      auto route = _servers.find(sensor_id);
      std::weak_ptr<Primary> server;
      TokenError error = TokenError::None;
      auto token = SendGetToken(sensor_id, route->second.actor, &session, server, error);
      if (!token) {
        if ((error == TokenError::NoReply) || !_router->IsConnected(session)) {
          log_warning("multigpu: the new secondary left while routing sensor ", sensor_id, "; it stays lost");
          break;
        }
        log_warning("multigpu: could not route sensor ", sensor_id, " to the new secondary; its listeners must listen() again");
        _servers.erase(route);
        _tokens.erase(sensor_id);
        EraseRoute(sensor_id);
        continue;
      }
      auto old_token = _tokens.find(sensor_id);
      if ((old_token != _tokens.end()) && (old_token->second.get_port() != token->get_port())) {
        log_warning("multigpu: sensor ", sensor_id, " moved from port ", old_token->second.get_port(),
            " to port ", token->get_port(), "; existing listeners cannot follow it");
      }
      _tokens.erase(sensor_id);
      _tokens.emplace(sensor_id, *token);
      route->second.session = server;
      SetRoute(sensor_id, server);
      log_info("multigpu: routed sensor ", sensor_id, " to the new secondary at port ", token->get_port());
      ++rerouted;
      RestoreRosAfterReroute(sensor_id);
    }
  }
  return rerouted;
}

void PrimaryCommands::ForgetSensor(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  _servers.erase(sensor_id);
  _tokens.erase(sensor_id);
  EraseRoute(sensor_id);
}

bool PrimaryCommands::IsRoutedToLiveSecondary(stream_id sensor_id) {
  auto it = _servers.find(sensor_id);
  return (it != _servers.end()) && _router->IsConnected(it->second.session);
}

void PrimaryCommands::RestoreRosAfterReroute(stream_id sensor_id) {
  auto it = _servers.find(sensor_id);
  if ((it == _servers.end()) || !it->second.ros_enabled) {
    return;
  }
  if (!SendEnableForROS(sensor_id)) {
    log_warning("multigpu: could not enable ROS again for sensor ", sensor_id, " on its new secondary");
  }
}

bool PrimaryCommands::IsRouted(stream_id sensor_id) const {
  std::weak_ptr<Primary> server;
  {
    std::scoped_lock<std::mutex> lock(_routes_mutex);
    auto it = _routes.find(sensor_id);
    if (it == _routes.end()) {
      return false;
    }
    server = it->second;
  }
  return (_router != nullptr) && _router->IsConnected(server);
}

void PrimaryCommands::SetRoute(stream_id sensor_id, std::weak_ptr<Primary> server) {
  std::scoped_lock<std::mutex> lock(_routes_mutex);
  _routes[sensor_id] = std::move(server);
}

void PrimaryCommands::EraseRoute(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_routes_mutex);
  _routes.erase(sensor_id);
}

void PrimaryCommands::ClearRoutes() {
  std::scoped_lock<std::mutex> lock(_routes_mutex);
  _routes.clear();
}

bool PrimaryCommands::ReadBoolReply(const carla::Buffer &buffer) {
  bool value = false;
  std::memcpy(&value, buffer.data(), sizeof(value));
  return value;
}

} // namespace multigpu
} // namespace carla
