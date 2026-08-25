// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
#include "carla/MsgPack.h"
#include "carla/sensor/RawData.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace carla {
namespace sensor {

  class SensorData;

namespace s11n {

#pragma pack(push, 1)
  /// Snapshot of a vehicle's kinematic + control state, streamed each tick by
  /// the Autoware VehicleStatusSensor (UE side, next phase) and decoded by
  /// ROS2::ProcessDataFromStatusSensor into the six /vehicle/status/* report
  /// publishers. Ported from tier4/autoware-support.
  ///
  /// Bit masks:
  ///   control_flags: b0 = reverse, b1 = manual gear shift
  ///   turn_mask:     b0 = left blinker, b1 = right blinker, b2 = hazards
  struct VehicleStatusData {
    double timestamp;
    float speed_mps;
    float vel_x_mps, vel_y_mps, vel_z_mps;           // local
    float angVel_x_mps, angVel_y_mps, angVel_z_mps;  // local
    float rot_pitch, rot_yaw, rot_roll;              // relative
    float steer;
    int32_t gear;
    uint8_t turn_mask;
    uint8_t control_flags;

    MSGPACK_DEFINE_ARRAY(timestamp,
                         speed_mps,
                         vel_x_mps, vel_y_mps, vel_z_mps,
                         angVel_x_mps, angVel_y_mps, angVel_z_mps,
                         rot_pitch, rot_yaw, rot_roll,
                         steer,
                         gear,
                         turn_mask,
                         control_flags)
  };
#pragma pack(pop)

  class VehicleStatusSerializer {
  public:

    constexpr static auto header_offset = 0u;

    static VehicleStatusData DeserializeRawData(const RawData &message) {
      return MsgPack::UnPack<VehicleStatusData>(message.begin(), message.size());
    }

    /// Direct field-wise serialization (server side).
    template <typename SensorT>
    static Buffer Serialize(
        const SensorT &,
        double timestamp,
        float speed_mps,
        float vel_x_mps,
        float vel_y_mps,
        float vel_z_mps,
        float angVel_x_mps,
        float angVel_y_mps,
        float angVel_z_mps,
        float rot_pitch,
        float rot_yaw,
        float rot_roll,
        float steer,
        int32_t gear,
        uint8_t turn_mask,
        uint8_t control_flags) {
      return MsgPack::Pack(VehicleStatusData{
          timestamp,
          speed_mps,
          vel_x_mps, vel_y_mps, vel_z_mps,
          angVel_x_mps, angVel_y_mps, angVel_z_mps,
          rot_pitch, rot_yaw, rot_roll,
          steer,
          gear,
          turn_mask,
          control_flags});
    }

    /// Overload for the Unreal sensor streaming a packed struct. The UE-side
    /// packed layout carries two trailing pad bytes (4-byte struct alignment);
    /// everything before them matches VehicleStatusData byte-for-byte.
    template <typename SensorT>
    static Buffer Serialize(const SensorT &, const Buffer &buffer) {
      constexpr size_t packed_size = sizeof(VehicleStatusData) + 2u;  // + _pad0/_pad1
      if (buffer.size() != packed_size && buffer.size() != sizeof(VehicleStatusData)) {
        throw std::runtime_error("Invalid buffer size for VehicleStatusSensor");
      }
      VehicleStatusData msg;
      std::memcpy(&msg, buffer.data(), sizeof(VehicleStatusData));
      return MsgPack::Pack(msg);
    }

    static SharedPtr<SensorData> Deserialize(RawData &&data);
  };

} // namespace s11n
} // namespace sensor
} // namespace carla
