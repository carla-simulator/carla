// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/primaryCommands.h"

// #include "carla/Logging.h"
#include "carla/multigpu/commands.h"
#include "carla/multigpu/primary.h"
#include "carla/multigpu/router.h"
#include "carla/streaming/detail/Message.h"
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
  carla::Buffer buf((unsigned char *) map.c_str(), (size_t) map.size() + 1);
  _router->Write(MultiGPUCommand::LOAD_MAP, std::move(buf));
}

// send to who the router wants the request for a token
token_type PrimaryCommands::SendGetToken(stream_id stream_id) {
  log_info("asking for a token");
  carla::Buffer buf((carla::Buffer::value_type *) &stream_id,
                    (size_t) sizeof(stream_id));
  auto fut = _router->WriteToNext(MultiGPUCommand::GET_TOKEN, std::move(buf));

  auto response = fut.get();
  token_type new_token(*reinterpret_cast<carla::streaming::detail::token_data *>(response.buffer.data()));
  log_info("got a token: ", new_token.get_stream_id(), ", ", new_token.get_port());
  return new_token;
}

// send to know if a connection is alive
void PrimaryCommands::SendIsAlive() {
  std::string msg("Are you alive?");
  carla::Buffer buf((unsigned char *) msg.c_str(), (size_t) msg.size());
  log_info("sending is alive command");
  auto fut = _router->WriteToNext(MultiGPUCommand::YOU_ALIVE, std::move(buf));
  auto response = fut.get();
  log_info("response from alive command: ", response.buffer.data());
}

void PrimaryCommands::SendEnableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  // search if the actor has been activated in any secondary server
  auto it = _servers.find(stream_actor_id.stream_id);
  if (it != _servers.end()) {
    carla::Buffer buf((carla::Buffer::value_type *) &stream_actor_id,
                      (size_t) (sizeof(stream_actor_id)));
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::ENABLE_ROS, std::move(buf));

    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
  } else {
    log_error("enable_for_ros for actor ", stream_actor_id.stream_id, " not found on any server");
  }
}

void PrimaryCommands::SendDisableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  // search if the actor has been activated in any secondary server
  auto it = _servers.find(stream_actor_id.stream_id);
  if (it != _servers.end()) {
    carla::Buffer buf((carla::Buffer::value_type *) &stream_actor_id,
                      (size_t) (sizeof(stream_actor_id)));
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::DISABLE_ROS, std::move(buf));

    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
  } else {
    log_error("disable_for_ros for actor ", stream_actor_id.stream_id, " not found on any server");
  }
}

bool PrimaryCommands::SendIsEnabledForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  // search if the actor has been activated in any secondary server
  auto it = _servers.find(stream_actor_id.stream_id);
  if (it != _servers.end()) {
    carla::Buffer buf((carla::Buffer::value_type *) &stream_actor_id,
                      (size_t) (sizeof(stream_actor_id)));
    auto fut = _router->WriteToOne(it->second, MultiGPUCommand::IS_ENABLED_ROS, std::move(buf));

    auto response = fut.get();
    bool res = (*reinterpret_cast<bool *>(response.buffer.data()));
    return res;
  } else {
    log_error("is_enabled_for_ros for actor ", stream_actor_id.stream_id, " not found on any server");
    return false;
  }
}

token_type PrimaryCommands::GetToken(stream_id stream_id) {
  // search if the sensor has been activated in any secondary server
  auto it = _tokens.find(stream_id);
  if (it != _tokens.end()) {
    // return already activated sensor token
    log_debug("Using token from already activated sensor: ", it->second.get_stream_id(), ", ", it->second.get_port());
    return it->second;
  }
  else {
    // enable the sensor on one secondary server
    auto server = _router->GetNextServer();
    auto token = SendGetToken(stream_id);
    // add to the maps
    _tokens[stream_id] = token;
    _servers[stream_id] = server;
    log_debug("Using token from new activated sensor: ", token.get_stream_id(), ", ", token.get_port());
    return token;
  }
}

void PrimaryCommands::EnableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  auto it = _servers.find(stream_actor_id.stream_id);
  if (it != _servers.end()) {
    SendEnableForROS(stream_actor_id);
  } else {
    // we need to activate the actor in any server yet, and repeat
    GetToken(stream_actor_id.stream_id);
    EnableForROS(stream_actor_id);
  }
}

void PrimaryCommands::DisableForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  auto it = _servers.find(stream_actor_id.stream_id);
  if (it != _servers.end()) {
    SendDisableForROS(stream_actor_id);
  }
}

bool PrimaryCommands::IsEnabledForROS(carla::streaming::detail::stream_actor_id_type stream_actor_id) {
  auto it = _servers.find(stream_actor_id.stream_id);
  if (it != _servers.end()) {
    return SendIsEnabledForROS(stream_actor_id);
  }
  return false;
}

} // namespace multigpu
} // namespace carla
