// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/streaming/detail/StreamCounters.h"

#include <atomic>
#include <chrono>
#include <cstdint>

namespace carla {
namespace rpc {

  /// Cumulative counters of the server's RPC and frame work since it started.
  /// Read them twice and subtract to get the cost of what ran in between.
  class ServerPerfCounters {
  public:

    /// Frames whose RPC loop finished on the game thread.
    uint64_t frames = 0u;

    /// Calls run on the game thread (BindSync), and the time spent in them.
    uint64_t sync_calls = 0u;
    uint64_t sync_call_ns = 0u;

    /// Calls run on the RPC worker threads (BindAsync).
    uint64_t async_calls = 0u;

    /// Game-thread time spent in the per-frame RPC loop. In synchronous mode
    /// it includes the time waiting for the client's tick.
    uint64_t rpc_loop_ns = 0u;

    /// Game-thread calls still queued when a frame's RPC loop ended, summed
    /// over frames, and the largest such backlog.
    uint64_t backlog_sum = 0u;
    uint64_t backlog_max = 0u;

    /// Episode snapshots serialized, the game-thread time it took to collect
    /// them and their total size. This is what the server builds once per
    /// tick, before it goes to the sessions; snapshot_wire_bytes is what the
    /// sessions sent.
    uint64_t snapshots = 0u;
    uint64_t snapshot_ns = 0u;
    uint64_t snapshot_bytes = 0u;

    /// What the episode stream's sessions did with those snapshots: messages
    /// written to a socket and their bytes on the wire, messages that never
    /// reached a socket (skipped while the previous one was still being
    /// written in asynchronous mode, or the session was closing), the time
    /// the game thread waited for a slow session in synchronous mode, the
    /// time a message waited for a streaming thread, and the time the socket
    /// took to accept it. Totals over every session.
    uint64_t snapshot_writes = 0u;
    uint64_t snapshot_wire_bytes = 0u;
    uint64_t snapshot_drops = 0u;
    uint64_t snapshot_blocked_ns = 0u;
    uint64_t snapshot_queue_ns = 0u;
    uint64_t snapshot_send_ns = 0u;

    /// The same for every other stream (sensors).
    uint64_t stream_writes = 0u;
    uint64_t stream_wire_bytes = 0u;
    uint64_t stream_drops = 0u;
    uint64_t stream_blocked_ns = 0u;
    uint64_t stream_queue_ns = 0u;
    uint64_t stream_send_ns = 0u;

    MSGPACK_DEFINE_ARRAY(
        frames,
        sync_calls,
        sync_call_ns,
        async_calls,
        rpc_loop_ns,
        backlog_sum,
        backlog_max,
        snapshots,
        snapshot_ns,
        snapshot_bytes,
        snapshot_writes,
        snapshot_wire_bytes,
        snapshot_drops,
        snapshot_blocked_ns,
        snapshot_queue_ns,
        snapshot_send_ns,
        stream_writes,
        stream_wire_bytes,
        stream_drops,
        stream_blocked_ns,
        stream_queue_ns,
        stream_send_ns);
  };

  /// Process-wide accumulator behind ServerPerfCounters. Every update is a
  /// relaxed atomic add, so it is safe from the game thread and the RPC
  /// worker threads alike.
  class ServerPerfStats {
  public:

    static ServerPerfStats &Get() {
      static ServerPerfStats instance;
      return instance;
    }

    static uint64_t NowNs() {
      return static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    void OnSyncCallPosted() {
      _sync_posted.fetch_add(1u, std::memory_order_relaxed);
    }

    void OnSyncCallDone(uint64_t ns) {
      _sync_calls.fetch_add(1u, std::memory_order_relaxed);
      _sync_call_ns.fetch_add(ns, std::memory_order_relaxed);
    }

    void OnAsyncCall() {
      _async_calls.fetch_add(1u, std::memory_order_relaxed);
    }

    /// Called once per frame when its RPC loop ends.
    void OnRpcLoopDone(uint64_t ns) {
      _frames.fetch_add(1u, std::memory_order_relaxed);
      _rpc_loop_ns.fetch_add(ns, std::memory_order_relaxed);
      const uint64_t posted = _sync_posted.load(std::memory_order_relaxed);
      const uint64_t done = _sync_calls.load(std::memory_order_relaxed);
      const uint64_t backlog = posted > done ? posted - done : 0u;
      _backlog_sum.fetch_add(backlog, std::memory_order_relaxed);
      uint64_t max = _backlog_max.load(std::memory_order_relaxed);
      while (backlog > max &&
             !_backlog_max.compare_exchange_weak(max, backlog, std::memory_order_relaxed)) {}
    }

    void OnSnapshotSerialized(uint64_t ns, uint64_t bytes) {
      _snapshots.fetch_add(1u, std::memory_order_relaxed);
      _snapshot_ns.fetch_add(ns, std::memory_order_relaxed);
      _snapshot_bytes.fetch_add(bytes, std::memory_order_relaxed);
    }

    ServerPerfCounters Snapshot() const {
      ServerPerfCounters c;
      c.frames = _frames.load(std::memory_order_relaxed);
      c.sync_calls = _sync_calls.load(std::memory_order_relaxed);
      c.sync_call_ns = _sync_call_ns.load(std::memory_order_relaxed);
      c.async_calls = _async_calls.load(std::memory_order_relaxed);
      c.rpc_loop_ns = _rpc_loop_ns.load(std::memory_order_relaxed);
      c.backlog_sum = _backlog_sum.load(std::memory_order_relaxed);
      c.backlog_max = _backlog_max.load(std::memory_order_relaxed);
      c.snapshots = _snapshots.load(std::memory_order_relaxed);
      c.snapshot_ns = _snapshot_ns.load(std::memory_order_relaxed);
      c.snapshot_bytes = _snapshot_bytes.load(std::memory_order_relaxed);
      using Registry = streaming::detail::StreamCountersRegistry;
      ReadStreamCounters(Registry::Episode(),
          c.snapshot_writes, c.snapshot_wire_bytes, c.snapshot_drops,
          c.snapshot_blocked_ns, c.snapshot_queue_ns, c.snapshot_send_ns);
      ReadStreamCounters(Registry::Other(),
          c.stream_writes, c.stream_wire_bytes, c.stream_drops,
          c.stream_blocked_ns, c.stream_queue_ns, c.stream_send_ns);
      return c;
    }

  private:

    static void ReadStreamCounters(
        const streaming::detail::StreamCounters &s,
        uint64_t &writes,
        uint64_t &wire_bytes,
        uint64_t &drops,
        uint64_t &blocked_ns,
        uint64_t &queue_ns,
        uint64_t &send_ns) {
      constexpr auto relaxed = std::memory_order_relaxed;
      writes = s.writes.load(relaxed);
      wire_bytes = s.wire_bytes.load(relaxed);
      drops = s.busy_drops.load(relaxed) + s.closed_drops.load(relaxed) +
          s.strand_drops.load(relaxed) + s.write_errors.load(relaxed);
      blocked_ns = s.blocked_ns.load(relaxed);
      queue_ns = s.queue_ns.load(relaxed);
      send_ns = s.send_ns.load(relaxed);
    }

    ServerPerfStats() = default;

    std::atomic<uint64_t> _frames{0u};
    std::atomic<uint64_t> _sync_posted{0u};
    std::atomic<uint64_t> _sync_calls{0u};
    std::atomic<uint64_t> _sync_call_ns{0u};
    std::atomic<uint64_t> _async_calls{0u};
    std::atomic<uint64_t> _rpc_loop_ns{0u};
    std::atomic<uint64_t> _backlog_sum{0u};
    std::atomic<uint64_t> _backlog_max{0u};
    std::atomic<uint64_t> _snapshots{0u};
    std::atomic<uint64_t> _snapshot_ns{0u};
    std::atomic<uint64_t> _snapshot_bytes{0u};
  };

} // namespace rpc
} // namespace carla
