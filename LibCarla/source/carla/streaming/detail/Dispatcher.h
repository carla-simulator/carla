// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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
#include <optional>
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

    /// Like GetToken(), but returns std::nullopt for an unknown @a stream_id
    /// instead of creating an empty stream for it.
    [[nodiscard]]
    std::optional<token_type> FindToken(stream_id_type stream_id);

    /// When enabled, a session subscribes through an alias (see
    /// SetStreamAlias) and a stream id without an alias is refused, since
    /// aliases and local stream ids are separate id spaces that may overlap.
    void SetSessionsUseStreamAliases(bool enabled);

    /// Routes sessions subscribing to @a alias to the local stream @a stream_id.
    void SetStreamAlias(stream_id_type alias, stream_id_type stream_id);

    void RemoveStreamAlias(stream_id_type alias);

    [[nodiscard]]
    std::optional<stream_id_type> FindStreamAlias(stream_id_type alias);

    void SetROS2TopicVisibilityDefaultEnabled(bool enabled) {
      std::scoped_lock<std::mutex> lock(_mutex);
      _topic_visibility_default_enabled = enabled;
    }

    void EnableForROS(stream_id_type sensor_id) {
      std::scoped_lock<std::mutex> lock(_mutex);
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        search->second->EnableForROS();
      }
    }

    void DisableForROS(stream_id_type sensor_id) {
      std::scoped_lock<std::mutex> lock(_mutex);
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        search->second->DisableForROS();
      }
    }

    bool IsEnabledForROS(stream_id_type sensor_id) {
      std::scoped_lock<std::mutex> lock(_mutex);
      auto search = _stream_map.find(sensor_id);
      if (search != _stream_map.end()) {
        return search->second->IsEnabledForROS();
      }
      return false;
    }

  private:

    /// Stream a session subscribing to @a session_stream_id belongs to.
    std::optional<stream_id_type> ResolveSessionStreamId(stream_id_type session_stream_id) const;

    // We use a mutex here, but we assume that sessions and streams won't be
    // created too often.
    std::mutex _mutex;

    token_type _cached_token;

    StreamMap _stream_map;

    bool _sessions_use_stream_aliases{false};

    std::unordered_map<stream_id_type, stream_id_type> _stream_aliases;

    std::unordered_map<const Session *, std::weak_ptr<MultiStreamState>> _session_streams;

    // When true, every newly created stream starts visible to ROS 2 without an
    // explicit EnableForROS call. Driven by the ROS2TopicVisibility setting.
    bool _topic_visibility_default_enabled{false};
  };

} // namespace detail
} // namespace streaming
} // namespace carla
