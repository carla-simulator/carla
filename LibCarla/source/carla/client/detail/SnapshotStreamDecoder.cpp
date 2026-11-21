// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/SnapshotStreamDecoder.h"

#include "carla/Logging.h"
#include "carla/sensor/s11n/SensorHeaderSerializer.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <string>

namespace carla {
namespace client {
namespace detail {

  namespace codec = sensor::s11n::episode_codec;

  static constexpr size_t kSensorHeaderSize = sensor::s11n::SensorHeaderSerializer::header_offset;

  /// Frames whose digest is kept.
  static constexpr size_t kDigestHistory = 4096u;

  static bool DigestsEnabled() {
    const char *value = std::getenv("CARLA_SNAPSHOT_DIGEST");
    return (value != nullptr) && (std::string(value) == "1");
  }

  SnapshotStreamDecoder::SnapshotStreamDecoder()
    : _digests_enabled(DigestsEnabled()) {}

  uint64_t SnapshotStreamDecoder::NowNs() {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
  }

  SnapshotStreamDecoder::Action SnapshotStreamDecoder::Decode(Buffer &message) {
    constexpr auto relaxed = std::memory_order_relaxed;
    const uint64_t start = NowNs();
    _messages.fetch_add(1u, relaxed);
    _wire_bytes.fetch_add(message.size(), relaxed);

    Action action = Action::Publish;
    if (message.size() < kSensorHeaderSize) {
      log_warning("snapshot stream: message shorter than its sensor header; resynchronizing");
      _decoder.Reset();
      _resyncs.fetch_add(1u, relaxed);
      action = Action::Resync;
    } else {
      Buffer rebuilt;
      const auto result = _decoder.Decode(
          message.data() + kSensorHeaderSize,
          message.size() - kSensorHeaderSize,
          [&](size_t size) {
            rebuilt.reset(static_cast<uint64_t>(kSensorHeaderSize + size));
            std::memcpy(rebuilt.data(), message.data(), kSensorHeaderSize);
            return rebuilt.data() + kSensorHeaderSize;
          });
      switch (result.status) {
        case codec::DecodeStatus::Keyframe:
          _keyframes.fetch_add(1u, relaxed);
          break;
        case codec::DecodeStatus::Delta:
          _deltas.fetch_add(1u, relaxed);
          message = std::move(rebuilt);
          break;
        case codec::DecodeStatus::NeedResync:
          log_warning("snapshot stream:", result.error, "; resynchronizing");
          _resyncs.fetch_add(1u, relaxed);
          action = Action::Resync;
          break;
        case codec::DecodeStatus::Incompatible:
          _incompatible.fetch_add(1u, relaxed);
          if (!_incompatible_logged) {
            _incompatible_logged = true;
            log_error("snapshot stream:", result.error,
                "; the server is from another build, client and server must match");
          }
          action = Action::Skip;
          break;
      }
    }
    if (action == Action::Publish) {
      _published_bytes.fetch_add(message.size(), relaxed);
      if (_digests_enabled) {
        AddDigest(message);
      }
    }
    _decode_ns.fetch_add(NowNs() - start, relaxed);
    return action;
  }

  void SnapshotStreamDecoder::AddDigest(const Buffer &message) {
    uint64_t frame;
    std::memcpy(
        &frame,
        message.data() + offsetof(sensor::s11n::SensorHeaderSerializer::Header, frame),
        sizeof(frame));
    // FNV-1a over the records ascending by id (the decoder's copy is the
    // published snapshot in that order), so that it does not share code with
    // the state hash it checks.
    const auto &records = _decoder.GetBase();
    uint64_t digest = 14695981039346656037ull;
    for (const uint8_t byte : records) {
      digest ^= byte;
      digest *= 1099511628211ull;
    }
    std::lock_guard<std::mutex> lock(_digest_mutex);
    _digests.emplace_back(frame, digest);
    while (_digests.size() > kDigestHistory) {
      _digests.pop_front();
    }
  }

  SnapshotStreamStats SnapshotStreamDecoder::GetStats() const {
    constexpr auto relaxed = std::memory_order_relaxed;
    SnapshotStreamStats stats;
    stats.messages = _messages.load(relaxed);
    stats.keyframes = _keyframes.load(relaxed);
    stats.deltas = _deltas.load(relaxed);
    stats.resyncs = _resyncs.load(relaxed);
    stats.incompatible = _incompatible.load(relaxed);
    stats.wire_bytes = _wire_bytes.load(relaxed);
    stats.published_bytes = _published_bytes.load(relaxed);
    stats.decode_ns = _decode_ns.load(relaxed);
    stats.callback_ns = _callback_ns.load(relaxed);
    stats.digests_enabled = _digests_enabled;
    if (_digests_enabled) {
      std::lock_guard<std::mutex> lock(_digest_mutex);
      stats.digests.assign(_digests.begin(), _digests.end());
    }
    return stats;
  }

} // namespace detail
} // namespace client
} // namespace carla
