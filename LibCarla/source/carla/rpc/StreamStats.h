// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/streaming/detail/StreamCounters.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace carla {
namespace rpc {

  /// Streaming counters of one class of streams: the episode stream (world
  /// snapshots) or every other stream (sensors). See
  /// streaming::detail::StreamCounters for what each value counts.
  struct StreamClassStats {
    uint64_t sessions_opened = 0u;
    uint64_t sessions_closed = 0u;
    uint64_t writes = 0u;
    uint64_t wire_bytes = 0u;
    uint64_t busy_drops = 0u;
    uint64_t closed_drops = 0u;
    uint64_t timeouts = 0u;
    uint64_t strand_drops = 0u;
    uint64_t write_errors = 0u;
    uint64_t blocked_writes = 0u;
    uint64_t blocked_ns = 0u;
    uint64_t queue_ns = 0u;
    uint64_t send_ns = 0u;
    /// Log2 histograms of microseconds, see streaming::detail::LatencyHistogram.
    std::vector<uint64_t> blocked_histogram;
    std::vector<uint64_t> queue_histogram;
    std::vector<uint64_t> send_histogram;

    static StreamClassStats From(const streaming::detail::StreamCounters &c) {
      constexpr auto relaxed = std::memory_order_relaxed;
      StreamClassStats s;
      s.sessions_opened = c.sessions_opened.load(relaxed);
      s.sessions_closed = c.sessions_closed.load(relaxed);
      s.writes = c.writes.load(relaxed);
      s.wire_bytes = c.wire_bytes.load(relaxed);
      s.busy_drops = c.busy_drops.load(relaxed);
      s.closed_drops = c.closed_drops.load(relaxed);
      s.timeouts = c.timeouts.load(relaxed);
      s.strand_drops = c.strand_drops.load(relaxed);
      s.write_errors = c.write_errors.load(relaxed);
      s.blocked_writes = c.blocked_writes.load(relaxed);
      s.blocked_ns = c.blocked_ns.load(relaxed);
      s.queue_ns = c.queue_ns.load(relaxed);
      s.send_ns = c.send_ns.load(relaxed);
      const auto blocked = c.blocked_histogram.Load();
      const auto queue = c.queue_histogram.Load();
      const auto send = c.send_histogram.Load();
      s.blocked_histogram.assign(blocked.begin(), blocked.end());
      s.queue_histogram.assign(queue.begin(), queue.end());
      s.send_histogram.assign(send.begin(), send.end());
      return s;
    }

    /// Messages that never reached the socket, whatever the reason.
    uint64_t Drops() const {
      return busy_drops + closed_drops + strand_drops + write_errors;
    }

    MSGPACK_DEFINE_ARRAY(
        sessions_opened,
        sessions_closed,
        writes,
        wire_bytes,
        busy_drops,
        closed_drops,
        timeouts,
        strand_drops,
        write_errors,
        blocked_writes,
        blocked_ns,
        queue_ns,
        send_ns,
        blocked_histogram,
        queue_histogram,
        send_histogram);
  };

  /// One streaming session (one subscription of one client to one stream).
  struct SessionStreamStats {
    uint32_t stream_id = 0u;
    uint64_t session_id = 0u;
    bool episode = false;
    std::string remote;
    bool loopback = false;
    uint64_t writes = 0u;
    uint64_t wire_bytes = 0u;
    uint64_t busy_drops = 0u;
    uint64_t closed_drops = 0u;
    uint64_t timeouts = 0u;
    uint64_t blocked_ns = 0u;
    uint64_t queue_ns = 0u;
    uint64_t send_ns = 0u;

    static SessionStreamStats From(const streaming::detail::SessionCountersRow &r) {
      SessionStreamStats s;
      s.stream_id = r.stream_id;
      s.session_id = r.session_id;
      s.episode = r.episode;
      s.remote = r.remote;
      s.loopback = r.loopback;
      s.writes = r.writes;
      s.wire_bytes = r.wire_bytes;
      s.busy_drops = r.busy_drops;
      s.closed_drops = r.closed_drops;
      s.timeouts = r.timeouts;
      s.blocked_ns = r.blocked_ns;
      s.queue_ns = r.queue_ns;
      s.send_ns = r.send_ns;
      return s;
    }

    MSGPACK_DEFINE_ARRAY(
        stream_id,
        session_id,
        episode,
        remote,
        loopback,
        writes,
        wire_bytes,
        busy_drops,
        closed_drops,
        timeouts,
        blocked_ns,
        queue_ns,
        send_ns);
  };

  /// How the actor records of the world snapshot changed from one tick to the
  /// next, measured on the server while the console variable
  /// carla.Snapshot.Stats is 1. It sizes the snapshot deltas before they
  /// exist on the wire.
  struct SnapshotChangeStats {

    /// Actor types, in the order of the rows of by_type (the order of the
    /// server's FCarlaActor::ActorType).
    static constexpr size_t kTypeCount = 6u;

    /// Columns of a by_type row: records, unchanged, added, then the number of
    /// records whose field group 0..5 changed.
    static constexpr size_t kRecords = 0u;
    static constexpr size_t kUnchanged = 1u;
    static constexpr size_t kAdded = 2u;
    static constexpr size_t kFirstGroup = 3u;
    static constexpr size_t kGroupCount = 6u;
    static constexpr size_t kColumns = kFirstGroup + kGroupCount;

    /// Ticks compared with the previous tick.
    uint64_t frames = 0u;
    /// Ticks with nothing to compare with (the first one, a new episode).
    uint64_t keyframes = 0u;
    /// Actors that left the snapshot from one tick to the next.
    uint64_t removed = 0u;
    /// Bytes of the snapshot bodies of the compared ticks as keyframes, and as
    /// deltas, in the delta format (header included).
    uint64_t keyframe_bytes = 0u;
    uint64_t delta_bytes = 0u;
    /// kTypeCount rows of kColumns values.
    std::vector<std::vector<uint64_t>> by_type;

    SnapshotChangeStats()
      : by_type(kTypeCount, std::vector<uint64_t>(kColumns, 0u)) {}

    void Add(const SnapshotChangeStats &rhs) {
      frames += rhs.frames;
      keyframes += rhs.keyframes;
      removed += rhs.removed;
      keyframe_bytes += rhs.keyframe_bytes;
      delta_bytes += rhs.delta_bytes;
      for (size_t t = 0u; t < kTypeCount && t < rhs.by_type.size(); ++t) {
        for (size_t c = 0u; c < kColumns && c < rhs.by_type[t].size(); ++c) {
          by_type[t][c] += rhs.by_type[t][c];
        }
      }
    }

    MSGPACK_DEFINE_ARRAY(
        frames,
        keyframes,
        removed,
        keyframe_bytes,
        delta_bytes,
        by_type);
  };

  /// Process-wide accumulator behind SnapshotChangeStats. The game thread adds
  /// one tick at a time, an RPC thread reads it.
  class SnapshotChangeAccumulator {
  public:

    static SnapshotChangeAccumulator &Get() {
      static SnapshotChangeAccumulator instance;
      return instance;
    }

    void Add(const SnapshotChangeStats &tick) {
      std::scoped_lock<std::mutex> lock(_mutex);
      _stats.Add(tick);
    }

    SnapshotChangeStats Read() const {
      std::scoped_lock<std::mutex> lock(_mutex);
      return _stats;
    }

  private:

    SnapshotChangeAccumulator() = default;

    mutable std::mutex _mutex;

    SnapshotChangeStats _stats;
  };

  /// Everything the streaming server counted, as returned by
  /// carla.Client.get_stream_stats. Every value only grows.
  struct StreamStats {
    StreamClassStats episode;
    StreamClassStats other;
    std::vector<SessionStreamStats> sessions;
    SnapshotChangeStats snapshot_changes;

    MSGPACK_DEFINE_ARRAY(
        episode,
        other,
        sessions,
        snapshot_changes);
  };

} // namespace rpc
} // namespace carla
