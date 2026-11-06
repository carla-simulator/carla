// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/multigpu/frameDataCodec.h>
#include <carla/multigpu/worldState.h>

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

using carla::multigpu::ActorAppearance;
using carla::multigpu::ActorAppearances;
using carla::multigpu::ComputeMapLayerDiff;
using carla::multigpu::DiffSortedIds;
using carla::multigpu::WorldGlobals;
using carla::multigpu::WorldObjects;

namespace {

  template <typename Packet>
  bool RoundTrip(const Packet &sent, Packet &got) {
    std::ostringstream out;
    sent.Write(out);
    const std::string bytes = out.str();
    if (static_cast<uint8_t>(bytes[0]) != Packet::kPacketId) {
      return false;
    }
    uint32_t size = 0u;
    std::memcpy(&size, bytes.data() + 1, sizeof(size));
    std::istringstream in(bytes.substr(carla::multigpu::kFrameDataHeaderSize));
    return got.Read(in, size);
  }

} // namespace

TEST(MultiGpuWorldState, globals_round_trip) {
  WorldGlobals sent;
  sent.map_layers = 0x0005u;
  sent.imu_gravity = 3.71f;
  sent.day_night_cycle = false;
  sent.has_day_time_state = true;
  sent.last_is_day = false;
  sent.publish_tf = false;
  WorldGlobals got;
  ASSERT_TRUE(RoundTrip(sent, got));
  EXPECT_EQ(got.map_layers, 0x0005u);
  EXPECT_EQ(got.imu_gravity, 3.71f);
  EXPECT_FALSE(got.day_night_cycle);
  EXPECT_TRUE(got.has_day_time_state);
  EXPECT_FALSE(got.last_is_day);
  EXPECT_FALSE(got.publish_tf);
}

TEST(MultiGpuWorldState, objects_round_trip) {
  WorldObjects sent;
  sent.disabled_environment_objects = std::vector<uint64_t>{3u, 99u, 0xFFFFFFFFFFull};
  WorldObjects got;
  ASSERT_TRUE(RoundTrip(sent, got));
  ASSERT_TRUE(got.disabled_environment_objects.has_value());
  EXPECT_EQ(*got.disabled_environment_objects, *sent.disabled_environment_objects);
}

TEST(MultiGpuWorldState, an_empty_set_is_not_an_absent_one) {
  // Everything enabled again must reach the secondaries as an empty set.
  WorldObjects sent;
  sent.disabled_environment_objects = std::vector<uint64_t>{};
  WorldObjects got;
  ASSERT_TRUE(RoundTrip(sent, got));
  ASSERT_TRUE(got.disabled_environment_objects.has_value());
  EXPECT_TRUE(got.disabled_environment_objects->empty());

  WorldObjects none;
  ASSERT_TRUE(RoundTrip(WorldObjects{}, none));
  EXPECT_FALSE(none.disabled_environment_objects.has_value());
}

TEST(MultiGpuWorldState, objects_with_an_unknown_field_keep_the_set) {
  std::ostringstream payload;
  carla::multigpu::WriteField(payload, 7u, std::string("future"));
  std::ostringstream ids;
  const uint32_t count = 1u;
  const uint64_t id = 42u;
  ids.write(reinterpret_cast<const char *>(&count), sizeof(count));
  ids.write(reinterpret_cast<const char *>(&id), sizeof(id));
  carla::multigpu::WriteField(payload, WorldObjects::kDisabledEnvironmentObjects, ids.str());
  const std::string body = payload.str();
  std::istringstream in(body);
  WorldObjects got;
  ASSERT_TRUE(got.Read(in, static_cast<uint32_t>(body.size())));
  ASSERT_TRUE(got.disabled_environment_objects.has_value());
  EXPECT_EQ(*got.disabled_environment_objects, std::vector<uint64_t>{42u});
}

TEST(MultiGpuWorldState, an_impossible_object_count_fails) {
  std::ostringstream payload;
  std::ostringstream ids;
  const uint32_t count = 1000000u;
  ids.write(reinterpret_cast<const char *>(&count), sizeof(count));
  carla::multigpu::WriteField(payload, WorldObjects::kDisabledEnvironmentObjects, ids.str());
  const std::string body = payload.str();
  std::istringstream in(body);
  WorldObjects got;
  EXPECT_FALSE(got.Read(in, static_cast<uint32_t>(body.size())));
}

TEST(MultiGpuWorldState, appearance_round_trip) {
  ActorAppearances sent;
  ActorAppearance prop;
  prop.actor_id = 12u;
  prop.scale = std::array<double, 3>{2.0, 2.0, 0.5};
  prop.collisions = false;
  ActorAppearance hidden;
  hidden.actor_id = 13u;
  hidden.hidden = true;
  sent.actors = {prop, hidden};
  ActorAppearances got;
  ASSERT_TRUE(RoundTrip(sent, got));
  ASSERT_EQ(got.actors.size(), 2u);
  EXPECT_EQ(got.actors[0].scale, prop.scale);
  EXPECT_EQ(got.actors[0].collisions, std::optional<bool>(false));
  EXPECT_FALSE(got.actors[0].hidden.has_value());
  EXPECT_EQ(got.actors[1].hidden, std::optional<bool>(true));
}

TEST(MultiGpuWorldState, layer_diff) {
  const auto diff = ComputeMapLayerDiff(0x00FFu, 0x0F0Fu);
  EXPECT_EQ(diff.first, 0x0F00u);
  EXPECT_EQ(diff.second, 0x00F0u);
  EXPECT_EQ(ComputeMapLayerDiff(0x1234u, 0x1234u), (std::pair<uint16_t, uint16_t>{0u, 0u}));
}

TEST(MultiGpuWorldState, diff_sorted_ids) {
  const auto diff = DiffSortedIds({1u, 3u, 5u}, {3u, 4u, 5u, 6u});
  EXPECT_EQ(diff.first, (std::vector<uint64_t>{4u, 6u}));
  EXPECT_EQ(diff.second, (std::vector<uint64_t>{1u}));
}
