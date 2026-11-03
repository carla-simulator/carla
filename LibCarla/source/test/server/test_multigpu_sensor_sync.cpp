// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/multigpu/frameDataCodec.h>
#include <carla/multigpu/sensorSync.h>

#include <cstring>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using carla::multigpu::GetEngineState;
using carla::multigpu::SensorState;
using carla::multigpu::SensorStates;
using carla::multigpu::SensorTick;
using carla::multigpu::SensorTicks;
using carla::multigpu::SetEngineState;

namespace {

  /// The payload of the single packet in @a bytes, and its declared size.
  std::string Payload(const std::string &bytes, uint32_t &size) {
    std::memcpy(&size, bytes.data() + 1, sizeof(size));
    return bytes.substr(carla::multigpu::kFrameDataHeaderSize);
  }

} // namespace

TEST(MultiGpuSensorSync, ticks_round_trip_with_bit_exact_deltas) {
  SensorTicks sent;
  sent.ticks.push_back(SensorTick{7u, 0.05f});
  sent.ticks.push_back(SensorTick{9u, 0.1f + 1e-7f});
  std::ostringstream out;
  sent.Write(out);
  const std::string bytes = out.str();
  ASSERT_EQ(static_cast<uint8_t>(bytes[0]), SensorTicks::kPacketId);

  uint32_t size = 0u;
  std::istringstream in(Payload(bytes, size));
  SensorTicks got;
  ASSERT_TRUE(got.Read(in, size));
  ASSERT_EQ(got.ticks.size(), 2u);
  EXPECT_EQ(got.ticks[0].actor_id, 7u);
  EXPECT_EQ(got.ticks[0].delta_seconds, 0.05f);
  EXPECT_EQ(got.ticks[1].delta_seconds, 0.1f + 1e-7f);
}

TEST(MultiGpuSensorSync, empty_ticks_are_still_a_packet) {
  SensorTicks sent;
  std::ostringstream out;
  sent.Write(out);
  uint32_t size = 0u;
  std::istringstream in(Payload(out.str(), size));
  SensorTicks got;
  got.ticks.resize(3u);
  ASSERT_TRUE(got.Read(in, size));
  EXPECT_TRUE(got.ticks.empty());
}

TEST(MultiGpuSensorSync, states_round_trip_every_field) {
  SensorStates sent;
  SensorState lidar;
  lidar.actor_id = 3u;
  lidar.random_engine = 123456789u;
  lidar.lidar_horizontal_angle = 2.5f;
  SensorState imu;
  imu.actor_id = 4u;
  imu.random_engine = 42u;
  carla::multigpu::ImuHistory history;
  history.older_location = {1.0, 2.0, 3.0};
  history.last_location = {4.0, 5.0, 6.000001};
  history.last_delta_seconds = 0.05f;
  imu.imu_history = history;
  imu.imu_local_rotation = std::array<double, 4>{0.0, -0.25881904510252074, 0.0, 0.96592582628906831};
  SensorState radar;
  radar.actor_id = 5u;
  radar.radar_previous_location = std::array<double, 3>{-10.5, 20.25, 0.125};
  sent.states = {lidar, imu, radar};

  std::ostringstream out;
  sent.Write(out);
  uint32_t size = 0u;
  std::istringstream in(Payload(out.str(), size));
  SensorStates got;
  ASSERT_TRUE(got.Read(in, size));
  ASSERT_EQ(got.states.size(), 3u);
  EXPECT_EQ(got.states[0].random_engine, lidar.random_engine);
  EXPECT_EQ(got.states[0].lidar_horizontal_angle, lidar.lidar_horizontal_angle);
  EXPECT_FALSE(got.states[0].imu_history.has_value());
  ASSERT_TRUE(got.states[1].imu_history.has_value());
  EXPECT_EQ(got.states[1].imu_history->last_location[2], 6.000001);
  EXPECT_EQ(got.states[1].imu_history->last_delta_seconds, 0.05f);
  ASSERT_TRUE(got.states[1].imu_local_rotation.has_value());
  EXPECT_EQ(*got.states[1].imu_local_rotation, *imu.imu_local_rotation);
  EXPECT_FALSE(got.states[0].imu_local_rotation.has_value());
  ASSERT_TRUE(got.states[2].radar_previous_location.has_value());
  EXPECT_EQ((*got.states[2].radar_previous_location)[1], 20.25);
  EXPECT_FALSE(got.states[2].random_engine.has_value());
}

TEST(MultiGpuSensorSync, unknown_and_longer_fields_are_skipped) {
  // One record: id 8, an unknown kind 9, then a random engine field that grew.
  std::ostringstream record;
  const uint32_t id = 8u;
  record.write(reinterpret_cast<const char *>(&id), sizeof(id));
  carla::multigpu::WriteField(record, 9u, std::string("future"));
  std::string engine(sizeof(uint32_t) + 2u, '\0');
  const uint32_t state = 777u;
  std::memcpy(engine.data(), &state, sizeof(state));
  carla::multigpu::WriteField(record, SensorStates::kRandomEngine, engine);
  std::ostringstream payload;
  const uint32_t count = 1u;
  payload.write(reinterpret_cast<const char *>(&count), sizeof(count));
  carla::multigpu::WriteSizedBytes(payload, record.str());

  const std::string body = payload.str();
  std::istringstream in(body);
  SensorStates got;
  ASSERT_TRUE(got.Read(in, static_cast<uint32_t>(body.size())));
  ASSERT_EQ(got.states.size(), 1u);
  EXPECT_EQ(got.states[0].actor_id, 8u);
  EXPECT_EQ(got.states[0].random_engine, std::optional<uint32_t>(777u));
}

TEST(MultiGpuSensorSync, truncated_records_fail) {
  SensorStates sent;
  SensorState radar;
  radar.actor_id = 5u;
  radar.radar_previous_location = std::array<double, 3>{1.0, 2.0, 3.0};
  sent.states = {radar};
  std::ostringstream out;
  sent.Write(out);
  uint32_t size = 0u;
  const std::string payload = Payload(out.str(), size);
  const std::string cut = payload.substr(0u, payload.size() - 4u);
  std::istringstream in(cut);
  SensorStates got;
  EXPECT_FALSE(got.Read(in, static_cast<uint32_t>(cut.size())));
}

TEST(MultiGpuSensorSync, minstd_resumes_the_same_draws) {
  // A mirror created later continues the primary sensor's noise exactly.
  std::minstd_rand primary(1234u);
  for (int i = 0; i < 17; ++i) {
    (void)std::uniform_real_distribution<float>()(primary);
  }
  std::minstd_rand mirror(99u);
  ASSERT_TRUE(SetEngineState(mirror, GetEngineState(primary)));
  for (int i = 0; i < 50; ++i) {
    EXPECT_EQ(std::uniform_real_distribution<float>()(mirror), std::uniform_real_distribution<float>()(primary));
    EXPECT_EQ(std::normal_distribution<float>(0.0f, 2.0f)(mirror), std::normal_distribution<float>(0.0f, 2.0f)(primary));
    EXPECT_EQ(std::uniform_int_distribution<int32_t>(0, 1000)(mirror), std::uniform_int_distribution<int32_t>(0, 1000)(primary));
  }
}

TEST(MultiGpuSensorSync, impossible_engine_states_are_refused) {
  std::minstd_rand engine;
  EXPECT_FALSE(SetEngineState(engine, 0u));
  EXPECT_FALSE(SetEngineState(engine, 2147483647u));
  EXPECT_TRUE(SetEngineState(engine, 1u));
}
