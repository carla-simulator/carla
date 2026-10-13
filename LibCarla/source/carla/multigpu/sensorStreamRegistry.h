// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/streaming/Server.h"
#include "carla/streaming/Stream.h"
#include "carla/streaming/detail/Token.h"
#include "carla/streaming/detail/Types.h"

#include <cstdint>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace carla {
namespace multigpu {

/// Secondary-side map from a primary sensor actor to its local stream, exposed
/// to clients only through an alias of the primary's stream id. Entries are
/// per primary episode. Resolve() and BeginEpisodeChange() run on the multi-GPU
/// command thread, the rest on the game thread.
class SensorStreamRegistry {
public:
  using actor_id_type = uint32_t;
  using epoch_type = uint64_t;
  using stream_id_type = carla::streaming::detail::stream_id_type;
  using token_type = carla::streaming::detail::token_type;

  /// Token that serves primary actor @a actor_id under @a primary_stream_id,
  /// reserving a local stream if the sensor has not spawned here yet.
  [[nodiscard]]
  std::optional<token_type> Resolve(
      carla::streaming::Server &server,
      actor_id_type actor_id,
      stream_id_type primary_stream_id);

  /// Returns the stream reserved for @a actor_id if @a can_adopt (the sensor
  /// has never ticked) and the sensor must adopt it; otherwise the alias is
  /// retargeted to @a sensor_stream_id. A stream bound to another actor is refused.
  [[nodiscard]]
  std::optional<carla::streaming::Stream> Bind(
      carla::streaming::Server &server,
      actor_id_type actor_id,
      stream_id_type sensor_stream_id,
      bool can_adopt);

  /// Forgets @a actor_id, closing its reserved stream if no sensor adopted it.
  void Unbind(carla::streaming::Server &server, actor_id_type actor_id);

  /// The primary switched episode: forgets every actor and returns the epoch
  /// the secondary must pass to OpenEpisode() once it runs the new episode.
  [[nodiscard]]
  epoch_type BeginEpisodeChange(carla::streaming::Server &server);

  /// Accepts binding if @a epoch is the latest one BeginEpisodeChange() issued.
  void OpenEpisode(epoch_type epoch);

  void CloseEpisode();

  /// Whether Bind()/Unbind() are accepted, i.e. the episode of the latest
  /// BeginEpisodeChange() is open.
  [[nodiscard]]
  bool IsEpisodeOpen();

  /// The primary forgot every route (this secondary reconnected): drops all
  /// aliases so no sensor stays owned until Resolve() grants it again.
  void ResetOwnership(carla::streaming::Server &server);

  /// Whether the primary routed the sensor of @a local_stream_id to this
  /// secondary, i.e. Resolve() granted its token in the current episode.
  [[nodiscard]]
  bool IsOwnedStream(stream_id_type local_stream_id) const;

private:
  struct Entry {
    stream_id_type stream_id;
    std::optional<carla::streaming::Stream> reserved;
    std::optional<stream_id_type> primary_stream_id;
  };

  static void Forget(carla::streaming::Server &server, const Entry &entry);

  mutable std::mutex _mutex;
  epoch_type _epoch = 0u;
  bool _bindable = false;
  std::unordered_map<actor_id_type, Entry> _entries;
};

} // namespace multigpu
} // namespace carla
