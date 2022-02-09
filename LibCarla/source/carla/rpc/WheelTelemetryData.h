// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  class WheelTelemetryData {
  public:

    WheelTelemetryData() = default;

    WheelTelemetryData(
    float tire_friction,
    float lat_slip,
    float long_slip,
    float rpm,
    float normalized_load,
    float torque,
    float long_force,
    float lat_force)
      : tire_friction(tire_friction),
        lat_slip(lat_slip),
        long_slip(long_slip),
        rpm(rpm),
        normalized_load(normalized_load),
        torque(torque),
        long_force(long_force),
        lat_force(lat_force) {}

    float tire_friction = 0.0f;
    float lat_slip = 0.0f;
    float long_slip = 0.0f;
    float rpm = 0.0f;
    float normalized_load = 0.0f;
    float torque = 0.0f;
    float long_force = 0.0f;
    float lat_force = 0.0f;

    bool operator!=(const WheelTelemetryData &rhs) const {
      return
      tire_friction != rhs.tire_friction ||
      lat_slip != rhs.lat_slip ||
      long_slip != rhs.long_slip ||
      rpm != rhs.rpm ||
      normalized_load != rhs.normalized_load ||
      torque != rhs.torque ||
      long_force != rhs.long_force ||
      lat_force != rhs.lat_force;
    }

    bool operator==(const WheelTelemetryData &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelTelemetryData(const FWheelTelemetryData &TelemetryData)
      : tire_friction(TelemetryData.TireFriction),
        lat_slip(TelemetryData.LatSlip),
        long_slip(TelemetryData.LongSlip),
        rpm(TelemetryData.RPM),
        normalized_load(TelemetryData.NormalizedLoad),
        torque(TelemetryData.Torque),
        long_force(TelemetryData.LongForce),
        lat_force(TelemetryData.LatForce) {}

    operator FWheelTelemetryData() const {
      FWheelTelemetryData TelemetryData;
      TelemetryData.TireFriction = tire_friction;
      TelemetryData.LatSlip = lat_slip;
      TelemetryData.LongSlip = long_slip;
      TelemetryData.RPM = rpm;
      TelemetryData.NormalizedLoad = normalized_load;
      TelemetryData.Torque = torque;
      TelemetryData.LongForce = long_force;
      TelemetryData.LatForce = lat_force;

      return TelemetryData;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
    lat_slip,
    long_slip,
    rpm,
    normalized_load,
    torque,
    long_force,
    lat_force)
  };

} // namespace rpc
} // namespace carla
