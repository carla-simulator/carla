// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace carla {
namespace rpc {

  // ===========================================================================
  // -- Actor update batch -----------------------------------------------------
  // ===========================================================================
  //
  // Per-actor updates for many actors in one binary blob, sent with the
  // apply_actor_updates RPC. One section per kind of update holds the actor
  // ids and a fixed-size record per actor. The server applies the sections in
  // the order of their kind (SimulatePhysics first, TrafficLightState last)
  // and each section in array order.
  //
  // Layout, little-endian, no padding:
  //
  //   BlobHeader     magic "CAUB", u16 version, u16 section count, u32 size
  //   per section:   SectionHeader {u16 kind, u16 stride, u32 count}
  //                  u32 ids[count]
  //                  stride-byte records[count]
  //
  // Positions are f64 meters: on a World Partition map far from the origin, a
  // float would round them (0.49 mm of resolution at 7 km).

  enum class ActorUpdateKind : uint16_t {
    SimulatePhysics       = 1u,  ///< u8 enabled
    Transform             = 2u,  ///< f64 x, y, z (m); f32 pitch, yaw, roll (deg)
    Location              = 3u,  ///< f64 x, y, z (m)
    WalkerState           = 4u,  ///< Transform, then f32 speed (m/s)
    TargetVelocity        = 5u,  ///< f32 x, y, z (m/s)
    TargetAngularVelocity = 6u,  ///< f32 x, y, z (deg/s)
    VehicleControl        = 7u,  ///< f32 throttle, steer, brake; u8 hand_brake, reverse, manual_gear_shift, 0; i32 gear
    AckermannControl      = 8u,  ///< f32 steer, steer_speed, speed, acceleration, jerk
    WalkerControl         = 9u,  ///< f32 direction x, y, z; f32 speed; u8 jump, 0, 0, 0
    VehicleLightState     = 10u, ///< u32 flags
    TrafficLightState     = 11u, ///< u8 state
  };

  constexpr uint16_t kActorUpdateKindCount = 11u;

  /// Bytes of one record of @a kind, 0 for an unknown kind.
  constexpr uint16_t ActorUpdateStride(ActorUpdateKind kind) {
    switch (kind) {
      case ActorUpdateKind::SimulatePhysics:       return 1u;
      case ActorUpdateKind::Transform:             return 36u;
      case ActorUpdateKind::Location:              return 24u;
      case ActorUpdateKind::WalkerState:           return 40u;
      case ActorUpdateKind::TargetVelocity:        return 12u;
      case ActorUpdateKind::TargetAngularVelocity: return 12u;
      case ActorUpdateKind::VehicleControl:        return 20u;
      case ActorUpdateKind::AckermannControl:      return 20u;
      case ActorUpdateKind::WalkerControl:         return 20u;
      case ActorUpdateKind::VehicleLightState:     return 4u;
      case ActorUpdateKind::TrafficLightState:     return 1u;
    }
    return 0u;
  }

  constexpr std::array<char, 4> kActorUpdateMagic = {'C', 'A', 'U', 'B'};
  constexpr uint16_t kActorUpdateVersion = 1u;
  constexpr size_t kActorUpdateHeaderSize = 4u + 2u + 2u + 4u;
  constexpr size_t kActorUpdateSectionHeaderSize = 2u + 2u + 4u;

  /// Outcome of one update, one byte per update in the order of the blob.
  enum class ActorUpdateStatus : uint8_t {
    Success = 0u,
    ActorNotFound = 1u,
    /// The actor exists but the update failed, for example a vehicle control
    /// sent to a walker (see the server log).
    Failed = 2u,
  };

  // ===========================================================================
  // -- Writing ----------------------------------------------------------------
  // ===========================================================================

  /// Builds a blob. Add updates with the typed Add* functions, then Finish.
  /// Reusable: Finish leaves it empty.
  class ActorUpdateBatchBuilder {
  public:

    void AddSimulatePhysics(uint32_t id, bool enabled) {
      auto &s = Section(ActorUpdateKind::SimulatePhysics, id);
      Put<uint8_t>(s, enabled ? 1u : 0u);
    }

    void AddTransform(uint32_t id, const double location[3], const float rotation[3]) {
      auto &s = Section(ActorUpdateKind::Transform, id);
      PutPose(s, location, rotation);
    }

    void AddLocation(uint32_t id, const double location[3]) {
      auto &s = Section(ActorUpdateKind::Location, id);
      for (int i = 0; i < 3; ++i) {
        Put(s, location[i]);
      }
    }

    void AddWalkerState(uint32_t id, const double location[3], const float rotation[3], float speed) {
      auto &s = Section(ActorUpdateKind::WalkerState, id);
      PutPose(s, location, rotation);
      Put(s, speed);
    }

    void AddTargetVelocity(uint32_t id, const float velocity[3]) {
      auto &s = Section(ActorUpdateKind::TargetVelocity, id);
      for (int i = 0; i < 3; ++i) {
        Put(s, velocity[i]);
      }
    }

    void AddTargetAngularVelocity(uint32_t id, const float velocity[3]) {
      auto &s = Section(ActorUpdateKind::TargetAngularVelocity, id);
      for (int i = 0; i < 3; ++i) {
        Put(s, velocity[i]);
      }
    }

    void AddVehicleControl(
        uint32_t id, float throttle, float steer, float brake,
        bool hand_brake, bool reverse, bool manual_gear_shift, int32_t gear) {
      auto &s = Section(ActorUpdateKind::VehicleControl, id);
      Put(s, throttle);
      Put(s, steer);
      Put(s, brake);
      Put<uint8_t>(s, hand_brake ? 1u : 0u);
      Put<uint8_t>(s, reverse ? 1u : 0u);
      Put<uint8_t>(s, manual_gear_shift ? 1u : 0u);
      Put<uint8_t>(s, 0u);
      Put(s, gear);
    }

    void AddAckermannControl(
        uint32_t id, float steer, float steer_speed, float speed, float acceleration, float jerk) {
      auto &s = Section(ActorUpdateKind::AckermannControl, id);
      Put(s, steer);
      Put(s, steer_speed);
      Put(s, speed);
      Put(s, acceleration);
      Put(s, jerk);
    }

    void AddWalkerControl(uint32_t id, const float direction[3], float speed, bool jump) {
      auto &s = Section(ActorUpdateKind::WalkerControl, id);
      for (int i = 0; i < 3; ++i) {
        Put(s, direction[i]);
      }
      Put(s, speed);
      Put<uint8_t>(s, jump ? 1u : 0u);
      Put<uint8_t>(s, 0u);
      Put<uint8_t>(s, 0u);
      Put<uint8_t>(s, 0u);
    }

    void AddVehicleLightState(uint32_t id, uint32_t flags) {
      auto &s = Section(ActorUpdateKind::VehicleLightState, id);
      Put(s, flags);
    }

    void AddTrafficLightState(uint32_t id, uint8_t state) {
      auto &s = Section(ActorUpdateKind::TrafficLightState, id);
      Put(s, state);
    }

    /// Appends @a count records of @a kind at once: @a ids and @a records
    /// (count * stride bytes, already in the wire layout).
    void AddRecords(ActorUpdateKind kind, const uint32_t *ids, const uint8_t *records, size_t count) {
      auto &s = _sections[Index(kind)];
      s.ids.insert(s.ids.end(), ids, ids + count);
      s.data.insert(s.data.end(), records, records + count * ActorUpdateStride(kind));
    }

    /// Number of updates added so far.
    size_t size() const {
      size_t total = 0u;
      for (const auto &s : _sections) {
        total += s.ids.size();
      }
      return total;
    }

    bool empty() const {
      return size() == 0u;
    }

    void Clear() {
      for (auto &s : _sections) {
        s.ids.clear();
        s.data.clear();
      }
    }

    /// The blob. The builder is empty afterwards.
    std::vector<uint8_t> Finish() {
      uint16_t section_count = 0u;
      size_t total = kActorUpdateHeaderSize;
      for (const auto &s : _sections) {
        if (!s.ids.empty()) {
          ++section_count;
          total += kActorUpdateSectionHeaderSize + s.ids.size() * sizeof(uint32_t) + s.data.size();
        }
      }
      std::vector<uint8_t> blob;
      blob.reserve(total);
      Append(blob, kActorUpdateMagic.data(), kActorUpdateMagic.size());
      AppendPod(blob, kActorUpdateVersion);
      AppendPod(blob, section_count);
      AppendPod(blob, static_cast<uint32_t>(total));
      for (uint16_t k = 0u; k < kActorUpdateKindCount; ++k) {
        const auto &s = _sections[k];
        if (s.ids.empty()) {
          continue;
        }
        const auto kind = static_cast<ActorUpdateKind>(k + 1u);
        AppendPod(blob, static_cast<uint16_t>(kind));
        AppendPod(blob, ActorUpdateStride(kind));
        AppendPod(blob, static_cast<uint32_t>(s.ids.size()));
        Append(blob, s.ids.data(), s.ids.size() * sizeof(uint32_t));
        Append(blob, s.data.data(), s.data.size());
      }
      Clear();
      return blob;
    }

  private:

    struct SectionData {
      std::vector<uint32_t> ids;
      std::vector<uint8_t> data;
    };

    static size_t Index(ActorUpdateKind kind) {
      return static_cast<size_t>(kind) - 1u;
    }

    std::vector<uint8_t> &Section(ActorUpdateKind kind, uint32_t id) {
      auto &s = _sections[Index(kind)];
      s.ids.push_back(id);
      return s.data;
    }

    template <typename T>
    static void Put(std::vector<uint8_t> &data, T value) {
      Append(data, &value, sizeof(T));
    }

    static void PutPose(std::vector<uint8_t> &data, const double location[3], const float rotation[3]) {
      for (int i = 0; i < 3; ++i) {
        Put(data, location[i]);
      }
      for (int i = 0; i < 3; ++i) {
        Put(data, rotation[i]);
      }
    }

    template <typename T>
    static void AppendPod(std::vector<uint8_t> &blob, T value) {
      Append(blob, &value, sizeof(T));
    }

    static void Append(std::vector<uint8_t> &blob, const void *data, size_t size) {
      const auto *bytes = static_cast<const uint8_t *>(data);
      blob.insert(blob.end(), bytes, bytes + size);
    }

    std::array<SectionData, kActorUpdateKindCount> _sections;
  };

  // ===========================================================================
  // -- Reading ----------------------------------------------------------------
  // ===========================================================================

  /// One section of a validated blob.
  struct ActorUpdateSection {
    ActorUpdateKind kind;
    uint32_t count;
    const uint8_t *ids;
    const uint8_t *records;

    uint32_t Id(uint32_t i) const {
      uint32_t id;
      std::memcpy(&id, ids + i * sizeof(uint32_t), sizeof(id));
      return id;
    }

    const uint8_t *Record(uint32_t i) const {
      return records + static_cast<size_t>(i) * ActorUpdateStride(kind);
    }
  };

  /// Reads a value of type T at @a offset bytes into @a record.
  template <typename T>
  inline T ReadActorUpdateField(const uint8_t *record, size_t offset) {
    T value;
    std::memcpy(&value, record + offset, sizeof(T));
    return value;
  }

namespace detail {

  inline bool ParseActorUpdateBatchSections(
      const uint8_t *data,
      size_t size,
      std::vector<ActorUpdateSection> &sections,
      std::string &error) {
    sections.clear();
    error.clear();
    if (size < kActorUpdateHeaderSize) {
      error = "blob shorter than its header";
      return false;
    }
    if (std::memcmp(data, kActorUpdateMagic.data(), kActorUpdateMagic.size()) != 0) {
      error = "not an actor update batch";
      return false;
    }
    uint16_t version;
    uint16_t section_count;
    uint32_t total;
    std::memcpy(&version, data + 4u, sizeof(version));
    std::memcpy(&section_count, data + 6u, sizeof(section_count));
    std::memcpy(&total, data + 8u, sizeof(total));
    if (version != kActorUpdateVersion) {
      error = "unsupported version " + std::to_string(version) +
          " (this build reads " + std::to_string(kActorUpdateVersion) + ")";
      return false;
    }
    if (total != size) {
      error = "blob size " + std::to_string(size) + " does not match its header (" +
          std::to_string(total) + ")";
      return false;
    }
    size_t offset = kActorUpdateHeaderSize;
    uint16_t previous_kind = 0u;
    for (uint16_t i = 0u; i < section_count; ++i) {
      if (size - offset < kActorUpdateSectionHeaderSize) {
        error = "section " + std::to_string(i) + " is truncated";
        return false;
      }
      uint16_t kind;
      uint16_t stride;
      uint32_t count;
      std::memcpy(&kind, data + offset, sizeof(kind));
      std::memcpy(&stride, data + offset + 2u, sizeof(stride));
      std::memcpy(&count, data + offset + 4u, sizeof(count));
      offset += kActorUpdateSectionHeaderSize;
      const uint16_t expected = ActorUpdateStride(static_cast<ActorUpdateKind>(kind));
      if (expected == 0u) {
        error = "unknown update kind " + std::to_string(kind);
        return false;
      }
      if (kind <= previous_kind) {
        error = "update kind " + std::to_string(kind) + " is repeated or out of order";
        return false;
      }
      if (stride != expected) {
        error = "update kind " + std::to_string(kind) + " has records of " +
            std::to_string(stride) + " bytes, expected " + std::to_string(expected);
        return false;
      }
      const uint64_t bytes = static_cast<uint64_t>(count) * (sizeof(uint32_t) + stride);
      if (bytes > size - offset) {
        error = "section " + std::to_string(i) + " is truncated";
        return false;
      }
      sections.push_back(ActorUpdateSection{
          static_cast<ActorUpdateKind>(kind),
          count,
          data + offset,
          data + offset + static_cast<size_t>(count) * sizeof(uint32_t)});
      offset += static_cast<size_t>(bytes);
      previous_kind = kind;
    }
    if (offset != size) {
      error = std::to_string(size - offset) + " bytes after the last section";
      return false;
    }
    return true;
  }

} // namespace detail

  /// Checks a whole blob before anything is applied. On success, @a sections
  /// holds its sections in application order and @a error is empty. On
  /// failure, @a sections is empty and @a error says why.
  inline bool ParseActorUpdateBatch(
      const uint8_t *data,
      size_t size,
      std::vector<ActorUpdateSection> &sections,
      std::string &error) {
    if (!detail::ParseActorUpdateBatchSections(data, size, sections, error)) {
      sections.clear();
      return false;
    }
    return true;
  }

} // namespace rpc
} // namespace carla
