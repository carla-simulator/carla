// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/multigpu/frameDataCodec.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace carla {
namespace multigpu {

  /// One record of frame data packet 34: an actor's world pose as the primary
  /// holds it, in UE units, with its rotation as a quaternion (an Euler round
  /// trip snaps pitch near +-90 degrees, e.g. for cameras looking down).
  struct MirroredPosition {
    uint32_t id = 0u;
    double location[3] = {0.0, 0.0, 0.0};
    /// x, y, z, w
    double rotation[4] = {0.0, 0.0, 0.0, 1.0};
  };

  /// Frame data packet 34: every live actor's pose in this frame.
  struct MirroredPositions {

    static constexpr uint8_t kPacketId = 34u;

    static constexpr uint16_t kRecordSizeV1 = 60u;

    std::vector<MirroredPosition> positions;

    void Write(std::ostream &out) const {
      const uint32_t count = static_cast<uint32_t>(positions.size());
      WriteFrameDataHeader(out, kPacketId, sizeof(uint32_t) + sizeof(uint16_t) + count * kRecordSizeV1);
      WriteFixedListHeader(out, count, kRecordSizeV1);
      for (const MirroredPosition &position : positions) {
        detail::WritePod(out, position.id);
        for (double value : position.location) {
          detail::WritePod(out, value);
        }
        for (double value : position.rotation) {
          detail::WritePod(out, value);
        }
      }
    }

    /// Reads a payload of @a size bytes; records that grew in a newer build
    /// are read up to the fields known here.
    bool Read(std::istream &in, uint32_t size) {
      positions.clear();
      BoundedReader reader(in, size);
      uint32_t count = 0u;
      uint16_t record_size = 0u;
      if (!reader.ReadFixedListHeader(kRecordSizeV1, count, record_size)) {
        return false;
      }
      positions.reserve(count);
      for (uint32_t i = 0u; i < count; ++i) {
        MirroredPosition position;
        const bool ok = reader.ReadRecord(record_size, [&](BoundedReader &record) {
          bool read = record.Read(position.id);
          for (double &value : position.location) {
            read = read && record.Read(value);
          }
          for (double &value : position.rotation) {
            read = read && record.Read(value);
          }
          return read;
        });
        if (!ok) {
          return false;
        }
        positions.push_back(position);
      }
      return true;
    }
  };

  /// A wheel as the primary draws it: degrees and cm.
  struct WheelPose {
    float steer = 0.0f;
    float spin = 0.0f;
    float suspension_offset = 0.0f;
  };

  /// An open door, which on the primary is a physics body of its own: its pose
  /// relative to the vehicle, in UE units, rotation as a quaternion (x, y, z, w).
  struct DoorPose {
    uint8_t door = 0u;
    std::array<double, 3> location = {0.0, 0.0, 0.0};
    std::array<double, 4> rotation = {0.0, 0.0, 0.0, 1.0};
  };

  /// A vehicle's moving parts: every wheel, by wheel index, and the doors
  /// open (any other door is closed), and its drivetrain as the primary's
  /// simulation has it: what the vehicle's sensors report.
  struct VehicleParts {
    uint32_t actor_id = 0u;
    std::vector<WheelPose> wheels;
    std::vector<DoorPose> open_doors;
    std::optional<int32_t> current_gear;
    bool manual_gear_shift = false;
    /// Whether the primary's body simulates physics (it reports no angular
    /// velocity otherwise).
    bool simulating_physics = false;
  };

  /// Frame data packet 32, every frame: the moving parts of every vehicle.
  struct MirroredVehicleParts {

    static constexpr uint8_t kPacketId = 32u;

    enum Field : uint8_t {
      kWheels = 1u,
      kOpenDoors = 2u,
      kDrivetrain = 3u
    };

    static constexpr uint16_t kWheelRecordSizeV1 = 12u;

    static constexpr uint16_t kDoorRecordSizeV1 = 1u + 3u * 8u + 4u * 8u;

    std::vector<VehicleParts> vehicles;

    void Write(std::ostream &out) const {
      std::ostringstream payload;
      detail::WritePod(payload, static_cast<uint32_t>(vehicles.size()));
      for (const VehicleParts &vehicle : vehicles) {
        std::ostringstream record;
        detail::WritePod(record, vehicle.actor_id);
        {
          std::ostringstream value;
          WriteFixedListHeader(value, static_cast<uint32_t>(vehicle.wheels.size()), kWheelRecordSizeV1);
          for (const WheelPose &wheel : vehicle.wheels) {
            detail::WritePod(value, wheel.steer);
            detail::WritePod(value, wheel.spin);
            detail::WritePod(value, wheel.suspension_offset);
          }
          WriteField(record, kWheels, value.str());
        }
        if (!vehicle.open_doors.empty()) {
          std::ostringstream value;
          WriteFixedListHeader(value, static_cast<uint32_t>(vehicle.open_doors.size()), kDoorRecordSizeV1);
          for (const DoorPose &door : vehicle.open_doors) {
            detail::WritePod(value, door.door);
            for (double axis : door.location) {
              detail::WritePod(value, axis);
            }
            for (double axis : door.rotation) {
              detail::WritePod(value, axis);
            }
          }
          WriteField(record, kOpenDoors, value.str());
        }
        if (vehicle.current_gear) {
          std::ostringstream value;
          detail::WritePod(value, *vehicle.current_gear);
          const uint8_t flags =
              (vehicle.manual_gear_shift ? 1u : 0u) | (vehicle.simulating_physics ? 2u : 0u);
          detail::WritePod(value, flags);
          WriteField(record, kDrivetrain, value.str());
        }
        WriteSizedBytes(payload, record.str());
      }
      WriteFrameDataPacket(out, kPacketId, payload.str());
    }

    bool Read(std::istream &in, uint32_t size) {
      vehicles.clear();
      BoundedReader reader(in, size);
      uint32_t count = 0u;
      if (!reader.ReadVariableListCount(count)) {
        return false;
      }
      for (uint32_t i = 0u; i < count; ++i) {
        VehicleParts vehicle;
        const bool ok = reader.ReadSizedRecord([&](BoundedReader &record) {
          return record.Read(vehicle.actor_id) && record.ReadFields([&](uint8_t kind, BoundedReader &value) {
            switch (kind) {
              case kWheels:
                return ReadList(value, kWheelRecordSizeV1, vehicle.wheels, [](BoundedReader &item, WheelPose &wheel) {
                  return item.Read(wheel.steer) && item.Read(wheel.spin) && item.Read(wheel.suspension_offset);
                });
              case kOpenDoors:
                return ReadList(value, kDoorRecordSizeV1, vehicle.open_doors, [](BoundedReader &item, DoorPose &door) {
                  bool read = item.Read(door.door);
                  for (double &axis : door.location) {
                    read = read && item.Read(axis);
                  }
                  for (double &axis : door.rotation) {
                    read = read && item.Read(axis);
                  }
                  return read;
                });
              case kDrivetrain: {
                int32_t gear = 0;
                uint8_t flags = 0u;
                if (!value.Read(gear) || !value.Read(flags)) {
                  return false;
                }
                vehicle.current_gear = gear;
                vehicle.manual_gear_shift = (flags & 1u) != 0u;
                vehicle.simulating_physics = (flags & 2u) != 0u;
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
        vehicles.push_back(std::move(vehicle));
      }
      return true;
    }

  private:

    template <typename T, typename F>
    static bool ReadList(BoundedReader &value, uint16_t min_record_size, std::vector<T> &out, F &&read) {
      uint32_t count = 0u;
      uint16_t record_size = 0u;
      if (!value.ReadFixedListHeader(min_record_size, count, record_size)) {
        return false;
      }
      out.clear();
      out.reserve(count);
      for (uint32_t i = 0u; i < count; ++i) {
        T item;
        if (!value.ReadRecord(record_size, [&](BoundedReader &record) { return read(record, item); })) {
          return false;
        }
        out.push_back(item);
      }
      return true;
    }
  };

  /// Order in which to place actors so that every parent is placed before its
  /// children: by attachment depth, keeping the input order within a depth.
  /// @a parents maps an id to its parent's id (absent or 0 for a root). A
  /// parent that is not among @a ids counts as a root, and so does every actor
  /// of a cycle, so the result always holds every id once.
  inline std::vector<uint32_t> OrderParentsFirst(
      const std::vector<uint32_t> &ids,
      const std::unordered_map<uint32_t, uint32_t> &parents) {
    std::unordered_map<uint32_t, int> in_list;
    for (uint32_t id : ids) {
      in_list.emplace(id, 0);
    }
    std::unordered_map<uint32_t, size_t> depth;
    auto depth_of = [&](uint32_t id) {
      // Walk up the chain; a chain longer than the list can only be a cycle.
      size_t steps = 0u;
      uint32_t current = id;
      while (steps <= ids.size()) {
        const auto cached = depth.find(current);
        if (cached != depth.end()) {
          return cached->second + steps;
        }
        const auto parent = parents.find(current);
        if ((parent == parents.end()) || (parent->second == 0u) ||
            (in_list.find(parent->second) == in_list.end())) {
          return steps;
        }
        current = parent->second;
        ++steps;
      }
      return size_t{0u};
    };
    std::vector<std::pair<size_t, uint32_t>> ordered;
    ordered.reserve(ids.size());
    for (uint32_t id : ids) {
      const size_t d = depth_of(id);
      depth.emplace(id, d);
      ordered.emplace_back(d, id);
    }
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto &a, const auto &b) {
      return a.first < b.first;
    });
    std::vector<uint32_t> result;
    result.reserve(ordered.size());
    for (const auto &item : ordered) {
      result.push_back(item.second);
    }
    return result;
  }

} // namespace multigpu
} // namespace carla
