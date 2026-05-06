// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/VehicleTelemetryData.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

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

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelTelemetryData(const FWheelTelemetryData &Data)
      : tire_friction(Data.TireFriction),
        lat_slip(Data.LatSlip),
        long_slip(Data.LongSlip),
        omega(Data.Omega),
        tire_load(Data.TireLoad),
        normalized_tire_load(Data.NormalizedTireLoad),
        torque(Data.Torque),
        long_force(Data.LongForce),
        lat_force(Data.LatForce),
        normalized_long_force(Data.NormalizedLongForce),
        normalized_lat_force(Data.NormalizedLatForce) {}

    operator FWheelTelemetryData() const {
      FWheelTelemetryData Data;
      Data.TireFriction = tire_friction;
      Data.LatSlip = lat_slip;
      Data.LongSlip = long_slip;
      Data.Omega = omega;
      Data.TireLoad = tire_load;
      Data.NormalizedTireLoad = normalized_tire_load;
      Data.Torque = torque;
      Data.LongForce = long_force;
      Data.LatForce = lat_force;
      Data.NormalizedLongForce = normalized_long_force;
      Data.NormalizedLatForce = normalized_lat_force;
      return Data;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

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

    MSGPACK_DEFINE_ARRAY(
        tire_friction,
        lat_slip,
        long_slip,
        omega,
        tire_load,
        normalized_tire_load,
        torque,
        long_force,
        lat_force,
        normalized_long_force,
        normalized_lat_force
    );
  };

} // namespace rpc
} // namespace carla
