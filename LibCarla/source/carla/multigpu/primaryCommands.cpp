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
  carla::Buffer buf(reinterpret_cast<const unsigned char *>(map.c_str()), static_cast<size_t>(map.size() + 1));
  _router->Write(MultiGPUCommand::LOAD_MAP, std::move(buf));
}

// send to who the router wants the request for a token
std::optional<token_type> PrimaryCommands::SendGetToken(stream_id sensor_id, std::weak_ptr<Primary> &out_session) {
  log_info("asking for a token");
  carla::Buffer buf(reinterpret_cast<carla::Buffer::value_type *>(&sensor_id),
                    static_cast<size_t>(sizeof(stream_id)));
  auto fut = _router->WriteToNext(MultiGPUCommand::GET_TOKEN, std::move(buf));

  auto response = fut.get();
  if (response.session == nullptr) {
    log_error("multigpu: no secondary provided a token for sensor ", sensor_id);
    return std::nullopt;
  }
  if (response.buffer.size() < sizeof(carla::streaming::detail::token_data)) {
    log_error("multigpu: malformed token reply for sensor ", sensor_id, ": ", response.buffer.size(), " bytes");
    return std::nullopt;
  }
  out_session = response.session;
  token_type new_token(*reinterpret_cast<carla::streaming::detail::token_data *>(response.buffer.data()));
  log_info("got a token: ", new_token.get_stream_id(), ", ", new_token.get_port());
  return new_token;
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
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::ENABLE_ROS, std::move(buf));

    if (fut.get().session == nullptr) {
      log_error("enable_for_ros for sensor ", sensor_id, " got no reply from its secondary server");
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
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::DISABLE_ROS, std::move(buf));

    if (fut.get().session == nullptr) {
      log_error("disable_for_ros for sensor ", sensor_id, " got no reply from its secondary server");
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
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::IS_ENABLED_ROS, std::move(buf));

    auto response = fut.get();
    if ((response.session == nullptr) || response.buffer.empty()) {
      log_error("is_enabled_for_ros for sensor ", sensor_id, " got no reply from its secondary server");
      return false;
    }
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
    return res;
  } else {
    log_error("is_enabled_for_ros for sensor", sensor_id, " not found on any server");
    return false;
  }
}

std::optional<token_type> PrimaryCommands::GetToken(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  // search if the sensor has been activated in any secondary server
  auto it = _tokens.find(sensor_id);
  if (it != _tokens.end()) {
    // return already activated sensor token
    log_debug("Using token from already activated sensor: ", it->second.get_stream_id(), ", ", it->second.get_port());
    return it->second;
  }
  else {
    // enable the sensor on one secondary server; the session that actually
    // answers comes back from the round trip itself, not from a separate
    // (and potentially stale) "next server" lookup.
    std::weak_ptr<Primary> server;
    auto token = SendGetToken(sensor_id, server);
    if (!token) {
      return std::nullopt;
    }
    // add to the maps
    _tokens.emplace(sensor_id, *token);
    _servers[sensor_id] = server;
    log_debug("Using token from new activated sensor: ", token->get_stream_id(), ", ", token->get_port());
    return token;
  }
}

bool PrimaryCommands::EnableForROS(stream_id sensor_id) {
  {
    std::scoped_lock<std::mutex> lock(_mutex);
    auto it = _servers.find(sensor_id);
    if (it != _servers.end()) {
      return SendEnableForROS(sensor_id);
    }
  }
  // The sensor has not been routed to a secondary yet. GetToken() performs
  // that routing under its own critical section, so the lock above must be
  // released first to avoid a self-deadlock on the recursive call below.
  // On failure nothing was routed, so recursing would retry forever.
  if (!GetToken(sensor_id)) {
    log_error("enable_for_ros for sensor ", sensor_id, " failed: no secondary server accepted it");
    return false;
  }
  return EnableForROS(sensor_id);
}

void PrimaryCommands::DisableForROS(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    SendDisableForROS(sensor_id);
  }
}

bool PrimaryCommands::IsEnabledForROS(stream_id sensor_id) {
  std::scoped_lock<std::mutex> lock(_mutex);
  auto it = _servers.find(sensor_id);
  if (it != _servers.end()) {
    return SendIsEnabledForROS(sensor_id);
  }
  return false;
}

} // namespace multigpu
} // namespace carla
