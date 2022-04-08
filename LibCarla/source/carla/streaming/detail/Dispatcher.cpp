// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/Dispatcher.h"

#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/streaming/detail/MultiStreamState.h"

#include <exception>

namespace carla {
namespace streaming {
namespace detail {

  Dispatcher::~Dispatcher() {
    // Disconnect all the sessions from their streams, this should kill any
    // session remaining since at this point the io_context should be already
    // stopped.
    for (auto &pair : _stream_map) {
#ifndef LIBCARLA_NO_EXCEPTIONS
      try {
#endif // LIBCARLA_NO_EXCEPTIONS
        auto stream_state = pair.second.lock();
        if (stream_state != nullptr) {
          stream_state->ClearSessions();
        }
#ifndef LIBCARLA_NO_EXCEPTIONS
      } catch (const std::exception &e) {
        log_error("failed to clear sessions:", e.what());
      }
#endif // LIBCARLA_NO_EXCEPTIONS
    }
  }

  carla::streaming::Stream Dispatcher::MakeStream() {
    std::lock_guard<std::mutex> lock(_mutex);
    ++_cached_token._token.stream_id; // id zero only happens in overflow.
    log_info("Created new stream:", _cached_token._token.stream_id);
    std::shared_ptr<MultiStreamState> ptr;
    auto search = _stream_map.find(_cached_token.get_stream_id());
    if (search == _stream_map.end()) {
      ptr = std::make_shared<MultiStreamState>(_cached_token);
      auto result = _stream_map.emplace(std::make_pair(_cached_token.get_stream_id(), ptr));
      if (!result.second) {
        throw_exception(std::runtime_error("failed to create stream!"));
      }
      return carla::streaming::Stream(ptr);
    } else {
      ptr = search->second.lock();
      return carla::streaming::Stream(ptr);
    }
  }

  bool Dispatcher::RegisterSession(std::shared_ptr<Session> session) {
    DEBUG_ASSERT(session != nullptr);
    std::lock_guard<std::mutex> lock(_mutex);
    auto search = _stream_map.find(session->get_stream_id());
    if (search != _stream_map.end()) {
      auto stream_state = search->second.lock();
      if (stream_state != nullptr) {
        log_info("Connecting session (stream ", session->get_stream_id(), ")");
        stream_state->ConnectSession(std::move(session));
        return true;
      }
    }
    // log_error("Invalid session: no stream available with id", session->get_stream_id());
    return false;
  }

  void Dispatcher::DeregisterSession(std::shared_ptr<Session> session) {
    DEBUG_ASSERT(session != nullptr);
    std::lock_guard<std::mutex> lock(_mutex);
    ClearExpiredStreams();
    auto search = _stream_map.find(session->get_stream_id());
    if (search != _stream_map.end()) {
      auto stream_state = search->second.lock();
      if (stream_state != nullptr) {
        log_info("Disconnecting session (stream ", session->get_stream_id(), ")");
        stream_state->DisconnectSession(session);
      }
    }
  }

  void Dispatcher::ClearExpiredStreams() {
    for (auto it = _stream_map.begin(); it != _stream_map.end(); ) {
      if (it->second.expired()) {
        it = _stream_map.erase(it);
      } else {
        ++it;
      }
    }
  }

  token_type Dispatcher::GetToken(stream_id_type sensor_id) {
    std::lock_guard<std::mutex> lock(_mutex);
    log_info("Searching sensor id: ", sensor_id);
    auto search = _stream_map.find(sensor_id);
    if (search != _stream_map.end()) {
      log_info("Found sensor id: ", sensor_id);
      auto stream_state = search->second.lock();
      if (stream_state != nullptr) {
        log_info("Getting token from stream ", sensor_id, " on port ", stream_state->token().get_port());
        return stream_state->token();
      } else {
        log_info("Could not lock");
      }
    } else {
      log_info("Not Found sensor id, creating sensor stream: ", sensor_id);
      token_type temp_token(_cached_token);
      temp_token.set_stream_id(sensor_id);
      auto ptr = std::make_shared<MultiStreamState>(temp_token);
      auto result = _stream_map.emplace(std::make_pair(temp_token.get_stream_id(), ptr));
      log_info("Created token from stream ", sensor_id, " on port ", temp_token.get_port());
      return temp_token;
    }
    return token_type();
  }

} // namespace detail
} // namespace streaming
} // namespace carla
