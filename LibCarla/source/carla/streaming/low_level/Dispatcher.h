// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/Stream.h"
#include "carla/streaming/low_level/Session.h"
#include "carla/streaming/low_level/StreamState.h"
#include "carla/streaming/low_level/Token.h"

#include <exception>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace carla {
namespace streaming {
namespace low_level {

  class Dispatcher {
  public:

    template <typename P>
    explicit Dispatcher(const boost::asio::ip::basic_endpoint<P> &ep)
      : _cached_token(0u, ep) {}

    Stream MakeStream() {
      std::lock_guard<std::mutex> lock(_mutex);
      ++_cached_token._token.stream_id; // id zero only happens in overflow.
      auto ptr = std::make_shared<StreamState>(_cached_token);
      auto result = _stream_map.emplace(std::make_pair(_cached_token.get_stream_id(), ptr));
      if (!result.second) {
        throw std::runtime_error("failed to create stream!");
      }
      return ptr;
    }

    void RegisterSession(std::shared_ptr<Session> session) {
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

  private:

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;

    token_type _cached_token;

    std::unordered_map<
        stream_id_type,
        std::shared_ptr<StreamState>> _stream_map;
  };

} // namespace low_level
} // namespace streaming
} // namespace carla
