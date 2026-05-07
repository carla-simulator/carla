// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/MsgPack.h>
#include <carla/rpc/VehicleTelemetryData.h>
#include <carla/rpc/WheelTelemetryData.h>

#include <vector>

using carla::rpc::VehicleTelemetryData;
using carla::rpc::WheelTelemetryData;

// VehicleTelemetryData and WheelTelemetryData are POD-style RPC types crossing
// the simulator/client boundary via MsgPack. These tests pin two contracts:
// 1) default construction zero-initializes every field, so a fresh instance
//    is safe to inspect or send;
// 2) every field round-trips through MsgPack pack/unpack unchanged, including
//    the nested wheels vector.

TEST(WheelTelemetryData, default_construction_zero_initialises_all_fields) {
  WheelTelemetryData wheel{};
  EXPECT_EQ(wheel.lat_slip, 0.0f);
  EXPECT_EQ(wheel.long_slip, 0.0f);
  EXPECT_EQ(wheel.omega, 0.0f);
}

TEST(WheelTelemetryData, msgpack_roundtrip_preserves_every_field) {
  WheelTelemetryData wheel{1.25f, 0.42f, 31.7f};

  auto buffer = carla::MsgPack::Pack(wheel);
  auto restored = carla::MsgPack::UnPack<WheelTelemetryData>(buffer);

  EXPECT_EQ(restored, wheel);
}

TEST(VehicleTelemetryData, default_construction_zero_initialises_all_fields) {
  VehicleTelemetryData telemetry{};
  EXPECT_EQ(telemetry.speed, 0.0f);
  EXPECT_EQ(telemetry.steer, 0.0f);
  EXPECT_EQ(telemetry.throttle, 0.0f);
  EXPECT_EQ(telemetry.brake, 0.0f);
  EXPECT_EQ(telemetry.engine_rpm, 0.0f);
  EXPECT_EQ(telemetry.gear, 0);
  EXPECT_TRUE(telemetry.wheels.empty());
}

TEST(VehicleTelemetryData, msgpack_roundtrip_preserves_vehicle_and_wheel_fields) {
  std::vector<WheelTelemetryData> wheels{
      WheelTelemetryData{1.0f, 0.1f, 25.0f},
      WheelTelemetryData{2.0f, 0.2f, 26.0f},
      WheelTelemetryData{3.0f, 0.3f, 27.0f}};

  VehicleTelemetryData telemetry{
      14.5f, 0.12f, 0.8f, 0.0f, 3500.0f, 4, std::move(wheels)};

  auto buffer = carla::MsgPack::Pack(telemetry);
  auto restored = carla::MsgPack::UnPack<VehicleTelemetryData>(buffer);

  EXPECT_EQ(restored.speed, telemetry.speed);
  EXPECT_EQ(restored.steer, telemetry.steer);
  EXPECT_EQ(restored.throttle, telemetry.throttle);
  EXPECT_EQ(restored.brake, telemetry.brake);
  EXPECT_EQ(restored.engine_rpm, telemetry.engine_rpm);
  EXPECT_EQ(restored.gear, telemetry.gear);
  ASSERT_EQ(restored.wheels.size(), telemetry.wheels.size());
  for (size_t i = 0; i < telemetry.wheels.size(); ++i) {
    EXPECT_EQ(restored.wheels[i], telemetry.wheels[i]);
  }
  EXPECT_EQ(restored, telemetry);
}