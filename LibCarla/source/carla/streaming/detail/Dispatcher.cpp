// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/streaming/detail/Dispatcher.h"

#include "carla/Logging.h"

#include <exception>

namespace carla {
namespace streaming {
namespace detail {

  Dispatcher::~Dispatcher() {
    // Disconnect all the sessions from their streams, this should kill any
    // session remaining since at this point the io_service should be already
    // stopped.
    for (auto &pair : _stream_map) {
      pair.second->set_session(nullptr);
    }
  }

  Stream Dispatcher::MakeStream() {
    std::lock_guard<std::mutex> lock(_mutex);
    ++_cached_token._token.stream_id; // id zero only happens in overflow.
    auto ptr = std::make_shared<StreamState>(_cached_token);
    auto result = _stream_map.emplace(std::make_pair(_cached_token.get_stream_id(), ptr));
    if (!result.second) {
      throw std::runtime_error("failed to create stream!");
    }
    return ptr;
  }

  void Dispatcher::RegisterSession(std::shared_ptr<Session> session) {
    DEBUG_ASSERT(session != nullptr);
    std::lock_guard<std::mutex> lock(_mutex);
    auto search = _stream_map.find(session->get_stream_id());
    if (search != _stream_map.end()) {
      DEBUG_ASSERT(search->second != nullptr);
      search->second->set_session(std::move(session));
    } else {
      log_error("Invalid session: no stream available with id", session->get_stream_id());
    }
  }

} // namespace detail
} // namespace streaming
} // namespace carla
