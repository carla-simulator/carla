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
    float omega,
    float tire_load,
    float normalized_tire_load,
    float torque,
    float long_force,
    float lat_force,
    float normalized_long_force,
    float normalized_lat_force)
      : tire_friction(tire_friction),
        lat_slip(lat_slip),
        long_slip(long_slip),
        omega(omega),
        tire_load(tire_load),
        normalized_tire_load(normalized_tire_load),
        torque(torque),
        long_force(long_force),
        lat_force(lat_force),
        normalized_long_force(normalized_long_force),
        normalized_lat_force(normalized_lat_force) {}

    float tire_friction = 0.0f;
    float lat_slip = 0.0f;
    float long_slip = 0.0f;
    float omega = 0.0f;
    float tire_load = 0.0f;
    float normalized_tire_load = 0.0f;
    float torque = 0.0f;
    float long_force = 0.0f;
    float lat_force = 0.0f;
    float normalized_long_force = 0.0f;
    float normalized_lat_force = 0.0f;

    bool operator!=(const WheelTelemetryData &rhs) const {
      return
      tire_friction != rhs.tire_friction ||
      lat_slip != rhs.lat_slip ||
      long_slip != rhs.long_slip ||
      omega != rhs.omega ||
      tire_load != rhs.tire_load ||
      normalized_tire_load != rhs.normalized_tire_load ||
      torque != rhs.torque ||
      long_force != rhs.long_force ||
      lat_force != rhs.lat_force ||
      normalized_long_force != rhs.normalized_long_force ||
      normalized_lat_force != rhs.normalized_lat_force;
    }

    bool operator==(const WheelTelemetryData &rhs) const {
      return !(*this != rhs);
    }
#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelTelemetryData(const FWheelTelemetryData &TelemetryData)
      : tire_friction(TelemetryData.TireFriction),
        lat_slip(TelemetryData.LatSlip),
        long_slip(TelemetryData.LongSlip),
        omega(TelemetryData.Omega),
        tire_load(TelemetryData.TireLoad),
        normalized_tire_load(TelemetryData.NormalizedTireLoad),
        torque(TelemetryData.Torque),
        long_force(TelemetryData.LongForce),
        lat_force(TelemetryData.LatForce),
        normalized_long_force(TelemetryData.NormalizedLongForce),
        normalized_lat_force(TelemetryData.NormalizedLatForce) {}

    operator FWheelTelemetryData() const {
      FWheelTelemetryData TelemetryData;
      TelemetryData.TireFriction = tire_friction;
      TelemetryData.LatSlip = lat_slip;
      TelemetryData.LongSlip = long_slip;
      TelemetryData.Omega = omega;
      TelemetryData.TireLoad = tire_load;
      TelemetryData.NormalizedTireLoad = normalized_tire_load;
      TelemetryData.Torque = torque;
      TelemetryData.LongForce = long_force;
      TelemetryData.LatForce = lat_force;
      TelemetryData.NormalizedLongForce = normalized_long_force;
      TelemetryData.NormalizedLatForce = normalized_lat_force;

      return TelemetryData;
    }
#endif

    MSGPACK_DEFINE_ARRAY(tire_friction,
    lat_slip,
    long_slip,
    omega,
    tire_load,
    normalized_tire_load,
    torque,
    long_force,
    lat_force,
    normalized_long_force,
    normalized_lat_force)
  };

} // namespace rpc
} // namespace carla
