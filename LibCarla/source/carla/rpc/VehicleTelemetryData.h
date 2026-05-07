// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#include "carla/rpc/WheelTelemetryData.h"

#include <vector>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "Carla/Vehicle/VehicleTelemetryData.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace rpc {

  // Vehicle-level runtime telemetry exposed to clients.
  //
  // Only fields backed by live Chaos solver state are kept. The PhysX-era
  // aerodynamic-drag readout was driven by UChaosVehicleMovementComponent::
  // DebugDragMagnitude, which the Chaos plugin declares but never writes;
  // exposing it on UE5 would always read zero, so it is omitted.
  class VehicleTelemetryData {
  public:

    VehicleTelemetryData() = default;

    VehicleTelemetryData(
        float speed,
        float steer,
        float throttle,
        float brake,
        float engine_rpm,
        int32_t gear,
        std::vector<WheelTelemetryData> wheels)
      : speed(speed),
        steer(steer),
        throttle(throttle),
        brake(brake),
        engine_rpm(engine_rpm),
        gear(gear),
        wheels(std::move(wheels)) {}

    float speed = 0.0f;
    float steer = 0.0f;
    float throttle = 0.0f;
    float brake = 0.0f;
    float engine_rpm = 0.0f;
    int32_t gear = 0;
    std::vector<WheelTelemetryData> wheels = {};

    const std::vector<WheelTelemetryData> &GetWheels() const {
      return wheels;
    }

    void SetWheels(std::vector<WheelTelemetryData> in_wheels) {
      wheels = std::move(in_wheels);
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    VehicleTelemetryData(const FVehicleTelemetryData &Data)
      : speed(Data.Speed),
        steer(Data.Steer),
        throttle(Data.Throttle),
        brake(Data.Brake),
        engine_rpm(Data.EngineRPM),
        gear(Data.Gear) {
      wheels.reserve(static_cast<size_t>(Data.Wheels.Num()));
      for (const auto &Wheel : Data.Wheels) {
        wheels.emplace_back(Wheel);
      }
    }

    operator FVehicleTelemetryData() const {
      FVehicleTelemetryData Data;
      Data.Speed = speed;
      Data.Steer = steer;
      Data.Throttle = throttle;
      Data.Brake = brake;
      Data.EngineRPM = engine_rpm;
      Data.Gear = gear;
      Data.Wheels.Reserve(static_cast<int32>(wheels.size()));
      for (const auto &Wheel : wheels) {
        Data.Wheels.Add(Wheel);
      }
      return Data;
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    bool operator!=(const VehicleTelemetryData &rhs) const {
      return
          speed != rhs.speed ||
          steer != rhs.steer ||
          throttle != rhs.throttle ||
          brake != rhs.brake ||
          engine_rpm != rhs.engine_rpm ||
          gear != rhs.gear ||
          wheels != rhs.wheels;
    }

    bool operator==(const VehicleTelemetryData &rhs) const {
      return !(*this != rhs);
    }

    MSGPACK_DEFINE_ARRAY(
        speed,
        steer,
        throttle,
        brake,
        engine_rpm,
        gear,
        wheels
    );
  };

} // namespace rpc
} // namespace carla