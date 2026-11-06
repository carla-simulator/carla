// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/multigpu/frameDataCodec.h>
#include <carla/multigpu/mirrorPackets.h>

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using carla::multigpu::MirroredPosition;
using carla::multigpu::MirroredPositions;
using carla::multigpu::OrderParentsFirst;

namespace {

  /// Reads back the payload of the single packet in @a bytes.
  bool ReadPositions(const std::string &bytes, MirroredPositions &out) {
    uint32_t size = 0u;
    std::memcpy(&size, bytes.data() + 1, sizeof(size));
    std::istringstream in(bytes.substr(carla::multigpu::kFrameDataHeaderSize));
    return out.Read(in, size);
  }

} // namespace

TEST(MultiGpuMirror, positions_round_trip_bit_exact) {
  MirroredPositions sent;
  MirroredPosition camera;
  camera.id = 7u;
  camera.location[0] = 12345.678901234;
  camera.location[1] = -0.000001;
  camera.location[2] = 250.0;
  // Looking straight down: the pose an Euler round trip cannot keep.
  camera.rotation[0] = 0.0;
  camera.rotation[1] = 0.70710678118654757;
  camera.rotation[2] = 0.0;
  camera.rotation[3] = 0.70710678118654746;
  sent.positions.push_back(camera);
  MirroredPosition other;
  other.id = 9u;
  sent.positions.push_back(other);

  std::ostringstream out;
  sent.Write(out);
  const std::string bytes = out.str();
  ASSERT_EQ(static_cast<uint8_t>(bytes[0]), MirroredPositions::kPacketId);
  ASSERT_EQ(bytes.size(), carla::multigpu::kFrameDataHeaderSize + 6u + 2u * MirroredPositions::kRecordSizeV1);

  MirroredPositions got;
  ASSERT_TRUE(ReadPositions(bytes, got));
  ASSERT_EQ(got.positions.size(), 2u);
  EXPECT_EQ(got.positions[0].id, 7u);
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(got.positions[0].location[i], camera.location[i]);
  }
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(got.positions[0].rotation[i], camera.rotation[i]);
  }
  EXPECT_EQ(got.positions[1].id, 9u);
}

TEST(MultiGpuMirror, positions_from_a_newer_build_keep_their_known_fields) {
  // Records of 64 bytes: v1's 60 plus a field this build does not know.
  std::ostringstream payload;
  const uint32_t count = 2u;
  const uint16_t record_size = 64u;
  payload.write(reinterpret_cast<const char *>(&count), sizeof(count));
  payload.write(reinterpret_cast<const char *>(&record_size), sizeof(record_size));
  for (uint32_t id = 1u; id <= count; ++id) {
    payload.write(reinterpret_cast<const char *>(&id), sizeof(id));
    const double values[7] = {double(id), 2.0, 3.0, 0.0, 0.0, 0.0, 1.0};
    payload.write(reinterpret_cast<const char *>(values), sizeof(values));
    const uint32_t extra = 0xFFFFFFFFu;
    payload.write(reinterpret_cast<const char *>(&extra), sizeof(extra));
  }
  const std::string body = payload.str();
  std::istringstream in(body);
  MirroredPositions got;
  ASSERT_TRUE(got.Read(in, static_cast<uint32_t>(body.size())));
  ASSERT_EQ(got.positions.size(), 2u);
  EXPECT_EQ(got.positions[1].id, 2u);
  EXPECT_EQ(got.positions[1].location[0], 2.0);
}

TEST(MultiGpuMirror, truncated_positions_fail) {
  MirroredPositions sent;
  sent.positions.resize(3u);
  std::ostringstream out;
  sent.Write(out);
  const std::string bytes = out.str();
  const std::string body = bytes.substr(carla::multigpu::kFrameDataHeaderSize, 6u + 100u);
  std::istringstream in(body);
  MirroredPositions got;
  EXPECT_FALSE(got.Read(in, static_cast<uint32_t>(body.size())));
}

TEST(MultiGpuMirror, order_parents_first_depth_two) {
  // 3 is attached to 2, which is attached to 1; 4 is a root.
  const std::vector<uint32_t> ids = {3u, 2u, 4u, 1u};
  const std::unordered_map<uint32_t, uint32_t> parents = {{3u, 2u}, {2u, 1u}};
  const std::vector<uint32_t> order = OrderParentsFirst(ids, parents);
  EXPECT_EQ(order, (std::vector<uint32_t>{4u, 1u, 2u, 3u}));
}

TEST(MultiGpuMirror, missing_parent_and_cycle_are_guarded) {
  // 5's parent is not in the frame; 6 and 7 are each other's parent.
  const std::vector<uint32_t> ids = {5u, 6u, 7u, 8u};
  const std::unordered_map<uint32_t, uint32_t> parents = {{5u, 99u}, {6u, 7u}, {7u, 6u}, {8u, 7u}};
  const std::vector<uint32_t> order = OrderParentsFirst(ids, parents);
  ASSERT_EQ(order.size(), ids.size());
  // Every id once; the actor attached to the cycle comes after it.
  const auto pos = [&](uint32_t id) {
    return std::find(order.begin(), order.end(), id) - order.begin();
  };
  for (uint32_t id : ids) {
    EXPECT_EQ(std::count(order.begin(), order.end(), id), 1);
  }
  EXPECT_LT(pos(7u), pos(8u));
}

namespace {

  bool ReadParts(const std::string &bytes, carla::multigpu::MirroredVehicleParts &out) {
    uint32_t size = 0u;
    std::memcpy(&size, bytes.data() + 1, sizeof(size));
    std::istringstream in(bytes.substr(carla::multigpu::kFrameDataHeaderSize));
    return out.Read(in, size);
  }

} // namespace

TEST(MultiGpuMirror, vehicle_parts_round_trip) {
  carla::multigpu::MirroredVehicleParts sent;
  carla::multigpu::VehicleParts car;
  car.actor_id = 21u;
  car.wheels = {{12.5f, 361.25f, 3.5f}, {-12.5f, 0.125f, -1.0f}, {0.0f, 90.0f, 0.0f}, {0.0f, 180.0f, 2.25f}};
  carla::multigpu::DoorPose door;
  door.door = 1u;
  door.location = {10.0, -85.5, 40.25};
  door.rotation = {0.0, 0.0, 0.38268343236508978, 0.92387953251128674};
  car.open_doors = {door};
  car.current_gear = -1;
  car.manual_gear_shift = true;
  car.simulating_physics = true;
  carla::multigpu::VehicleParts bike;
  bike.actor_id = 22u;
  bike.wheels = {{5.0f, 10.0f, 0.5f}, {0.0f, 10.0f, 0.25f}};
  sent.vehicles = {car, bike};

  std::ostringstream out;
  sent.Write(out);
  const std::string bytes = out.str();
  ASSERT_EQ(static_cast<uint8_t>(bytes[0]), carla::multigpu::MirroredVehicleParts::kPacketId);
  carla::multigpu::MirroredVehicleParts got;
  ASSERT_TRUE(ReadParts(bytes, got));
  ASSERT_EQ(got.vehicles.size(), 2u);
  ASSERT_EQ(got.vehicles[0].wheels.size(), 4u);
  EXPECT_EQ(got.vehicles[0].wheels[1].steer, -12.5f);
  EXPECT_EQ(got.vehicles[0].wheels[0].spin, 361.25f);
  EXPECT_EQ(got.vehicles[0].wheels[3].suspension_offset, 2.25f);
  ASSERT_EQ(got.vehicles[0].open_doors.size(), 1u);
  EXPECT_EQ(got.vehicles[0].open_doors[0].door, 1u);
  EXPECT_EQ(got.vehicles[0].open_doors[0].location, door.location);
  EXPECT_EQ(got.vehicles[0].open_doors[0].rotation, door.rotation);
  EXPECT_EQ(got.vehicles[0].current_gear, std::optional<int32_t>(-1));
  EXPECT_TRUE(got.vehicles[0].manual_gear_shift);
  EXPECT_TRUE(got.vehicles[0].simulating_physics);
  EXPECT_EQ(got.vehicles[1].actor_id, 22u);
  EXPECT_EQ(got.vehicles[1].wheels.size(), 2u);
  EXPECT_TRUE(got.vehicles[1].open_doors.empty());
  EXPECT_FALSE(got.vehicles[1].current_gear.has_value());
}

TEST(MultiGpuMirror, vehicle_parts_from_a_newer_build_keep_their_known_fields) {
  // One vehicle: wheel records of 16 bytes (v1's 12 and a new float) and an
  // unknown field.
  std::ostringstream wheels;
  const uint32_t count = 1u;
  const uint16_t record_size = 16u;
  wheels.write(reinterpret_cast<const char *>(&count), sizeof(count));
  wheels.write(reinterpret_cast<const char *>(&record_size), sizeof(record_size));
  const float values[4] = {1.0f, 2.0f, 3.0f, 99.0f};
  wheels.write(reinterpret_cast<const char *>(values), sizeof(values));
  std::ostringstream record;
  const uint32_t id = 5u;
  record.write(reinterpret_cast<const char *>(&id), sizeof(id));
  carla::multigpu::WriteField(record, 9u, std::string("future"));
  carla::multigpu::WriteField(record, carla::multigpu::MirroredVehicleParts::kWheels, wheels.str());
  std::ostringstream payload;
  payload.write(reinterpret_cast<const char *>(&count), sizeof(count));
  carla::multigpu::WriteSizedBytes(payload, record.str());

  const std::string body = payload.str();
  std::istringstream in(body);
  carla::multigpu::MirroredVehicleParts got;
  ASSERT_TRUE(got.Read(in, static_cast<uint32_t>(body.size())));
  ASSERT_EQ(got.vehicles.size(), 1u);
  ASSERT_EQ(got.vehicles[0].wheels.size(), 1u);
  EXPECT_EQ(got.vehicles[0].wheels[0].suspension_offset, 3.0f);
}

TEST(MultiGpuMirror, truncated_vehicle_parts_fail) {
  carla::multigpu::MirroredVehicleParts sent;
  carla::multigpu::VehicleParts car;
  car.actor_id = 1u;
  car.wheels.resize(4u);
  car.open_doors.resize(2u);
  sent.vehicles = {car};
  std::ostringstream out;
  sent.Write(out);
  const std::string bytes = out.str();
  const std::string body = bytes.substr(carla::multigpu::kFrameDataHeaderSize, bytes.size() - carla::multigpu::kFrameDataHeaderSize - 10u);
  std::istringstream in(body);
  carla::multigpu::MirroredVehicleParts got;
  EXPECT_FALSE(got.Read(in, static_cast<uint32_t>(body.size())));
}
