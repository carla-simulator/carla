// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/sensor/SensorData.h"
#include "carla/sensor/s11n/VehicleStatusSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  /// Client-side view of one VehicleStatusSensor sample (Autoware vehicle
  /// interface). Ported from tier4/autoware-support.
  class VehicleStatusEvent : public SensorData {
    using Super = SensorData;
  protected:
    using Serializer = s11n::VehicleStatusSerializer;
    friend Serializer;

  public:
    explicit VehicleStatusEvent(const RawData &data)
      : Super(data),
        _parsed(Serializer::DeserializeRawData(data)) {}

    double GetStatusTimestamp() const { return _parsed.timestamp; }

    float GetSpeed() const { return _parsed.speed_mps; }
    float GetVelX() const { return _parsed.vel_x_mps; }  // local
    float GetVelY() const { return _parsed.vel_y_mps; }  // local
    float GetVelZ() const { return _parsed.vel_z_mps; }  // local

    float GetAngVelX() const { return _parsed.angVel_x_mps; }  // local
    float GetAngVelY() const { return _parsed.angVel_y_mps; }  // local
    float GetAngVelZ() const { return _parsed.angVel_z_mps; }  // local

    float GetRotPitch() const { return _parsed.rot_pitch; }  // relative
    float GetRotYaw() const { return _parsed.rot_yaw; }      // relative
    float GetRotRoll() const { return _parsed.rot_roll; }    // relative

    float GetSteer() const { return _parsed.steer; }
    int32_t GetGear() const { return _parsed.gear; }
    uint8_t GetTurnMask() const { return _parsed.turn_mask; }
    uint8_t GetControlFlags() const { return _parsed.control_flags; }

  private:
    s11n::VehicleStatusData _parsed;
  };

} // namespace data
} // namespace sensor
} // namespace carla
