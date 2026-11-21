// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/ActorDynamicState.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <numeric>
#include <vector>

// Header-only so that the Unreal module and the client share one definition
// without adding s11n sources to the server library.

namespace carla {
namespace sensor {
namespace s11n {
namespace episode_codec {

  using Record = data::ActorDynamicState;

  constexpr size_t kRecordSize = sizeof(Record);

  constexpr size_t kIdSize = sizeof(Record::id);

  /// A byte range [begin, end) of an actor record. A snapshot delta sends a
  /// changed record group by group, so that, for example, a moving vehicle
  /// does not resend the fields that stay the same.
  struct FieldGroup {
    uint32_t begin;
    uint32_t end;
  };

  constexpr size_t kGroupCount = 6u;

  constexpr size_t kStateOffset = offsetof(Record, state);

  /// The field groups, covering every byte of a record after its id once:
  /// 0. actor state;
  /// 1. location (double) and rotation;
  /// 2. velocity, angular velocity and acceleration;
  /// 3. vehicle or walker control (the start of a sign id);
  /// 4. the rest of the vehicle data (speed limit, traffic light, failure,
  ///    doors), the rest of a sign id and a traffic light's fixed timings;
  /// 5. a traffic light's elapsed time, pole, frozen flag and state (zeros
  ///    for the other actor types).
  constexpr std::array<FieldGroup, kGroupCount> kGroups = {{
    {offsetof(Record, actor_state), offsetof(Record, location)},
    {offsetof(Record, location), offsetof(Record, velocity)},
    {offsetof(Record, velocity), kStateOffset},
    {kStateOffset, kStateOffset + offsetof(data::detail::VehicleData, speed_limit)},
    {kStateOffset + offsetof(data::detail::VehicleData, speed_limit),
     kStateOffset + offsetof(data::detail::TrafficLightData, elapsed_time)},
    {kStateOffset + offsetof(data::detail::TrafficLightData, elapsed_time), kRecordSize},
  }};

  // The groups are part of the wire format: pin them.
  static_assert(kIdSize == 4u, "record id must be 4 bytes");
  static_assert(kRecordSize == 131u, "the field groups assume a 131-byte record");
  static_assert(kGroups[0].begin == kIdSize, "group 0 must start right after the id");
  static_assert(kGroups[0].end == 5u && kGroups[1].end == 41u && kGroups[2].end == 77u &&
                kGroups[3].end == 96u && kGroups[4].end == 121u && kGroups[5].end == 131u,
                "field group boundaries changed: this changes the wire format");
  static_assert(kGroups[1].begin == kGroups[0].end && kGroups[2].begin == kGroups[1].end &&
                kGroups[3].begin == kGroups[2].end && kGroups[4].begin == kGroups[3].end &&
                kGroups[5].begin == kGroups[4].end,
                "field groups must be contiguous");

  constexpr uint8_t kAllGroups = static_cast<uint8_t>((1u << kGroupCount) - 1u);

  inline uint32_t ReadId(const uint8_t *record) {
    uint32_t id;
    std::memcpy(&id, record, sizeof(id));
    return id;
  }

  /// Bit g set when group g of the two records differs, bit for bit (so -0.0
  /// against +0.0, or two NaNs with different payloads, count as changes).
  inline uint8_t ChangedGroups(const uint8_t *before, const uint8_t *after) {
    uint8_t mask = 0u;
    for (size_t g = 0u; g < kGroupCount; ++g) {
      const auto &group = kGroups[g];
      if (std::memcmp(before + group.begin, after + group.begin, group.end - group.begin) != 0) {
        mask = static_cast<uint8_t>(mask | (1u << g));
      }
    }
    return mask;
  }

  /// Bytes of the groups set in @a mask.
  inline size_t GroupBytes(uint8_t mask) {
    size_t bytes = 0u;
    for (size_t g = 0u; g < kGroupCount; ++g) {
      if ((mask & (1u << g)) != 0u) {
        bytes += kGroups[g].end - kGroups[g].begin;
      }
    }
    return bytes;
  }

  /// Size of the header of a snapshot body in the delta-capable format.
  constexpr size_t kHeaderSize = 76u;

  /// What a frame changed against the previous one, in ascending id order.
  struct FrameDiff {
    /// Ids that left, ascending.
    std::vector<uint32_t> removed;
    /// Records that joined, as indices into the frame, by ascending id.
    std::vector<uint32_t> added;
    /// Records present in both frames, as indices into the frame, by
    /// ascending id, and the groups that changed in each.
    std::vector<uint32_t> survivors;
    std::vector<uint8_t> masks;

    void Clear() {
      removed.clear();
      added.clear();
      survivors.clear();
      masks.clear();
    }

    /// Size of the body of a delta carrying this diff: header, the removed
    /// and added counts, the removed ids, the added records, the bitmap of
    /// changed survivors, then a group mask and the groups of each changed
    /// survivor.
    size_t DeltaSize() const {
      size_t size = kHeaderSize + 2u * sizeof(uint32_t);
      size += removed.size() * sizeof(uint32_t);
      size += added.size() * kRecordSize;
      size += (survivors.size() + 7u) / 8u;
      for (const uint8_t mask : masks) {
        if (mask != 0u) {
          size += 1u + GroupBytes(mask);
        }
      }
      return size;
    }
  };

  /// Size of the body of a keyframe carrying @a count records.
  inline size_t KeyframeSize(size_t count) {
    return kHeaderSize + count * kRecordSize;
  }

  /// Keeps the last frame, in ascending id order, and compares each new frame
  /// with it. The server's frames list the actors in its registry's order,
  /// which changes only when actors come or go, so the order found for a
  /// frame is reused while its ids stay the same.
  class FrameDiffer {
  public:

    enum class Result {
      /// @a diff holds the changes against the previous frame.
      Diffed,
      /// There was no previous frame: @a diff lists every actor as added.
      NoBase,
      /// The frame repeats an id. Nothing was compared and there is no base
      /// any more.
      DuplicateId
    };

    bool HasBase() const {
      return _has_base;
    }

    /// Forgets the last frame; the next one is compared with nothing.
    void Reset() {
      _has_base = false;
    }

    /// The last frame's records in ascending id order.
    const std::vector<uint8_t> &GetBase() const {
      return _base;
    }

    /// Compares @a count records at @a records (packed, unaligned) with the
    /// last frame, fills @a diff, and keeps them as the last frame.
    Result Next(const uint8_t *records, size_t count, FrameDiff &diff) {
      diff.Clear();
      if (!SortById(records, count)) {
        _has_base = false;
        _base.clear();
        _base_ids.clear();
        return Result::DuplicateId;
      }
      const bool had_base = _has_base;
      if (!had_base) {
        diff.added.assign(_order.begin(), _order.end());
      } else {
        size_t i = 0u;
        size_t j = 0u;
        diff.survivors.reserve(count);
        diff.masks.reserve(count);
        while ((i < _base_ids.size()) || (j < count)) {
          if (j == count || ((i < _base_ids.size()) && (_base_ids[i] < _ids[_order[j]]))) {
            diff.removed.push_back(_base_ids[i]);
            ++i;
          } else if (i == _base_ids.size() || (_ids[_order[j]] < _base_ids[i])) {
            diff.added.push_back(_order[j]);
            ++j;
          } else {
            const uint8_t *before = _base.data() + i * kRecordSize;
            const uint8_t *after = records + static_cast<size_t>(_order[j]) * kRecordSize;
            diff.survivors.push_back(_order[j]);
            diff.masks.push_back(ChangedGroups(before, after));
            ++i;
            ++j;
          }
        }
      }
      // Keep this frame, in id order, as the base of the next one.
      _base.resize(count * kRecordSize);
      _base_ids.resize(count);
      for (size_t k = 0u; k < count; ++k) {
        const uint32_t index = _order[k];
        std::memcpy(
            _base.data() + k * kRecordSize,
            records + static_cast<size_t>(index) * kRecordSize,
            kRecordSize);
        _base_ids[k] = _ids[index];
      }
      _has_base = true;
      return had_base ? Result::Diffed : Result::NoBase;
    }

  private:

    /// Fills _ids with the frame's ids in frame order and _order with the
    /// frame indices in ascending id order. False if an id repeats.
    bool SortById(const uint8_t *records, size_t count) {
      bool same_ids = (_ids.size() == count) && (_order.size() == count);
      _next_ids.resize(count);
      for (size_t k = 0u; k < count; ++k) {
        _next_ids[k] = ReadId(records + k * kRecordSize);
      }
      same_ids = same_ids && std::equal(_next_ids.begin(), _next_ids.end(), _ids.begin());
      if (same_ids) {
        return true;
      }
      _ids.swap(_next_ids);
      _order.resize(count);
      std::iota(_order.begin(), _order.end(), 0u);
      std::sort(_order.begin(), _order.end(), [this](uint32_t lhs, uint32_t rhs) {
        return _ids[lhs] < _ids[rhs];
      });
      for (size_t k = 1u; k < count; ++k) {
        if (_ids[_order[k - 1u]] == _ids[_order[k]]) {
          _ids.clear();
          _order.clear();
          return false;
        }
      }
      return true;
    }

    bool _has_base = false;

    /// Last frame, in ascending id order.
    std::vector<uint8_t> _base;
    std::vector<uint32_t> _base_ids;

    /// Ids of the current frame in frame order, and its indices sorted by id.
    std::vector<uint32_t> _ids;
    std::vector<uint32_t> _order;

    std::vector<uint32_t> _next_ids;
  };

} // namespace episode_codec
} // namespace s11n
} // namespace sensor
} // namespace carla
