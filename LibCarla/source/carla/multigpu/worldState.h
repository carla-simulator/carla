// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/multigpu/frameDataCodec.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <istream>
#include <iterator>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace carla {
namespace multigpu {

  /// Frame data packet 28, on every frame: world-wide state a client can
  /// change on the primary that changes what sensors see or measure.
  struct WorldGlobals {

    static constexpr uint8_t kPacketId = 28u;

    static constexpr uint32_t kPayloadSizeV1 = 12u;

    /// rpc::MapLayer bits of the layers loaded.
    uint16_t map_layers = 0xFFFFu;

    /// The IMU's gravity, m/s^2.
    float imu_gravity = 9.81f;

    /// UCarlaLightSubsystem: whether street lights follow the sun, and the
    /// last day/night state it broadcast (if any).
    bool day_night_cycle = true;
    bool has_day_time_state = false;
    bool last_is_day = true;

    /// ROS 2: whether TF frames are published.
    bool publish_tf = true;

    void Write(std::ostream &out) const {
      WriteFrameDataHeader(out, kPacketId, kPayloadSizeV1);
      detail::WritePod(out, map_layers);
      detail::WritePod(out, imu_gravity);
      const uint8_t flags =
          (day_night_cycle ? 1u : 0u) |
          (has_day_time_state ? 2u : 0u) |
          (last_is_day ? 4u : 0u) |
          (publish_tf ? 8u : 0u);
      detail::WritePod(out, flags);
      const uint8_t reserved[5] = {0u, 0u, 0u, 0u, 0u};
      out.write(reinterpret_cast<const char *>(reserved), sizeof(reserved));
    }

    bool Read(std::istream &in, uint32_t size) {
      if (size < kPayloadSizeV1) {
        in.setstate(std::ios_base::failbit);
        return false;
      }
      BoundedReader reader(in, size);
      uint8_t flags = 0u;
      uint8_t reserved[5];
      if (!reader.Read(map_layers) || !reader.Read(imu_gravity) || !reader.Read(flags) ||
          !reader.ReadBytes(reserved, sizeof(reserved))) {
        return false;
      }
      day_night_cycle = (flags & 1u) != 0u;
      has_day_time_state = (flags & 2u) != 0u;
      last_is_day = (flags & 4u) != 0u;
      publish_tf = (flags & 8u) != 0u;
      return true;
    }
  };

  /// Frame data packet 29, on change and on every full resync: map objects
  /// whose state is not part of any actor. Scene lights travel in packet 11,
  /// the recorder's SceneLight.
  struct WorldObjects {

    static constexpr uint8_t kPacketId = 29u;

    enum Field : uint8_t {
      /// The complete, sorted set of environment objects disabled.
      kDisabledEnvironmentObjects = 1u
    };

    std::optional<std::vector<uint64_t>> disabled_environment_objects;

    bool empty() const {
      return !disabled_environment_objects;
    }

    void Write(std::ostream &out) const {
      std::ostringstream payload;
      if (disabled_environment_objects) {
        std::ostringstream value;
        detail::WritePod(value, static_cast<uint32_t>(disabled_environment_objects->size()));
        for (uint64_t id : *disabled_environment_objects) {
          detail::WritePod(value, id);
        }
        WriteField(payload, kDisabledEnvironmentObjects, value.str());
      }
      WriteFrameDataPacket(out, kPacketId, payload.str());
    }

    bool Read(std::istream &in, uint32_t size) {
      disabled_environment_objects.reset();
      BoundedReader reader(in, size);
      return reader.ReadFields([&](uint8_t kind, BoundedReader &value) {
        switch (kind) {
          case kDisabledEnvironmentObjects: {
            uint32_t count = 0u;
            if (!value.Read(count) || (count > value.Remaining() / sizeof(uint64_t))) {
              return false;
            }
            std::vector<uint64_t> ids(count);
            for (uint64_t &id : ids) {
              if (!value.Read(id)) {
                return false;
              }
            }
            disabled_environment_objects = std::move(ids);
            return true;
          }
          default:
            return true;
        }
      });
    }
  };

  /// An actor's appearance on the primary that a mirror does not get from
  /// its spawn attributes or pose.
  struct ActorAppearance {
    uint32_t actor_id = 0u;
    std::optional<std::array<double, 3>> scale;
    std::optional<bool> collisions;
    std::optional<bool> hidden;
  };

  /// Frame data packet 30, on change and on every full resync.
  struct ActorAppearances {

    static constexpr uint8_t kPacketId = 30u;

    enum Field : uint8_t {
      kScale = 1u,
      kCollisions = 2u,
      kHidden = 3u
    };

    std::vector<ActorAppearance> actors;

    void Write(std::ostream &out) const {
      std::ostringstream payload;
      detail::WritePod(payload, static_cast<uint32_t>(actors.size()));
      for (const ActorAppearance &actor : actors) {
        std::ostringstream record;
        detail::WritePod(record, actor.actor_id);
        if (actor.scale) {
          std::ostringstream value;
          for (double axis : *actor.scale) {
            detail::WritePod(value, axis);
          }
          WriteField(record, kScale, value.str());
        }
        if (actor.collisions) {
          WriteField(record, kCollisions, std::string(1u, *actor.collisions ? '\1' : '\0'));
        }
        if (actor.hidden) {
          WriteField(record, kHidden, std::string(1u, *actor.hidden ? '\1' : '\0'));
        }
        WriteSizedBytes(payload, record.str());
      }
      WriteFrameDataPacket(out, kPacketId, payload.str());
    }

    bool Read(std::istream &in, uint32_t size) {
      actors.clear();
      BoundedReader reader(in, size);
      uint32_t count = 0u;
      if (!reader.ReadVariableListCount(count)) {
        return false;
      }
      for (uint32_t i = 0u; i < count; ++i) {
        ActorAppearance actor;
        const bool ok = reader.ReadSizedRecord([&](BoundedReader &record) {
          return record.Read(actor.actor_id) && record.ReadFields([&](uint8_t kind, BoundedReader &value) {
            switch (kind) {
              case kScale: {
                std::array<double, 3> scale;
                for (double &axis : scale) {
                  if (!value.Read(axis)) {
                    return false;
                  }
                }
                actor.scale = scale;
                return true;
              }
              case kCollisions:
              case kHidden: {
                uint8_t flag = 0u;
                if (!value.Read(flag)) {
                  return false;
                }
                (kind == kCollisions ? actor.collisions : actor.hidden) = (flag != 0u);
                return true;
              }
              default:
                return true;
            }
          });
        });
        if (!ok) {
          return false;
        }
        actors.push_back(std::move(actor));
      }
      return true;
    }
  };

  /// The map layers to load and to unload to go from @a current to @a desired.
  inline std::pair<uint16_t, uint16_t> ComputeMapLayerDiff(uint16_t current, uint16_t desired) {
    return {static_cast<uint16_t>(desired & ~current), static_cast<uint16_t>(current & ~desired)};
  }

  /// Ids in @a desired and not in @a current (first), and the other way round
  /// (second). Both inputs sorted.
  inline std::pair<std::vector<uint64_t>, std::vector<uint64_t>> DiffSortedIds(
      const std::vector<uint64_t> &current,
      const std::vector<uint64_t> &desired) {
    std::pair<std::vector<uint64_t>, std::vector<uint64_t>> diff;
    std::set_difference(desired.begin(), desired.end(), current.begin(), current.end(),
        std::back_inserter(diff.first));
    std::set_difference(current.begin(), current.end(), desired.begin(), desired.end(),
        std::back_inserter(diff.second));
    return diff;
  }

} // namespace multigpu
} // namespace carla
