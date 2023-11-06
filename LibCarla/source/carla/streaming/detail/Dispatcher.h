// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/EndPoint.h"
#include "carla/streaming/Stream.h"
#include "carla/streaming/detail/Session.h"
#include "carla/streaming/detail/Stream.h"
#include "carla/streaming/detail/Token.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace carla {
namespace streaming {
namespace detail {

  class MultiStreamState;
  using StreamMap = std::unordered_map<stream_id_type, std::shared_ptr<MultiStreamState>>;

  /// Keeps the mapping between streams and sessions.
  class Dispatcher {
  public:

    template <typename Protocol, typename EndPointType>
    explicit Dispatcher(const EndPoint<Protocol, EndPointType> &ep)
      : _cached_token(0u, ep) {}

    ~Dispatcher();

    carla::streaming::Stream MakeStream();

    void CloseStream(carla::streaming::detail::stream_id_type id);

    bool RegisterSession(std::shared_ptr<Session> session);

    void DeregisterSession(std::shared_ptr<Session> session);

    token_type GetToken(stream_id_type sensor_id);

    void EnableForROS(stream_id_type sensor_id) {
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        search->second->EnableForROS();
      }
    }

    void DisableForROS(stream_id_type sensor_id) {
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        search->second->DisableForROS();
      }
    }

    bool IsEnabledForROS(stream_id_type sensor_id) {
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        return search->second->IsEnabledForROS();
      }
      return false;
    }

  private:

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;

    token_type _cached_token;

    StreamMap _stream_map;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
