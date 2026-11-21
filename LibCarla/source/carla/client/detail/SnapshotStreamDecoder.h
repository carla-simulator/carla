// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/NonCopyable.h"
#include "carla/sensor/s11n/EpisodeStateCodec.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace carla {
namespace client {
namespace detail {

  /// What a client's world-snapshot stream received; see
  /// carla.World.get_snapshot_stream_stats. Every count only grows.
  struct SnapshotStreamStats {
    /// Messages received.
    uint64_t messages = 0u;
    /// Messages that were whole snapshots, and deltas against the previous
    /// one.
    uint64_t keyframes = 0u;
    uint64_t deltas = 0u;
    /// Times the client dropped its copy of the last snapshot and reconnected
    /// for a whole one (a message did not apply, or did not match its hash).
    uint64_t resyncs = 0u;
    /// Messages in another wire format (a server of another build).
    uint64_t incompatible = 0u;
    /// Bytes received, and bytes of the whole snapshots published.
    uint64_t wire_bytes = 0u;
    uint64_t published_bytes = 0u;
    /// Time decoding, and time in the whole stream callback (decoding, the
    /// snapshot update and the on_tick callbacks).
    uint64_t decode_ns = 0u;
    uint64_t callback_ns = 0u;
    /// With the environment variable CARLA_SNAPSHOT_DIGEST=1: a digest of the
    /// actor records of each published snapshot, ascending by id, for the
    /// last frames, as (frame, digest).
    bool digests_enabled = false;
    std::vector<std::pair<uint64_t, uint64_t>> digests;
  };

  /// Turns the messages of the world-snapshot stream back into whole
  /// snapshots. Decode runs on the stream's strand only; the statistics may
  /// be read from any thread.
  class SnapshotStreamDecoder : private NonCopyable {
  public:

    enum class Action {
      /// Publish the message (Decode may have replaced it).
      Publish,
      /// Ignore the message.
      Skip,
      /// Reconnect for a whole snapshot.
      Resync
    };

    SnapshotStreamDecoder();

    Action Decode(Buffer &message);

    void AddCallbackTime(uint64_t ns) {
      _callback_ns.fetch_add(ns, std::memory_order_relaxed);
    }

    uint64_t GetResyncCount() const {
      return _resyncs.load(std::memory_order_relaxed);
    }

    SnapshotStreamStats GetStats() const;

    static uint64_t NowNs();

  private:

    void AddDigest(const Buffer &message);

    sensor::s11n::episode_codec::DeltaDecoder _decoder;

    bool _incompatible_logged = false;

    const bool _digests_enabled;

    std::atomic<uint64_t> _messages{0u};
    std::atomic<uint64_t> _keyframes{0u};
    std::atomic<uint64_t> _deltas{0u};
    std::atomic<uint64_t> _resyncs{0u};
    std::atomic<uint64_t> _incompatible{0u};
    std::atomic<uint64_t> _wire_bytes{0u};
    std::atomic<uint64_t> _published_bytes{0u};
    std::atomic<uint64_t> _decode_ns{0u};
    std::atomic<uint64_t> _callback_ns{0u};

    mutable std::mutex _digest_mutex;
    std::deque<std::pair<uint64_t, uint64_t>> _digests;
  };

} // namespace detail
} // namespace client
} // namespace carla
