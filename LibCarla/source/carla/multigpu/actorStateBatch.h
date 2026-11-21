// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/multigpu/frameDataCodec.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

/// @file
/// Frame data packet 35, MultiGpuActorState: the state of every mirrored
/// actor in one frame, as columns per section with deltas against the previous
/// frame.
///
/// The state is a table of slots, one per actor (its primary id and its kind),
/// and sections. A section holds one record per slot, or none: the pose, the
/// kinematics, the controls, and so on. A record is a byte string, compared
/// bit for bit, so every value (NaN payloads, signed zeros) goes through
/// unchanged.
///
/// A keyframe carries the table and every record. A delta carries only the
/// records that differ, bit for bit, from the previous frame, and is valid
/// only on top of exactly that frame: the decoder rebuilds the whole state and
/// hands it out complete. It never hands out a delta.
///
/// Wire layout of the payload, host byte order like the rest of the frame data:
///
///     u8  layout              kActorStateLayout
///     u8  flags               kKeyframe
///     u16 section_count
///     u32 slot_count
///     u64 frame
///     u64 base_frame          the frame the delta applies to; frame on a keyframe
///     u64 table_hash          of ids, kinds and the section kinds and strides
///     keyframe only:
///       u32 ids[slot_count]
///       u8  kinds[slot_count]
///     per section, in ascending kind:
///       u8  kind
///       u8  section_flags     kSamePresence | kNoneChanged (deltas only)
///       u16 stride            record size; 0 for records of variable size
///       u32 body_size
///       body:
///         presence bitmap     unless kSamePresence; bit i = slot i, LSB first
///         changed bitmap      deltas only, unless kNoneChanged; a subset of presence
///         records             of every present slot (keyframe) or every
///                             changed one (delta), in slot order; a variable
///                             one is u32 size + bytes

namespace carla {
namespace multigpu {

  constexpr uint8_t kActorStatePacketId = 35u;

  /// Version of the packet 35 layout. A decoder rejects any other.
  constexpr uint8_t kActorStateLayout = 1u;

  /// The sections of packet 35. A decoder keeps sections it does not know
  /// (their records are bytes to it), so new ones can be added.
  enum class ActorStateSection : uint8_t {
    /// PoseRecord: the world pose, every actor.
    Pose = 1u,
    /// KinematicsRecord: velocities of actors that simulate physics.
    Kinematics = 2u,
    /// VehicleControlRecord: a vehicle's control input.
    VehicleControl = 3u,
    /// VehicleLightRecord: a vehicle's light state.
    VehicleLight = 4u,
    /// WheelRecord[]: every wheel, by wheel index (variable size).
    Wheels = 5u,
    /// DrivetrainRecord: the gear the primary's simulation is in.
    Drivetrain = 6u,
    /// DoorRecord[]: the open doors (variable size).
    Doors = 7u,
    /// WalkerRecord: a walker's animation speed.
    Walker = 8u,
    /// BikerRecord: a two-wheeler's animation.
    Biker = 9u
  };

#pragma pack(push, 1)

  /// UE units; rotation as a quaternion x, y, z, w.
  struct PoseRecord {
    double location[3];
    double rotation[4];
  };

  struct KinematicsRecord {
    double linear_velocity[3];
    double angular_velocity[3];
  };

  struct VehicleControlRecord {
    float steering;
    float throttle;
    float brake;
    uint8_t handbrake;
    int32_t gear;
  };

  struct VehicleLightRecord {
    uint32_t state;
  };

  /// Degrees and cm.
  struct WheelRecord {
    float steer;
    float spin;
    float suspension_offset;
  };

  struct DrivetrainRecord {
    int32_t current_gear;
    /// 1: manual gear shift, 2: the body simulates physics.
    uint8_t flags;
  };

  /// Pose relative to the vehicle, in UE units; rotation x, y, z, w.
  struct DoorRecord {
    uint8_t door;
    double location[3];
    double rotation[4];
  };

  struct WalkerRecord {
    float speed;
  };

  struct BikerRecord {
    float forward_speed;
    float engine_rotation;
  };

#pragma pack(pop)

  static_assert(sizeof(PoseRecord) == 56u, "packet 35 layout");
  static_assert(sizeof(KinematicsRecord) == 48u, "packet 35 layout");
  static_assert(sizeof(VehicleControlRecord) == 17u, "packet 35 layout");
  static_assert(sizeof(VehicleLightRecord) == 4u, "packet 35 layout");
  static_assert(sizeof(WheelRecord) == 12u, "packet 35 layout");
  static_assert(sizeof(DrivetrainRecord) == 5u, "packet 35 layout");
  static_assert(sizeof(DoorRecord) == 57u, "packet 35 layout");
  static_assert(sizeof(WalkerRecord) == 4u, "packet 35 layout");
  static_assert(sizeof(BikerRecord) == 8u, "packet 35 layout");

  /// The fixed record size of a known section, 0 for a variable one.
  constexpr uint16_t ActorStateStride(ActorStateSection kind) {
    switch (kind) {
      case ActorStateSection::Pose: return sizeof(PoseRecord);
      case ActorStateSection::Kinematics: return sizeof(KinematicsRecord);
      case ActorStateSection::VehicleControl: return sizeof(VehicleControlRecord);
      case ActorStateSection::VehicleLight: return sizeof(VehicleLightRecord);
      case ActorStateSection::Wheels: return 0u;
      case ActorStateSection::Drivetrain: return sizeof(DrivetrainRecord);
      case ActorStateSection::Doors: return 0u;
      case ActorStateSection::Walker: return sizeof(WalkerRecord);
      case ActorStateSection::Biker: return sizeof(BikerRecord);
    }
    return 0u;
  }

  /// One section of the state: a record, or none, per slot.
  class ActorStateColumn {
  public:

    ActorStateColumn(uint8_t kind, uint16_t stride) : _kind(kind), _stride(stride) {}

    uint8_t Kind() const {
      return _kind;
    }

    /// The record size, 0 for records of variable size.
    uint16_t Stride() const {
      return _stride;
    }

    bool IsVariable() const {
      return _stride == 0u;
    }

    size_t SlotCount() const {
      return _present.size();
    }

    /// Sets the number of slots and removes every record.
    void Reset(size_t slots) {
      _present.assign(slots, 0u);
      if (IsVariable()) {
        _offsets.assign(slots, 0u);
        _sizes.assign(slots, 0u);
        _heap.clear();
        _live = 0u;
      } else {
        _fixed.assign(slots * _stride, 0u);
      }
    }

    bool IsPresent(size_t slot) const {
      return _present[slot] != 0u;
    }

    void Clear(size_t slot) {
      _present[slot] = 0u;
      if (IsVariable()) {
        _live -= _sizes[slot];
        _sizes[slot] = 0u;
      } else {
        std::memset(_fixed.data() + slot * _stride, 0, _stride);
      }
    }

    /// Sets the record of @a slot to @a size bytes, which must not be inside
    /// this column. A fixed section takes exactly its stride.
    bool SetBytes(size_t slot, const void *data, size_t size) {
      if (IsVariable()) {
        SetVariable(slot, static_cast<const uint8_t *>(data), size);
      } else {
        if (size != _stride) {
          return false;
        }
        std::memcpy(_fixed.data() + slot * _stride, data, size);
      }
      _present[slot] = 1u;
      return true;
    }

    template <typename T>
    bool Set(size_t slot, const T &record) {
      static_assert(std::is_trivially_copyable_v<T>, "records are plain values");
      return SetBytes(slot, &record, sizeof(T));
    }

    /// A variable record holding @a count items.
    template <typename T>
    bool SetArray(size_t slot, const T *items, size_t count) {
      static_assert(std::is_trivially_copyable_v<T>, "records are plain values");
      return IsVariable() && SetBytes(slot, items, count * sizeof(T));
    }

    /// The bytes of the record of @a slot (none if absent).
    const uint8_t *Data(size_t slot) const {
      return IsVariable() ? _heap.data() + _offsets[slot] : _fixed.data() + slot * _stride;
    }

    size_t Size(size_t slot) const {
      if (!IsPresent(slot)) {
        return 0u;
      }
      return IsVariable() ? _sizes[slot] : _stride;
    }

    template <typename T>
    std::optional<T> Get(size_t slot) const {
      static_assert(std::is_trivially_copyable_v<T>, "records are plain values");
      if (!IsPresent(slot) || (Size(slot) != sizeof(T))) {
        return std::nullopt;
      }
      T record;
      std::memcpy(&record, Data(slot), sizeof(T));
      return record;
    }

    template <typename T>
    std::vector<T> GetArray(size_t slot) const {
      static_assert(std::is_trivially_copyable_v<T>, "records are plain values");
      std::vector<T> items;
      if (IsPresent(slot) && IsVariable() && ((Size(slot) % sizeof(T)) == 0u)) {
        items.resize(Size(slot) / sizeof(T));
        if (!items.empty()) {
          std::memcpy(items.data(), Data(slot), Size(slot));
        }
      }
      return items;
    }

    /// Whether the record of @a slot is bit for bit the one of @a other.
    bool SameRecord(size_t slot, const ActorStateColumn &other) const {
      const size_t size = Size(slot);
      return (IsPresent(slot) == other.IsPresent(slot)) &&
          (size == other.Size(slot)) &&
          ((size == 0u) || (std::memcmp(Data(slot), other.Data(slot), size) == 0));
    }

    bool SamePresence(const ActorStateColumn &other) const {
      return _present == other._present;
    }

  private:

    uint8_t _kind;

    uint16_t _stride;

    std::vector<uint8_t> _present;

    std::vector<uint8_t> _fixed;

    // Records of variable size live in one buffer, so that copying a column
    // is a few copies of memory and not one per slot. A record that changes
    // size is appended; the buffer is compacted when it is mostly garbage.
    void SetVariable(size_t slot, const uint8_t *bytes, size_t size) {
      const size_t old_size = IsPresent(slot) ? _sizes[slot] : 0u;
      if (IsPresent(slot) && (old_size == size)) {
        if (size > 0u) {
          std::memcpy(_heap.data() + _offsets[slot], bytes, size);
        }
        return;
      }
      _live -= old_size;
      _sizes[slot] = 0u;
      _present[slot] = 0u;
      if (_heap.size() + size > 2u * (_live + size) + 4096u) {
        Compact();
      }
      _offsets[slot] = static_cast<uint32_t>(_heap.size());
      _sizes[slot] = static_cast<uint32_t>(size);
      _heap.insert(_heap.end(), bytes, bytes + size);
      _live += size;
    }

    void Compact() {
      std::vector<uint8_t> heap;
      heap.reserve(2u * _live + 4096u);
      for (size_t i = 0u; i < _present.size(); ++i) {
        if (IsPresent(i)) {
          const size_t offset = heap.size();
          heap.insert(heap.end(), _heap.data() + _offsets[i], _heap.data() + _offsets[i] + _sizes[i]);
          _offsets[i] = static_cast<uint32_t>(offset);
        }
      }
      _heap.swap(heap);
    }

    std::vector<uint32_t> _offsets;

    std::vector<uint32_t> _sizes;

    std::vector<uint8_t> _heap;

    size_t _live = 0u;
  };

  /// The state of every mirrored actor in one frame.
  struct ActorStateFrame {

    uint64_t frame = 0u;

    /// Primary actor id of each slot.
    std::vector<uint32_t> ids;

    /// The actor kind of each slot, as the writer defines it.
    std::vector<uint8_t> kinds;

    /// By kind, so in ascending kind. A section stays where it is when others
    /// are added.
    std::map<uint8_t, ActorStateColumn> sections;

    size_t SlotCount() const {
      return ids.size();
    }

    /// Copies @a other, reusing this frame's memory when both have the same
    /// sections (a copy assignment of the map would allocate every column).
    void CopyFrom(const ActorStateFrame &other) {
      const bool same_sections = (sections.size() == other.sections.size()) &&
          std::equal(sections.begin(), sections.end(), other.sections.begin(),
              [](const auto &a, const auto &b) {
                return (a.first == b.first) && (a.second.Stride() == b.second.Stride());
              });
      if (!same_sections) {
        *this = other;
        return;
      }
      frame = other.frame;
      ids = other.ids;
      kinds = other.kinds;
      auto it = other.sections.begin();
      for (auto &item : sections) {
        item.second = (it++)->second;
      }
    }

    /// Sets the slots and removes every record (the sections stay).
    void ResetSlots(std::vector<uint32_t> slot_ids, std::vector<uint8_t> slot_kinds) {
      ids = std::move(slot_ids);
      kinds = std::move(slot_kinds);
      kinds.resize(ids.size(), 0u);
      for (auto &item : sections) {
        item.second.Reset(ids.size());
      }
    }

    /// The section of @a kind, added (empty) if missing.
    ActorStateColumn &Section(uint8_t kind, uint16_t stride) {
      const auto [it, added] = sections.try_emplace(kind, kind, stride);
      if (added) {
        it->second.Reset(ids.size());
      }
      return it->second;
    }

    ActorStateColumn &Section(ActorStateSection kind) {
      return Section(static_cast<uint8_t>(kind), ActorStateStride(kind));
    }

    const ActorStateColumn *FindSection(uint8_t kind) const {
      const auto it = sections.find(kind);
      return it == sections.end() ? nullptr : &it->second;
    }

    const ActorStateColumn *FindSection(ActorStateSection kind) const {
      return FindSection(static_cast<uint8_t>(kind));
    }

    /// Identifies the table a delta is built on: the slots and the sections'
    /// kinds and strides (FNV-1a, 64 bits).
    uint64_t TableHash() const {
      uint64_t hash = 14695981039346656037ull;
      auto add = [&hash](const void *data, size_t size) {
        const auto *bytes = static_cast<const uint8_t *>(data);
        for (size_t i = 0u; i < size; ++i) {
          hash ^= bytes[i];
          hash *= 1099511628211ull;
        }
      };
      const uint32_t slots = static_cast<uint32_t>(ids.size());
      add(&slots, sizeof(slots));
      if (!ids.empty()) {
        add(ids.data(), ids.size() * sizeof(uint32_t));
        add(kinds.data(), kinds.size());
      }
      for (const auto &item : sections) {
        const uint8_t kind = item.second.Kind();
        const uint16_t stride = item.second.Stride();
        add(&kind, sizeof(kind));
        add(&stride, sizeof(stride));
      }
      return hash;
    }
  };

namespace detail {

  constexpr uint8_t kActorStateKeyframe = 1u;

  constexpr uint8_t kActorStateSamePresence = 1u;

  constexpr uint8_t kActorStateNoneChanged = 2u;

  constexpr size_t kActorStateHeaderSize = 1u + 1u + 2u + 4u + 8u + 8u + 8u;

  constexpr size_t kActorStateSectionHeaderSize = 1u + 1u + 2u + 4u;

  inline size_t BitmapSize(size_t slots) {
    return (slots + 7u) / 8u;
  }

  /// Bounds-checked reader over a payload in memory.
  class ByteCursor {
  public:

    ByteCursor(const uint8_t *data, size_t size) : _data(data), _size(size) {}

    template <typename T>
    bool Read(T &value) {
      return ReadBytes(&value, sizeof(T));
    }

    bool ReadBytes(void *out, size_t size) {
      if (size > Remaining()) {
        return false;
      }
      if (size > 0u) {
        std::memcpy(out, _data + _offset, size);
      }
      _offset += size;
      return true;
    }

    /// Takes the next @a size bytes without copying them.
    const uint8_t *Take(size_t size) {
      if (size > Remaining()) {
        return nullptr;
      }
      const uint8_t *at = _data + _offset;
      _offset += size;
      return at;
    }

    size_t Remaining() const {
      return _size - _offset;
    }

  private:

    const uint8_t *_data;

    size_t _size;

    size_t _offset = 0u;
  };

  /// A bitmap of @a slots bits read from @a data; false if a bit past the
  /// last slot is set.
  inline bool ReadBitmap(const uint8_t *data, size_t slots, std::vector<uint8_t> &out) {
    out.resize(slots);
    for (size_t i = 0u; i < slots; ++i) {
      out[i] = (data[i / 8u] >> (i % 8u)) & 1u;
    }
    const size_t tail = slots % 8u;
    return (tail == 0u) || ((data[slots / 8u] >> tail) == 0u);
  }

  /// Writes into memory sized beforehand.
  class SpanWriter {
  public:

    explicit SpanWriter(uint8_t *at) : _at(at) {}

    template <typename T>
    void Put(const T &value) {
      static_assert(std::is_trivially_copyable_v<T>, "only plain values go on the wire");
      std::memcpy(_at, &value, sizeof(T));
      _at += sizeof(T);
    }

    void PutBytes(const void *data, size_t size) {
      if (size > 0u) {
        std::memcpy(_at, data, size);
        _at += size;
      }
    }

  private:

    uint8_t *_at;
  };

  template <typename GetBit>
  void WriteBitmap(SpanWriter &out, size_t slots, GetBit &&bit) {
    uint8_t byte = 0u;
    for (size_t i = 0u; i < slots; ++i) {
      if (bit(i)) {
        byte |= static_cast<uint8_t>(1u << (i % 8u));
      }
      if ((i % 8u) == 7u) {
        out.Put(byte);
        byte = 0u;
      }
    }
    if ((slots % 8u) != 0u) {
      out.Put(byte);
    }
  }

} // namespace detail

  /// What one Encode call wrote.
  struct ActorStateEncodeInfo {
    bool keyframe = false;
    /// Bytes written, the frame data header included.
    size_t bytes = 0u;
    /// Records written.
    size_t records = 0u;
  };

  /// Writes packet 35 for each frame, as a delta against the previous frame
  /// when it can. Every session gets the same bytes in the same order, so one
  /// encoder serves them all.
  class ActorStateEncoder {
  public:

    /// A keyframe at least every @a keyframe_interval frames (0: only when
    /// needed).
    explicit ActorStateEncoder(uint32_t keyframe_interval = 600u)
      : _keyframe_interval(keyframe_interval) {}

    void SetKeyframeInterval(uint32_t frames) {
      _keyframe_interval = frames;
    }

    /// Forgets the previous frame, so the next one is a keyframe.
    void Reset() {
      _has_baseline = false;
    }

    /// Writes the whole packet (header included) for @a state. A keyframe if
    /// @a force_keyframe, if there is no previous frame, if @a state is not
    /// the frame right after it, if the table changed, or if the interval is
    /// due.
    ActorStateEncodeInfo Encode(
        const ActorStateFrame &state,
        std::ostream &out,
        bool force_keyframe = false) {
      const uint64_t table_hash = state.TableHash();
      const bool keyframe = force_keyframe ||
          !_has_baseline ||
          (state.frame != _baseline.frame + 1u) ||
          (table_hash != _baseline_hash) ||
          ((_keyframe_interval != 0u) && (_since_keyframe + 1u >= _keyframe_interval));

      const size_t slots = state.SlotCount();
      const size_t bitmap = detail::BitmapSize(slots);
      ActorStateEncodeInfo info;
      info.keyframe = keyframe;

      // Which records go out, and the size of each section.
      _same_presence.assign(state.sections.size(), 0u);
      _changed.resize(state.sections.size());
      _body_sizes.assign(state.sections.size(), 0u);
      _counts.assign(state.sections.size(), 0u);
      size_t s = 0u;
      auto base_it = _baseline.sections.begin();
      for (const auto &item : state.sections) {
        const ActorStateColumn &section = item.second;
        auto &changed = _changed[s];
        changed.assign(slots, 0u);
        size_t body = 0u;
        size_t count = 0u;
        if (keyframe) {
          body += bitmap;
          for (size_t i = 0u; i < slots; ++i) {
            if (section.IsPresent(i)) {
              changed[i] = 1u;
              body += RecordWireSize(section, i);
              ++count;
            }
          }
        } else {
          // The same table, so the same sections in the same order.
          const ActorStateColumn &base = (base_it++)->second;
          _same_presence[s] = section.SamePresence(base) ? 1u : 0u;
          if (_same_presence[s] == 0u) {
            body += bitmap;
          }
          for (size_t i = 0u; i < slots; ++i) {
            if (section.IsPresent(i) && !section.SameRecord(i, base)) {
              changed[i] = 1u;
              body += RecordWireSize(section, i);
              ++count;
            }
          }
          if (count > 0u) {
            body += bitmap;
          }
        }
        _body_sizes[s] = body;
        _counts[s] = count;
        info.records += count;
        ++s;
      }

      size_t payload = detail::kActorStateHeaderSize;
      if (keyframe) {
        payload += slots * (sizeof(uint32_t) + sizeof(uint8_t));
      }
      for (const size_t body : _body_sizes) {
        payload += detail::kActorStateSectionHeaderSize + body;
      }

      _wire.resize(kFrameDataHeaderSize + payload);
      detail::SpanWriter w(_wire.data());
      w.Put(kActorStatePacketId);
      w.Put(static_cast<uint32_t>(payload));
      w.Put(kActorStateLayout);
      w.Put(keyframe ? detail::kActorStateKeyframe : uint8_t{0u});
      w.Put(static_cast<uint16_t>(state.sections.size()));
      w.Put(static_cast<uint32_t>(slots));
      w.Put(state.frame);
      w.Put(keyframe ? state.frame : _baseline.frame);
      w.Put(table_hash);
      if (keyframe && (slots > 0u)) {
        w.PutBytes(state.ids.data(), slots * sizeof(uint32_t));
        w.PutBytes(state.kinds.data(), slots);
      }
      s = 0u;
      for (const auto &item : state.sections) {
        const ActorStateColumn &section = item.second;
        const auto &changed = _changed[s];
        const bool any_changed = _counts[s] > 0u;
        uint8_t flags = 0u;
        if (!keyframe) {
          if (_same_presence[s] != 0u) {
            flags |= detail::kActorStateSamePresence;
          }
          if (!any_changed) {
            flags |= detail::kActorStateNoneChanged;
          }
        }
        w.Put(section.Kind());
        w.Put(flags);
        w.Put(section.Stride());
        w.Put(static_cast<uint32_t>(_body_sizes[s]));
        if ((flags & detail::kActorStateSamePresence) == 0u) {
          detail::WriteBitmap(w, slots, [&](size_t i) { return section.IsPresent(i); });
        }
        if (!keyframe && any_changed) {
          detail::WriteBitmap(w, slots, [&](size_t i) { return changed[i] != 0u; });
        }
        for (size_t i = 0u; i < slots; ++i) {
          if (changed[i] == 0u) {
            continue;
          }
          const size_t size = section.Size(i);
          if (section.IsVariable()) {
            w.Put(static_cast<uint32_t>(size));
          }
          if (size > 0u) {
            w.PutBytes(section.Data(i), size);
          }
        }
        ++s;
      }
      out.write(reinterpret_cast<const char *>(_wire.data()), static_cast<std::streamsize>(_wire.size()));
      info.bytes = kFrameDataHeaderSize + payload;

      _baseline.CopyFrom(state);
      _baseline_hash = table_hash;
      _has_baseline = true;
      _since_keyframe = keyframe ? 0u : _since_keyframe + 1u;
      return info;
    }

  private:

    static size_t RecordWireSize(const ActorStateColumn &section, size_t slot) {
      return section.Size(slot) + (section.IsVariable() ? sizeof(uint32_t) : 0u);
    }

    uint32_t _keyframe_interval;

    uint32_t _since_keyframe = 0u;

    bool _has_baseline = false;

    uint64_t _baseline_hash = 0u;

    ActorStateFrame _baseline;

    std::vector<uint8_t> _same_presence;

    std::vector<std::vector<uint8_t>> _changed;

    std::vector<size_t> _body_sizes;

    std::vector<size_t> _counts;

    std::vector<uint8_t> _wire;
  };

  enum class ActorStateDecodeStatus : uint8_t {
    /// The state of the packet's frame is complete.
    Ok,
    /// A delta that does not apply to the state held (a frame was lost, the
    /// table changed, or there is no state yet): ask for a resync. Every delta
    /// is refused until a keyframe arrives.
    NeedResync,
    /// A packet that contradicts its own layout. The state held is unchanged.
    Malformed,
    /// A layout this build does not know.
    UnknownLayout
  };

  inline const char *ActorStateDecodeStatusName(ActorStateDecodeStatus status) {
    switch (status) {
      case ActorStateDecodeStatus::Ok: return "ok";
      case ActorStateDecodeStatus::NeedResync: return "delta without its base frame";
      case ActorStateDecodeStatus::Malformed: return "malformed packet";
      case ActorStateDecodeStatus::UnknownLayout: return "unknown layout";
    }
    return "unknown status";
  }

  struct ActorStateDecodeResult {
    ActorStateDecodeStatus status = ActorStateDecodeStatus::Malformed;
    /// The packet's frame, if its header could be read.
    std::optional<uint64_t> frame;
    bool keyframe = false;

    bool IsOk() const {
      return status == ActorStateDecodeStatus::Ok;
    }
  };

  /// Rebuilds the whole state of each frame from packet 35. A packet that is
  /// refused leaves the state held as it was.
  class ActorStateDecoder {
  public:

    /// Decodes the payload of one packet 35 (without the frame data header).
    ActorStateDecodeResult Decode(const uint8_t *data, size_t size) {
      ActorStateDecodeResult result;
      detail::ByteCursor in(data, size);
      uint8_t layout = 0u;
      uint8_t flags = 0u;
      uint16_t section_count = 0u;
      uint32_t slot_count = 0u;
      uint64_t frame = 0u;
      uint64_t base_frame = 0u;
      uint64_t table_hash = 0u;
      if (!in.Read(layout)) {
        return result;
      }
      if (layout != kActorStateLayout) {
        result.status = ActorStateDecodeStatus::UnknownLayout;
        return result;
      }
      if (!in.Read(flags) || !in.Read(section_count) || !in.Read(slot_count) ||
          !in.Read(frame) || !in.Read(base_frame) || !in.Read(table_hash) ||
          ((flags & ~detail::kActorStateKeyframe) != 0u)) {
        return result;
      }
      result.frame = frame;
      result.keyframe = (flags & detail::kActorStateKeyframe) != 0u;

      if (!result.keyframe) {
        if (_waiting_resync || !_has_state ||
            (base_frame != _state.frame) || (frame != base_frame + 1u) ||
            (table_hash != _state_hash) || (slot_count != _state.SlotCount()) ||
            (section_count != _state.sections.size())) {
          _waiting_resync = true;
          result.status = ActorStateDecodeStatus::NeedResync;
          return result;
        }
      } else if (base_frame != frame) {
        return result;
      }

      const size_t slots = slot_count;
      const size_t bitmap = detail::BitmapSize(slots);
      // A slot takes at least its id and kind on a keyframe, and a bit per
      // section on a delta: refuse counts the payload cannot hold before
      // allocating for them.
      if (result.keyframe && (slots > in.Remaining() / (sizeof(uint32_t) + 1u))) {
        return result;
      }

      ActorStateFrame &next = _next;
      if (result.keyframe) {
        next.ids.resize(slots);
        next.kinds.resize(slots);
        if ((slots > 0u) &&
            (!in.ReadBytes(next.ids.data(), slots * sizeof(uint32_t)) ||
             !in.ReadBytes(next.kinds.data(), slots))) {
          return result;
        }
        next.sections.clear();
      } else {
        next.CopyFrom(_state);
      }
      next.frame = frame;

      auto delta_it = next.sections.begin();
      int last_kind = -1;
      size_t memory_budget = 128u * size + 65536u;
      for (size_t s = 0u; s < section_count; ++s) {
        uint8_t kind = 0u;
        uint8_t section_flags = 0u;
        uint16_t stride = 0u;
        uint32_t body_size = 0u;
        if (!in.Read(kind) || !in.Read(section_flags) || !in.Read(stride) || !in.Read(body_size)) {
          return result;
        }
        const uint8_t *body_data = in.Take(body_size);
        if ((body_data == nullptr) || (static_cast<int>(kind) <= last_kind) ||
            ((section_flags & ~(detail::kActorStateSamePresence | detail::kActorStateNoneChanged)) != 0u) ||
            (result.keyframe && (section_flags != 0u))) {
          return result;
        }
        last_kind = kind;
        detail::ByteCursor body(body_data, body_size);

        ActorStateColumn *section = nullptr;
        if (result.keyframe) {
          // A section takes memory for every slot. A real packet needs far
          // less than this budget; a corrupted stride or count could ask for
          // gigabytes.
          const size_t cost = slots * std::max<size_t>(stride, sizeof(std::vector<uint8_t>));
          if (cost > memory_budget) {
            return result;
          }
          memory_budget -= cost;
          section = &next.sections.emplace_hint(
              next.sections.end(), kind, ActorStateColumn(kind, stride))->second;
          section->Reset(slots);
        } else {
          section = &(delta_it++)->second;
          if ((section->Kind() != kind) || (section->Stride() != stride)) {
            return result;
          }
        }

        // Presence.
        if ((section_flags & detail::kActorStateSamePresence) == 0u) {
          const uint8_t *bits = body.Take(bitmap);
          if ((bits == nullptr) || !detail::ReadBitmap(bits, slots, _present)) {
            return result;
          }
        } else {
          _present.resize(slots);
          for (size_t i = 0u; i < slots; ++i) {
            _present[i] = section->IsPresent(i) ? 1u : 0u;
          }
        }

        // Which records come in this packet.
        if (result.keyframe) {
          _changed = _present;
        } else if ((section_flags & detail::kActorStateNoneChanged) == 0u) {
          const uint8_t *bits = body.Take(bitmap);
          if ((bits == nullptr) || !detail::ReadBitmap(bits, slots, _changed)) {
            return result;
          }
        } else {
          _changed.assign(slots, 0u);
        }

        for (size_t i = 0u; i < slots; ++i) {
          if (_changed[i] != 0u) {
            if (_present[i] == 0u) {
              return result;
            }
            uint32_t record_size = stride;
            if (stride == 0u && !body.Read(record_size)) {
              return result;
            }
            const uint8_t *record = body.Take(record_size);
            if (record == nullptr) {
              return result;
            }
            section->SetBytes(i, record, record_size);
          } else if (_present[i] == 0u) {
            if (section->IsPresent(i)) {
              section->Clear(i);
            }
          } else if (!section->IsPresent(i)) {
            // Present now, unchanged, but absent in the base frame.
            return result;
          }
        }
        if (body.Remaining() != 0u) {
          return result;
        }
      }
      if (in.Remaining() != 0u) {
        return result;
      }

      // A delta keeps the table, already checked against the hash.
      const uint64_t next_hash = result.keyframe ? next.TableHash() : _state_hash;
      if (next_hash != table_hash) {
        return result;
      }
      std::swap(_state, _next);
      _state_hash = next_hash;
      _has_state = true;
      _waiting_resync = false;
      result.status = ActorStateDecodeStatus::Ok;
      return result;
    }

    /// The state of the last frame decoded.
    const ActorStateFrame &State() const {
      return _state;
    }

    bool HasState() const {
      return _has_state;
    }

    /// The last frame whose state is complete, if any.
    std::optional<uint64_t> LastGoodFrame() const {
      return _has_state ? std::optional<uint64_t>(_state.frame) : std::nullopt;
    }

    /// Whether deltas are refused until a keyframe arrives.
    bool WaitingResync() const {
      return _waiting_resync;
    }

    void Reset() {
      _has_state = false;
      _waiting_resync = false;
    }

  private:

    ActorStateFrame _state;

    ActorStateFrame _next;

    uint64_t _state_hash = 0u;

    bool _has_state = false;

    bool _waiting_resync = false;

    std::vector<uint8_t> _present;

    std::vector<uint8_t> _changed;
  };

} // namespace multigpu
} // namespace carla
