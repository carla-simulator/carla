// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/Dispatcher.h"

#include "carla/Exception.h"
#include "carla/Logging.h"
#include "carla/streaming/detail/MultiStreamState.h"
#include "carla/streaming/detail/StreamState.h"

#include <exception>

namespace carla {
namespace streaming {
namespace detail {

  template <typename StreamStateT, typename StreamMapT>
  static auto MakeStreamState(const token_type &cached_token, StreamMapT &stream_map) {
    auto ptr = std::make_shared<StreamStateT>(cached_token);
    auto result = stream_map.emplace(std::make_pair(cached_token.get_stream_id(), ptr));
    if (!result.second) {
      throw_exception(std::runtime_error("failed to create stream!"));
    }
    return ptr;
  }

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
    return MakeStreamState<StreamState>(_cached_token, _stream_map);
  }

  carla::streaming::MultiStream Dispatcher::MakeMultiStream() {
    std::lock_guard<std::mutex> lock(_mutex);
    ++_cached_token._token.stream_id; // id zero only happens in overflow.
    return MakeStreamState<MultiStreamState>(_cached_token, _stream_map);
  }

  bool Dispatcher::RegisterSession(std::shared_ptr<Session> session) {
    DEBUG_ASSERT(session != nullptr);
    std::lock_guard<std::mutex> lock(_mutex);
    auto search = _stream_map.find(session->get_stream_id());
    if (search != _stream_map.end()) {
      auto stream_state = search->second.lock();
      if (stream_state != nullptr) {
        stream_state->ConnectSession(std::move(session));
        return true;
      }
    }
    log_error("Invalid session: no stream available with id", session->get_stream_id());
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

} // namespace detail
} // namespace streaming
} // namespace carla
