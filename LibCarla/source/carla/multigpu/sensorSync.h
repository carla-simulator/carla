// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/multigpu/frameDataCodec.h"

#include <array>
#include <cstdint>
#include <istream>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace carla {
namespace multigpu {

  /// One record of frame data packet 26: a sensor of the primary that ticks
  /// in this frame, and the delta of that tick.
  struct SensorTick {
    uint32_t actor_id = 0u;
    float delta_seconds = 0.0f;
  };

  /// Frame data packet 26: every sensor of the primary that ticks in this
  /// frame. A secondary's mirrors tick on exactly these frames, with these
  /// deltas, whatever their own tick interval says, so their sensor_tick
  /// phase is the primary's even if they were created later. Written on every
  /// frame, even when empty.
  struct SensorTicks {

    static constexpr uint8_t kPacketId = 26u;

    static constexpr uint16_t kRecordSizeV1 = 8u;

    std::vector<SensorTick> ticks;

    void Write(std::ostream &out) const {
      const uint32_t count = static_cast<uint32_t>(ticks.size());
      WriteFrameDataHeader(out, kPacketId, sizeof(uint32_t) + sizeof(uint16_t) + count * kRecordSizeV1);
      WriteFixedListHeader(out, count, kRecordSizeV1);
      for (const SensorTick &tick : ticks) {
        detail::WritePod(out, tick.actor_id);
        detail::WritePod(out, tick.delta_seconds);
      }
    }

    bool Read(std::istream &in, uint32_t size) {
      ticks.clear();
      BoundedReader reader(in, size);
      uint32_t count = 0u;
      uint16_t record_size = 0u;
      if (!reader.ReadFixedListHeader(kRecordSizeV1, count, record_size)) {
        return false;
      }
      ticks.reserve(count);
      for (uint32_t i = 0u; i < count; ++i) {
        SensorTick tick;
        const bool ok = reader.ReadRecord(record_size, [&](BoundedReader &record) {
          return record.Read(tick.actor_id) && record.Read(tick.delta_seconds);
        });
        if (!ok) {
          return false;
        }
        ticks.push_back(tick);
      }
      return true;
    }
  };

  /// An IMU's history: its two previous locations (world, cm) and the delta
  /// between them; its accelerometer is a second difference over them.
  struct ImuHistory {
    std::array<double, 3> older_location = {0.0, 0.0, 0.0};
    std::array<double, 3> last_location = {0.0, 0.0, 0.0};
    float last_delta_seconds = 0.0f;
  };

  /// One record of frame data packet 27: the runtime state a sensor of the
  /// primary starts this frame's tick from. A mirror created at another frame
  /// starts from its own defaults; with this it computes exactly what the
  /// primary's sensor would.
  struct SensorState {
    uint32_t actor_id = 0u;
    /// URandomEngine (noise, lidar drop-off, radar ray directions).
    std::optional<uint32_t> random_engine;
    /// Ray-cast lidars: where this tick's sweep starts (radians).
    std::optional<float> lidar_horizontal_angle;
    std::optional<ImuHistory> imu_history;
    /// Radar: its previous location (world, cm), for its own velocity.
    std::optional<std::array<double, 3>> radar_previous_location;
    /// IMU: its rotation relative to its parent (x, y, z, w), which turns the
    /// parent's angular velocity into the gyroscope's. A mirror is attached
    /// rigidly, so a spring arm's rotation would differ.
    std::optional<std::array<double, 4>> imu_local_rotation;

    bool empty() const {
      return !random_engine && !lidar_horizontal_angle && !imu_history && !radar_previous_location &&
          !imu_local_rotation;
    }
  };

  /// Frame data packet 27, for the sensors of packet 26 that carry state.
  struct SensorStates {

    static constexpr uint8_t kPacketId = 27u;

    /// Field kinds; a reader skips the kinds it does not know.
    enum Field : uint8_t {
      kRandomEngine = 1u,
      kLidarHorizontalAngle = 2u,
      kImuHistory = 3u,
      kRadarPreviousLocation = 4u,
      kImuLocalRotation = 5u
    };

    std::vector<SensorState> states;

    void Write(std::ostream &out) const {
      std::ostringstream payload;
      detail::WritePod(payload, static_cast<uint32_t>(states.size()));
      for (const SensorState &state : states) {
        std::ostringstream record;
        detail::WritePod(record, state.actor_id);
        if (state.random_engine) {
          WriteField(record, kRandomEngine, Bytes(*state.random_engine));
        }
        if (state.lidar_horizontal_angle) {
          WriteField(record, kLidarHorizontalAngle, Bytes(*state.lidar_horizontal_angle));
        }
        if (state.imu_history) {
          std::ostringstream value;
          WriteVector(value, state.imu_history->older_location);
          WriteVector(value, state.imu_history->last_location);
          detail::WritePod(value, state.imu_history->last_delta_seconds);
          WriteField(record, kImuHistory, value.str());
        }
        if (state.radar_previous_location) {
          std::ostringstream value;
          WriteVector(value, *state.radar_previous_location);
          WriteField(record, kRadarPreviousLocation, value.str());
        }
        if (state.imu_local_rotation) {
          std::ostringstream value;
          for (double axis : *state.imu_local_rotation) {
            detail::WritePod(value, axis);
          }
          WriteField(record, kImuLocalRotation, value.str());
        }
        WriteSizedBytes(payload, record.str());
      }
      WriteFrameDataPacket(out, kPacketId, payload.str());
    }

    bool Read(std::istream &in, uint32_t size) {
      states.clear();
      BoundedReader reader(in, size);
      uint32_t count = 0u;
      if (!reader.ReadVariableListCount(count)) {
        return false;
      }
      for (uint32_t i = 0u; i < count; ++i) {
        SensorState state;
        const bool ok = reader.ReadSizedRecord([&](BoundedReader &record) {
          return record.Read(state.actor_id) && record.ReadFields([&](uint8_t kind, BoundedReader &value) {
            switch (kind) {
              case kRandomEngine: {
                uint32_t engine = 0u;
                if (!value.Read(engine)) {
                  return false;
                }
                state.random_engine = engine;
                return true;
              }
              case kLidarHorizontalAngle: {
                float angle = 0.0f;
                if (!value.Read(angle)) {
                  return false;
                }
                state.lidar_horizontal_angle = angle;
                return true;
              }
              case kImuHistory: {
                ImuHistory history;
                if (!ReadVector(value, history.older_location) ||
                    !ReadVector(value, history.last_location) ||
                    !value.Read(history.last_delta_seconds)) {
                  return false;
                }
                state.imu_history = history;
                return true;
              }
              case kRadarPreviousLocation: {
                std::array<double, 3> location;
                if (!ReadVector(value, location)) {
                  return false;
                }
                state.radar_previous_location = location;
                return true;
              }
              case kImuLocalRotation: {
                std::array<double, 4> rotation;
                for (double &axis : rotation) {
                  if (!value.Read(axis)) {
                    return false;
                  }
                }
                state.imu_local_rotation = rotation;
                return true;
              }
              default:
                return true;  // a kind of a newer build: skipped
            }
          });
        });
        if (!ok) {
          return false;
        }
        states.push_back(std::move(state));
      }
      return true;
    }

  private:

    template <typename T>
    static std::string Bytes(const T &value) {
      std::ostringstream out;
      detail::WritePod(out, value);
      return out.str();
    }

    static void WriteVector(std::ostream &out, const std::array<double, 3> &vector) {
      for (double value : vector) {
        detail::WritePod(out, value);
      }
    }

    static bool ReadVector(BoundedReader &reader, std::array<double, 3> &vector) {
      for (double &value : vector) {
        if (!reader.Read(value)) {
          return false;
        }
      }
      return true;
    }
  };

  /// The state of a minstd_rand: the value its next draw is computed from.
  inline uint32_t GetEngineState(const std::minstd_rand &engine) {
    std::minstd_rand copy = engine;
    // copy() returns 48271 * x mod (2^31 - 1); 1899818559 is 48271's
    // inverse modulo 2^31 - 1, so this recovers x.
    const uint64_t next = copy();
    return static_cast<uint32_t>((next * uint64_t{1899818559u}) % uint64_t{2147483647u});
  }

  /// Makes @a engine continue from @a state, as GetEngineState returned it.
  /// False for a value no minstd_rand can hold.
  inline bool SetEngineState(std::minstd_rand &engine, uint32_t state) {
    if ((state == 0u) || (state >= std::minstd_rand::modulus)) {
      return false;
    }
    engine.seed(state);
    return true;
  }

} // namespace multigpu
} // namespace carla
