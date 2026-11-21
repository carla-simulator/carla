// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

namespace carla {
namespace streaming {
namespace detail {

  /// Number of buckets of a LatencyHistogram.
  constexpr size_t kLatencyBuckets = 24u;

  /// Histogram of durations on a log2 scale of microseconds: bucket 0 counts
  /// durations below 2 us, bucket i (0 < i < last) those in [2^i, 2^(i+1)) us,
  /// and the last bucket everything from 2^23 us (about 8 s) up.
  class LatencyHistogram {
  public:

    LatencyHistogram() {
      for (auto &bucket : _buckets) {
        bucket.store(0u, std::memory_order_relaxed);
      }
    }

    static size_t BucketOf(uint64_t ns) {
      uint64_t us = ns / 1000u;
      size_t bucket = 0u;
      while ((us >= 2u) && (bucket + 1u < kLatencyBuckets)) {
        us >>= 1u;
        ++bucket;
      }
      return bucket;
    }

    void Add(uint64_t ns) {
      _buckets[BucketOf(ns)].fetch_add(1u, std::memory_order_relaxed);
    }

    std::array<uint64_t, kLatencyBuckets> Load() const {
      std::array<uint64_t, kLatencyBuckets> result;
      for (size_t i = 0u; i < kLatencyBuckets; ++i) {
        result[i] = _buckets[i].load(std::memory_order_relaxed);
      }
      return result;
    }

  private:

    std::array<std::atomic<uint64_t>, kLatencyBuckets> _buckets;
  };

  /// What the streaming server did with the messages of a set of sessions.
  /// Every value only grows; every update is a relaxed atomic add, safe from
  /// the producer threads (game, render, task threads) and the io threads.
  struct StreamCounters {

    using counter = std::atomic<uint64_t>;

    /// Sessions that joined and left.
    counter sessions_opened{0u};
    counter sessions_closed{0u};

    /// Messages written completely to the socket, and their bytes on the wire
    /// (the 4-byte size prefix included).
    counter writes{0u};
    counter wire_bytes{0u};

    /// Messages not sent:
    /// - busy_drops: asynchronous mode, the previous message was still being
    ///   written (the message is skipped, the session stays open);
    /// - closed_drops: the session was closing or closed (this includes the
    ///   message given up on by a synchronous time-out);
    /// - timeouts: synchronous mode, the client did not read for the whole
    ///   session time-out (the session is closed);
    /// - strand_drops: accepted, then dropped because the session closed
    ///   before its write started;
    /// - write_errors: the socket write failed (the session is closed).
    counter busy_drops{0u};
    counter closed_drops{0u};
    counter timeouts{0u};
    counter strand_drops{0u};
    counter write_errors{0u};

    /// Synchronous mode: writes that had to wait for the previous message, and
    /// the time the producer thread spent waiting.
    counter blocked_writes{0u};
    counter blocked_ns{0u};

    /// Time from a message being accepted to its write starting on an io
    /// thread (the io_context queue), and from the write starting to it
    /// completing (the socket taking every byte).
    counter queue_ns{0u};
    counter send_ns{0u};

    LatencyHistogram blocked_histogram;
    LatencyHistogram queue_histogram;
    LatencyHistogram send_histogram;
  };

  /// The streaming counters of this process, one set for the episode stream
  /// (the world snapshots) and one for every other stream (sensors).
  class StreamCountersRegistry {
  public:

    static StreamCounters &Episode() {
      static StreamCounters counters;
      return counters;
    }

    static StreamCounters &Other() {
      static StreamCounters counters;
      return counters;
    }

    static uint64_t NowNs() {
      return static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now().time_since_epoch()).count());
    }
  };

  /// One session's counters, read at a point in time.
  struct SessionCountersRow {
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
  };

} // namespace detail
} // namespace streaming
} // namespace carla
