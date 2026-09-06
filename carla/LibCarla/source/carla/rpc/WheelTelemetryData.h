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

  // Per-wheel runtime telemetry exposed to clients.
  //
  // Only the fields that the Chaos vehicle solver populates live are kept here.
  // The PhysX-era surface (tire_friction, tire_load, normalized_tire_load,
  // torque, long_force, lat_force, normalized_long_force, normalized_lat_force)
  // is intentionally absent: those values were read from UVehicleWheel debug
  // members that the Chaos plugin declares but never writes, so reporting them
  // on UE5 would produce permanent zeros.
  class WheelTelemetryData {
  public:

    WheelTelemetryData() = default;

    WheelTelemetryData(
        float lat_slip,
        float long_slip,
        float omega)
      : lat_slip(lat_slip),
        long_slip(long_slip),
        omega(omega) {}

    float lat_slip = 0.0f;
    float long_slip = 0.0f;
    float omega = 0.0f;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    WheelTelemetryData(const FWheelTelemetryData &Data)
      : lat_slip(Data.LatSlip),
        long_slip(Data.LongSlip),
        omega(Data.Omega) {}

    operator FWheelTelemetryData() const {
      FWheelTelemetryData Data;
      Data.LatSlip = lat_slip;
      Data.LongSlip = long_slip;
      Data.Omega = omega;
      return Data;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const WheelTelemetryData &rhs) const {
      return
          lat_slip != rhs.lat_slip ||
          long_slip != rhs.long_slip ||
          omega != rhs.omega;
    }

    bool operator==(const WheelTelemetryData &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        lat_slip,
        long_slip,
        omega
    );
  };

} // namespace rpc
} // namespace carla