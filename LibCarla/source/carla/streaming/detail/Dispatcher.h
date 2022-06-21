// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/EndPoint.h"
#include "carla/streaming/Stream.h"
#include "carla/streaming/detail/Session.h"
#include "carla/streaming/detail/Token.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace carla {
namespace streaming {
namespace detail {

  class StreamStateBase;

  /// Keeps the mapping between streams and sessions.
  class Dispatcher {
  public:

    template <typename Protocol, typename EndPointType>
    explicit Dispatcher(const EndPoint<Protocol, EndPointType> &ep)
      : _cached_token(0u, ep) {}

    ~Dispatcher();

    carla::streaming::Stream MakeStream();

    bool RegisterSession(std::shared_ptr<Session> session);

    void DeregisterSession(std::shared_ptr<Session> session);

  private:

    void ClearExpiredStreams();

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;

    token_type _cached_token;

    std::unordered_map<
        stream_id_type,
        std::weak_ptr<StreamStateBase>> _stream_map;
  };

} // namespace detail
} // namespace streaming
} // namespace carla
