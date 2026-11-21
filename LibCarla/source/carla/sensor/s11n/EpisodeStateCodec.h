// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/data/ActorDynamicState.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"

#include <algorithm>
#include <array>
#include <bit>
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

  // ===========================================================================
  // -- Hash -------------------------------------------------------------------
  // ===========================================================================

  static_assert(std::endian::native == std::endian::little,
      "the snapshot wire format and its hash assume a little-endian host");

namespace detail {

  constexpr uint64_t kPrime1 = 11400714785074694791ull;
  constexpr uint64_t kPrime2 = 14029467366897019727ull;
  constexpr uint64_t kPrime3 = 1609587929392839161ull;
  constexpr uint64_t kPrime4 = 9650029242287828579ull;
  constexpr uint64_t kPrime5 = 2870177450012600261ull;

  inline uint64_t Rotl(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
  }

  inline uint64_t Load64(const uint8_t *p) {
    uint64_t v;
    std::memcpy(&v, p, sizeof(v));
    return v;
  }

  inline uint32_t Load32(const uint8_t *p) {
    uint32_t v;
    std::memcpy(&v, p, sizeof(v));
    return v;
  }

  inline uint64_t Round(uint64_t acc, uint64_t input) {
    acc += input * kPrime2;
    acc = Rotl(acc, 31);
    return acc * kPrime1;
  }

  inline uint64_t MergeRound(uint64_t h, uint64_t acc) {
    h ^= Round(0u, acc);
    return h * kPrime1 + kPrime4;
  }

} // namespace detail

  /// XXH64 with seed 0 of the kRecordSize bytes of a record, id included.
  /// Fixed here rather than taken from a library or std::hash so that the
  /// server and every client compute the same value.
  inline uint64_t RecordHash(const uint8_t *record) {
    using namespace detail;
    static_assert(kRecordSize >= 32u, "XXH64 long-input path");
    uint64_t a0 = kPrime1 + kPrime2;
    uint64_t a1 = kPrime2;
    uint64_t a2 = 0u;
    uint64_t a3 = 0u - kPrime1;
    const uint8_t *p = record;
    const uint8_t *const end = record + kRecordSize;
    const uint8_t *const stripes_end = record + (kRecordSize / 32u) * 32u;
    for (; p < stripes_end; p += 32u) {
      a0 = Round(a0, Load64(p));
      a1 = Round(a1, Load64(p + 8u));
      a2 = Round(a2, Load64(p + 16u));
      a3 = Round(a3, Load64(p + 24u));
    }
    uint64_t h = Rotl(a0, 1) + Rotl(a1, 7) + Rotl(a2, 12) + Rotl(a3, 18);
    h = MergeRound(h, a0);
    h = MergeRound(h, a1);
    h = MergeRound(h, a2);
    h = MergeRound(h, a3);
    h += kRecordSize;
    for (; p + 8u <= end; p += 8u) {
      h ^= Round(0u, Load64(p));
      h = Rotl(h, 27) * kPrime1 + kPrime4;
    }
    for (; p + 4u <= end; p += 4u) {
      h ^= static_cast<uint64_t>(Load32(p)) * kPrime1;
      h = Rotl(h, 23) * kPrime2 + kPrime3;
    }
    for (; p < end; ++p) {
      h ^= static_cast<uint64_t>(*p) * kPrime5;
      h = Rotl(h, 11) * kPrime1;
    }
    h ^= h >> 33;
    h *= kPrime2;
    h ^= h >> 29;
    h *= kPrime3;
    h ^= h >> 32;
    return h;
  }

  /// Hash of a whole frame: the sum, modulo 2^64, of the hash of each record,
  /// so it does not depend on the order of the records.
  inline uint64_t StateHash(const uint8_t *records, size_t count) {
    uint64_t sum = 0u;
    for (size_t k = 0u; k < count; ++k) {
      sum += RecordHash(records + k * kRecordSize);
    }
    return sum;
  }

  // ===========================================================================
  // -- Differences between frames ---------------------------------------------
  // ===========================================================================

  using Serializer = EpisodeStateSerializer;

  using Header = Serializer::Header;

  /// Size of the header of a snapshot body.
  constexpr size_t kHeaderSize = sizeof(Header);

  static_assert(kHeaderSize == 76u, "the snapshot header is part of the wire format");

  /// Removed and added counts at the start of a delta's payload.
  constexpr size_t kCountsSize = 2u * sizeof(uint32_t);

  inline Header ReadHeader(const uint8_t *body) {
    Header header;
    std::memcpy(&header, body, sizeof(header));
    return header;
  }

  inline void WriteHeader(uint8_t *body, const Header &header) {
    std::memcpy(body, &header, sizeof(header));
  }

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
      size_t size = kHeaderSize + kCountsSize;
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
  /// frame is reused while its ids stay the same. It also keeps the hash of
  /// every record, so that only the records that changed are hashed again.
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

    /// StateHash of the last frame. Valid after Next returned Diffed or
    /// NoBase.
    uint64_t GetHash() const {
      return _hash;
    }

    /// Compares @a count records at @a records (packed, unaligned) with the
    /// last frame, fills @a diff, and keeps them as the last frame.
    Result Next(const uint8_t *records, size_t count, FrameDiff &diff) {
      diff.Clear();
      if (!SortById(records, count)) {
        _has_base = false;
        _base.clear();
        _base_ids.clear();
        _base_hashes.clear();
        return Result::DuplicateId;
      }
      const bool had_base = _has_base;
      _frame_hashes.resize(count);
      if (!had_base) {
        diff.added.assign(_order.begin(), _order.end());
        for (size_t k = 0u; k < count; ++k) {
          _frame_hashes[k] = RecordHash(records + k * kRecordSize);
        }
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
            const uint32_t index = _order[j];
            diff.added.push_back(index);
            _frame_hashes[index] = RecordHash(records + static_cast<size_t>(index) * kRecordSize);
            ++j;
          } else {
            const uint32_t index = _order[j];
            const uint8_t *before = _base.data() + i * kRecordSize;
            const uint8_t *after = records + static_cast<size_t>(index) * kRecordSize;
            const uint8_t mask = ChangedGroups(before, after);
            diff.survivors.push_back(index);
            diff.masks.push_back(mask);
            // Same id and same groups: the very same bytes.
            _frame_hashes[index] = (mask == 0u) ? _base_hashes[i] : RecordHash(after);
            ++i;
            ++j;
          }
        }
      }
      // Keep this frame, in id order, as the base of the next one.
      _base.resize(count * kRecordSize);
      _base_ids.resize(count);
      _base_hashes.resize(count);
      _hash = 0u;
      for (size_t k = 0u; k < count; ++k) {
        const uint32_t index = _order[k];
        std::memcpy(
            _base.data() + k * kRecordSize,
            records + static_cast<size_t>(index) * kRecordSize,
            kRecordSize);
        _base_ids[k] = _ids[index];
        _base_hashes[k] = _frame_hashes[index];
        _hash += _frame_hashes[index];
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

    /// Last frame, in ascending id order, the RecordHash of each record and
    /// their sum.
    std::vector<uint8_t> _base;
    std::vector<uint32_t> _base_ids;
    std::vector<uint64_t> _base_hashes;
    uint64_t _hash = 0u;

    /// RecordHash of each record of the current frame, in frame order.
    std::vector<uint64_t> _frame_hashes;

    /// Ids of the current frame in frame order, and its indices sorted by id.
    std::vector<uint32_t> _ids;
    std::vector<uint32_t> _order;

    std::vector<uint32_t> _next_ids;
  };

  // ===========================================================================
  // -- DeltaEncoder -----------------------------------------------------------
  // ===========================================================================

  /// Server side. Completes the header of each snapshot body and, when the
  /// previous body is the one numbered just before it, offers a delta
  /// against it.
  ///
  /// A delta's payload, after the header:
  ///   1. u32 removed count, u32 added count;
  ///   2. the removed ids, ascending;
  ///   3. the added records, whole, by ascending id;
  ///   4. a bitmap with one bit per survivor (an actor of the previous body
  ///      that is still there), by ascending id, least significant bit first,
  ///      set when the survivor changed;
  ///   5. for each set bit, a group mask (bit g for kGroups[g]) and the bytes
  ///      of every group in it.
  class DeltaEncoder {
  public:

    /// @a body is a keyframe: a header with the episode fields filled in
    /// (episode id, timestamps, map origin, simulation state), then whole
    /// records in any order. Fills the rest of the header as body number
    /// @a sequence, compares the records with the previous body, and
    /// returns whether a delta is available (see HasDelta).
    bool Prepare(uint8_t *body, size_t size, uint64_t sequence) {
      _has_delta = false;
      if (size < kHeaderSize || ((size - kHeaderSize) % kRecordSize) != 0u) {
        Reset();
        return false;
      }
      const size_t count = (size - kHeaderSize) / kRecordSize;
      const uint8_t *records = body + kHeaderSize;
      Header header = ReadHeader(body);
      header.magic = Serializer::kMagic;
      header.version = Serializer::kVersion;
      header.kind = Serializer::Kind::Keyframe;
      header.flags = 0u;
      header.actor_count = static_cast<uint32_t>(count);
      header.payload_size = static_cast<uint32_t>(size - kHeaderSize);
      header.sequence = sequence;
      header.base_sequence = 0u;
      _last_result = _differ.Next(records, count, _diff);
      header.state_hash = (_last_result == FrameDiffer::Result::DuplicateId) ?
          StateHash(records, count) :
          _differ.GetHash();
      _has_delta =
          (_last_result == FrameDiffer::Result::Diffed) &&
          _has_body &&
          (_body_sequence + 1u == sequence) &&
          (_episode_id == header.episode_id);
      if (_has_delta) {
        _delta_size = _diff.DeltaSize();
        _has_delta = (_delta_size < size);
      }
      WriteHeader(body, header);
      _header = header;
      _has_body = (_last_result != FrameDiffer::Result::DuplicateId);
      _body_sequence = sequence;
      _episode_id = header.episode_id;
      return _has_delta;
    }

    /// Whether the last Prepare found a delta against the body numbered just
    /// before it, of the same episode, smaller than the keyframe.
    bool HasDelta() const {
      return _has_delta;
    }

    /// Size of the delta body. Valid while HasDelta().
    size_t GetDeltaSize() const {
      return _delta_size;
    }

    /// What the last Prepare compared: valid when GetLastResult() is
    /// Diffed. Frame indices refer to the order of the records in the body.
    const FrameDiff &GetDiff() const {
      return _diff;
    }

    FrameDiffer::Result GetLastResult() const {
      return _last_result;
    }

    /// Writes the delta of the last Prepare into GetDeltaSize() bytes at
    /// @a out. @a body is the body given to Prepare, unchanged since.
    /// Returns the bytes written.
    size_t WriteDelta(const uint8_t *body, uint8_t *out) const {
      const uint8_t *records = body + kHeaderSize;
      Header header = _header;
      header.kind = Serializer::Kind::Delta;
      header.base_sequence = _header.sequence - 1u;
      header.payload_size = static_cast<uint32_t>(_delta_size - kHeaderSize);
      uint8_t *p = out;
      WriteHeader(p, header);
      p += kHeaderSize;
      const uint32_t removed = static_cast<uint32_t>(_diff.removed.size());
      const uint32_t added = static_cast<uint32_t>(_diff.added.size());
      std::memcpy(p, &removed, sizeof(removed));
      p += sizeof(removed);
      std::memcpy(p, &added, sizeof(added));
      p += sizeof(added);
      if (removed > 0u) {
        std::memcpy(p, _diff.removed.data(), removed * sizeof(uint32_t));
        p += removed * sizeof(uint32_t);
      }
      for (const uint32_t index : _diff.added) {
        std::memcpy(p, records + static_cast<size_t>(index) * kRecordSize, kRecordSize);
        p += kRecordSize;
      }
      const size_t survivors = _diff.survivors.size();
      uint8_t *bitmap = p;
      const size_t bitmap_size = (survivors + 7u) / 8u;
      std::fill(bitmap, bitmap + bitmap_size, uint8_t(0u));
      p += bitmap_size;
      for (size_t k = 0u; k < survivors; ++k) {
        const uint8_t mask = _diff.masks[k];
        if (mask == 0u) {
          continue;
        }
        bitmap[k / 8u] = static_cast<uint8_t>(bitmap[k / 8u] | (1u << (k % 8u)));
        *p++ = mask;
        const uint8_t *record = records + static_cast<size_t>(_diff.survivors[k]) * kRecordSize;
        for (size_t g = 0u; g < kGroupCount; ++g) {
          if ((mask & (1u << g)) != 0u) {
            const auto &group = kGroups[g];
            std::memcpy(p, record + group.begin, group.end - group.begin);
            p += group.end - group.begin;
          }
        }
      }
      return static_cast<size_t>(p - out);
    }

    /// Forgets the previous body: the next Prepare offers no delta. The
    /// sequence numbers are the caller's and do not restart.
    void Reset() {
      _differ.Reset();
      _has_body = false;
      _has_delta = false;
    }

  private:

    FrameDiffer _differ;

    FrameDiff _diff;

    FrameDiffer::Result _last_result = FrameDiffer::Result::NoBase;

    Header _header;

    bool _has_body = false;

    bool _has_delta = false;

    size_t _delta_size = 0u;

    uint64_t _body_sequence = 0u;

    uint64_t _episode_id = 0u;
  };

  // ===========================================================================
  // -- DeltaDecoder -----------------------------------------------------------
  // ===========================================================================

  enum class DecodeStatus {
    /// A keyframe: publish the message as it came.
    Keyframe,
    /// A delta: publish the keyframe written where the allocator said.
    Delta,
    /// The message does not apply to the base, is malformed, or the result
    /// does not match its hash. Nothing is published and there is no base
    /// any more: only a keyframe recovers.
    NeedResync,
    /// Not a snapshot body of this wire format (a server of another build).
    /// Nothing changed.
    Incompatible
  };

  struct DecodeResult {
    DecodeStatus status;
    /// Why, for NeedResync and Incompatible.
    const char *error = nullptr;
  };

  /// Client side. Keeps the last body as whole records in ascending id
  /// order and turns each message back into a keyframe. A message is fully
  /// validated before the base changes.
  class DeltaDecoder {
  public:

    bool HasBase() const {
      return _has_base;
    }

    /// Sequence of the body the base holds.
    uint64_t GetSequence() const {
      return _sequence;
    }

    /// The base's records, ascending by id.
    const std::vector<uint8_t> &GetBase() const {
      return _base;
    }

    void Reset() {
      _has_base = false;
      _base.clear();
    }

    /// Decodes the snapshot body of @a size bytes at @a body. For a delta,
    /// @a allocate(n) must return n writable bytes; the decoder writes there
    /// the keyframe body to publish (header, then the records ascending by
    /// id) and checks the header's state_hash against those very bytes.
    template <typename AllocateT>
    DecodeResult Decode(const uint8_t *body, size_t size, AllocateT &&allocate) {
      if (size < 8u) {
        return {DecodeStatus::Incompatible, "message shorter than the snapshot magic"};
      }
      uint32_t magic;
      uint16_t version;
      std::memcpy(&magic, body, sizeof(magic));
      std::memcpy(&version, body + sizeof(magic), sizeof(version));
      if (magic != Serializer::kMagic) {
        return {DecodeStatus::Incompatible, "not a snapshot of this wire format (bad magic)"};
      }
      if (version != Serializer::kVersion) {
        return {DecodeStatus::Incompatible, "snapshot wire format version differs"};
      }
      if (size < kHeaderSize) {
        return Resync("snapshot shorter than its header");
      }
      const Header header = ReadHeader(body);
      if (header.flags != 0u) {
        return Resync("snapshot with unknown flags");
      }
      if (static_cast<uint64_t>(header.payload_size) != size - kHeaderSize) {
        return Resync("snapshot payload size does not match the message");
      }
      if (header.kind == Serializer::Kind::Keyframe) {
        return DecodeKeyframe(header, body + kHeaderSize);
      }
      if (header.kind == Serializer::Kind::Delta) {
        return DecodeDelta(header, body + kHeaderSize, std::forward<AllocateT>(allocate));
      }
      return Resync("snapshot of unknown kind");
    }

  private:

    DecodeResult Resync(const char *error) {
      Reset();
      return {DecodeStatus::NeedResync, error};
    }

    DecodeResult DecodeKeyframe(const Header &header, const uint8_t *records) {
      const size_t count = header.actor_count;
      if (static_cast<uint64_t>(count) * kRecordSize != header.payload_size) {
        return Resync("keyframe size does not match its actor count");
      }
      if (StateHash(records, count) != header.state_hash) {
        return Resync("keyframe does not match its hash");
      }
      // Keep it ascending by id.
      _order.resize(count);
      std::iota(_order.begin(), _order.end(), 0u);
      std::sort(_order.begin(), _order.end(), [records](uint32_t lhs, uint32_t rhs) {
        return ReadId(records + static_cast<size_t>(lhs) * kRecordSize) <
               ReadId(records + static_cast<size_t>(rhs) * kRecordSize);
      });
      _base.resize(count * kRecordSize);
      for (size_t k = 0u; k < count; ++k) {
        std::memcpy(
            _base.data() + k * kRecordSize,
            records + static_cast<size_t>(_order[k]) * kRecordSize,
            kRecordSize);
      }
      _has_base = true;
      for (size_t k = 1u; k < count; ++k) {
        if (ReadId(&_base[(k - 1u) * kRecordSize]) == ReadId(&_base[k * kRecordSize])) {
          // Publishable, as before deltas existed, but nothing can apply to
          // it: the server never sends deltas against such a body.
          _has_base = false;
          break;
        }
      }
      _sequence = header.sequence;
      _episode_id = header.episode_id;
      return {DecodeStatus::Keyframe};
    }

    template <typename AllocateT>
    DecodeResult DecodeDelta(const Header &header, const uint8_t *payload, AllocateT &&allocate) {
      if (!_has_base) {
        return Resync("delta without a keyframe to apply to");
      }
      if (header.base_sequence != _sequence || header.sequence <= header.base_sequence) {
        return Resync("delta does not apply to the last snapshot received");
      }
      if (header.episode_id != _episode_id) {
        return Resync("delta of another episode");
      }
      // -- Validate, without touching the base. ------------------------------
      const uint64_t size = header.payload_size;
      if (size < kCountsSize) {
        return Resync("delta shorter than its counts");
      }
      uint32_t removed_count;
      uint32_t added_count;
      std::memcpy(&removed_count, payload, sizeof(removed_count));
      std::memcpy(&added_count, payload + sizeof(removed_count), sizeof(added_count));
      const uint64_t base_count = _base.size() / kRecordSize;
      if (removed_count > base_count) {
        return Resync("delta removes more actors than there are");
      }
      const uint64_t survivors = base_count - removed_count;
      if (survivors + added_count != header.actor_count) {
        return Resync("delta counts do not match its actor count");
      }
      uint64_t offset = kCountsSize;
      const uint64_t removed_bytes = uint64_t(removed_count) * sizeof(uint32_t);
      const uint64_t added_bytes = uint64_t(added_count) * kRecordSize;
      const uint64_t bitmap_bytes = (survivors + 7u) / 8u;
      if (removed_bytes > size - offset) {
        return Resync("delta truncated in its removed ids");
      }
      const uint8_t *removed_ids = payload + offset;
      offset += removed_bytes;
      if (added_bytes > size - offset) {
        return Resync("delta truncated in its added actors");
      }
      const uint8_t *added = payload + offset;
      offset += added_bytes;
      if (bitmap_bytes > size - offset) {
        return Resync("delta truncated in its bitmap");
      }
      const uint8_t *bitmap = payload + offset;
      offset += bitmap_bytes;
      if ((survivors % 8u) != 0u &&
          (bitmap[bitmap_bytes - 1u] >> (survivors % 8u)) != 0u) {
        return Resync("delta bitmap marks actors past its last survivor");
      }
      // Removed ids: ascending and in the base. Added ids: ascending and not
      // in the base.
      {
        size_t i = 0u;
        for (uint32_t r = 0u; r < removed_count; ++r) {
          const uint32_t id = ReadId(removed_ids + r * sizeof(uint32_t));
          if (r > 0u && id <= ReadId(removed_ids + (r - 1u) * sizeof(uint32_t))) {
            return Resync("delta removed ids not ascending");
          }
          while (i < base_count && ReadId(&_base[i * kRecordSize]) < id) {
            ++i;
          }
          if (i == base_count || ReadId(&_base[i * kRecordSize]) != id) {
            return Resync("delta removes an actor that is not there");
          }
        }
        i = 0u;
        for (uint32_t a = 0u; a < added_count; ++a) {
          const uint32_t id = ReadId(added + uint64_t(a) * kRecordSize);
          if (a > 0u && id <= ReadId(added + uint64_t(a - 1u) * kRecordSize)) {
            return Resync("delta added ids not ascending");
          }
          while (i < base_count && ReadId(&_base[i * kRecordSize]) < id) {
            ++i;
          }
          if (i < base_count && ReadId(&_base[i * kRecordSize]) == id) {
            return Resync("delta adds an actor that is already there");
          }
        }
      }
      // Changed survivors.
      const uint8_t *changes = payload + offset;
      for (uint64_t s = 0u; s < survivors; ++s) {
        if ((bitmap[s / 8u] & (1u << (s % 8u))) == 0u) {
          continue;
        }
        if (offset >= size) {
          return Resync("delta truncated in its changes");
        }
        const uint8_t mask = payload[offset];
        if (mask == 0u || (mask & ~kAllGroups) != 0u) {
          return Resync("delta with an invalid group mask");
        }
        offset += 1u;
        const uint64_t bytes = GroupBytes(mask);
        if (bytes > size - offset) {
          return Resync("delta truncated in its changes");
        }
        offset += bytes;
      }
      if (offset != size) {
        return Resync("delta with bytes left over");
      }
      // -- Apply, into the keyframe to publish. ------------------------------
      const size_t count = header.actor_count;
      uint8_t *out = allocate(kHeaderSize + count * kRecordSize);
      Header out_header = header;
      out_header.kind = Serializer::Kind::Keyframe;
      out_header.base_sequence = 0u;
      out_header.payload_size = static_cast<uint32_t>(count * kRecordSize);
      WriteHeader(out, out_header);
      uint8_t *records = out + kHeaderSize;
      uint8_t *p = records;
      size_t i = 0u;          // base record
      uint32_t r = 0u;        // removed id
      uint32_t a = 0u;        // added record
      uint64_t s = 0u;        // survivor
      while (i < base_count || a < added_count) {
        // Skip the removed ones.
        if (i < base_count && r < removed_count &&
            ReadId(&_base[i * kRecordSize]) == ReadId(removed_ids + r * sizeof(uint32_t))) {
          ++i;
          ++r;
          continue;
        }
        const bool take_added = (i == base_count) ||
            (a < added_count &&
             ReadId(added + uint64_t(a) * kRecordSize) < ReadId(&_base[i * kRecordSize]));
        if (take_added) {
          std::memcpy(p, added + uint64_t(a) * kRecordSize, kRecordSize);
          ++a;
        } else {
          std::memcpy(p, &_base[i * kRecordSize], kRecordSize);
          if ((bitmap[s / 8u] & (1u << (s % 8u))) != 0u) {
            const uint8_t mask = *changes++;
            for (size_t g = 0u; g < kGroupCount; ++g) {
              if ((mask & (1u << g)) != 0u) {
                const auto &group = kGroups[g];
                std::memcpy(p + group.begin, changes, group.end - group.begin);
                changes += group.end - group.begin;
              }
            }
          }
          ++i;
          ++s;
        }
        p += kRecordSize;
      }
      if (StateHash(records, count) != header.state_hash) {
        return Resync("snapshot rebuilt from a delta does not match its hash");
      }
      _base.assign(records, records + count * kRecordSize);
      _sequence = header.sequence;
      return {DecodeStatus::Delta};
    }

    bool _has_base = false;

    uint64_t _sequence = 0u;

    uint64_t _episode_id = 0u;

    /// Records of the last body, ascending by id.
    std::vector<uint8_t> _base;

    std::vector<uint32_t> _order;
  };

} // namespace episode_codec
} // namespace s11n
} // namespace sensor
} // namespace carla
