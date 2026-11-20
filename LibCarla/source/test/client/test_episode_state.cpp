// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/client/WorldSnapshot.h>
#include <carla/client/detail/EpisodeState.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/ActorDynamicState.h>
#include <carla/sensor/data/RawEpisodeState.h>
#include <carla/sensor/s11n/EpisodeStateSerializer.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>

#include <cstring>
#include <memory>
#include <vector>

namespace {

  using carla::sensor::data::ActorDynamicState;
  using carla::client::detail::EpisodeState;

  struct Actor {
    carla::ActorId id;
    double location_cm[3u];
    float yaw;
  };

  /// The bytes the server sends for one frame, as FWorldObserver writes them.
  std::shared_ptr<EpisodeState> MakeState(const std::vector<Actor> &actors) {
    namespace s11n = carla::sensor::s11n;
    s11n::SensorHeaderSerializer::Header sensor_header{};
    sensor_header.sensor_type =
        carla::sensor::SensorRegistry::get<FWorldObserver *>::index;
    sensor_header.frame = 42u;
    sensor_header.timestamp = 2.5;
    s11n::EpisodeStateSerializer::Header episode_header{};
    episode_header.episode_id = 7u;
    episode_header.delta_seconds = 0.05f;

    std::vector<unsigned char> bytes;
    auto append = [&bytes](const auto &value) {
      const auto *begin = reinterpret_cast<const unsigned char *>(&value);
      bytes.insert(bytes.end(), begin, begin + sizeof(value));
    };
    append(sensor_header);
    append(episode_header);
    for (const auto &actor : actors) {
      ActorDynamicState state;
      std::memset(&state, 0, sizeof(state));
      state.id = actor.id;
      state.actor_state = carla::rpc::ActorState::Active;
      std::memcpy(state.location, actor.location_cm, sizeof(state.location));
      state.rotation = carla::geom::Rotation{0.0f, actor.yaw, 0.0f};
      state.velocity = carla::geom::Vector3D{float(actor.id), 0.0f, 0.0f};
      append(state);
    }
    carla::Buffer buffer(bytes.data(), bytes.size());
    auto data = carla::sensor::SensorRegistry::Deserialize(std::move(buffer));
    auto raw = std::static_pointer_cast<carla::sensor::data::RawEpisodeState>(data);
    return std::make_shared<EpisodeState>(*raw);
  }

} // namespace

TEST(episode_state, actors_are_sorted_by_id_and_found) {
  const std::vector<Actor> actors = {
      {30u, {100.0, 0.0, 0.0}, 3.0f},
      {10u, {200.0, 0.0, 0.0}, 1.0f},
      {25u, {300.0, 0.0, 0.0}, 2.5f},
      {11u, {400.0, 0.0, 0.0}, 1.1f}};
  const auto state = MakeState(actors);

  ASSERT_EQ(state->size(), 4u);
  ASSERT_EQ(state->GetFrame(), 42u);
  ASSERT_EQ(state->GetEpisodeId(), 7u);

  const std::vector<carla::ActorId> expected_ids = {10u, 11u, 25u, 30u};
  const auto ids = state->GetActorIds();
  ASSERT_EQ(std::vector<carla::ActorId>(ids.begin(), ids.end()), expected_ids);

  size_t index = 0u;
  for (const auto &snapshot : *state) {
    ASSERT_EQ(snapshot.id, expected_ids[index]);
    ASSERT_EQ(snapshot.velocity.x, float(snapshot.id));
    ++index;
  }

  for (const auto &actor : actors) {
    ASSERT_TRUE(state->ContainsActorSnapshot(actor.id));
    const auto found = state->GetActorSnapshotIfPresent(actor.id);
    ASSERT_TRUE(found.has_value());
    ASSERT_EQ(found->id, actor.id);
    ASSERT_EQ(found->transform.rotation.yaw, actor.yaw);
    const auto at = state->FindIndex(actor.id);
    ASSERT_TRUE(at.has_value());
    ASSERT_EQ(state->GetActorSnapshotAt(*at).id, actor.id);
  }

  for (const carla::ActorId missing : {0u, 12u, 26u, 31u}) {
    ASSERT_FALSE(state->ContainsActorSnapshot(missing));
    ASSERT_FALSE(state->GetActorSnapshotIfPresent(missing).has_value());
    ASSERT_FALSE(state->FindIndex(missing).has_value());
    ASSERT_EQ(state->GetActorSnapshot(missing).id, 0u);
  }
}

TEST(episode_state, empty_frame) {
  const auto state = MakeState({});
  ASSERT_EQ(state->size(), 0u);
  ASSERT_EQ(state->begin(), state->end());
  ASSERT_FALSE(state->FindIndex(1u).has_value());
}

TEST(episode_state, locations_keep_double_far_from_the_origin) {
  // Town13 reaches about 7 km from the origin; a float in meters there has a
  // resolution of about 0.5 mm.
  const double x_cm = 700012.3456789012;
  const double y_cm = -650098.7654321098;
  const double z_cm = 1234.5678901234;
  const auto state = MakeState({{5u, {x_cm, y_cm, z_cm}, 90.0f}});
  const auto index = state->FindIndex(5u);
  ASSERT_TRUE(index.has_value());

  // The double location is Unreal's, converted to meters in double.
  const auto &location = state->GetLocationAt(*index);
  ASSERT_EQ(location[0u], x_cm / 1e2);
  ASSERT_EQ(location[1u], y_cm / 1e2);
  ASSERT_EQ(location[2u], z_cm / 1e2);

  // The float location is rounded as geom::Location(FVector) did on the
  // server before: the per-actor API returns the same values as before.
  const auto &transform = state->GetActorSnapshotAt(*index).transform;
  ASSERT_EQ(transform.location.x, static_cast<float>(1e-2f * x_cm));
  ASSERT_EQ(transform.location.y, static_cast<float>(1e-2f * y_cm));
  ASSERT_EQ(transform.location.z, static_cast<float>(1e-2f * z_cm));

  // And the float loses what the double keeps.
  ASSERT_NE(static_cast<double>(transform.location.x), location[0u]);
}

TEST(episode_state, world_snapshot_reads_the_same_state) {
  const auto state = MakeState({{3u, {1.0, 2.0, 3.0}, 0.0f}, {1u, {4.0, 5.0, 6.0}, 0.0f}});
  const carla::client::WorldSnapshot snapshot(state);
  ASSERT_EQ(&snapshot.GetEpisodeState(), state.get());
  ASSERT_EQ(snapshot.size(), 2u);
  ASSERT_TRUE(snapshot.Find(3u).has_value());
  ASSERT_FALSE(snapshot.Find(2u).has_value());
  ASSERT_EQ(snapshot.begin()->id, 1u);
}

TEST(episode_state, sparse_ids_are_sorted_and_found) {
  // Too far apart for the slot table: sorted and binary-searched instead.
  const std::vector<Actor> actors = {
      {4000000u, {1.0, 0.0, 0.0}, 0.0f},
      {7u, {2.0, 0.0, 0.0}, 0.0f},
      {90000u, {3.0, 0.0, 0.0}, 0.0f}};
  const auto state = MakeState(actors);
  const std::vector<carla::ActorId> expected_ids = {7u, 90000u, 4000000u};
  const auto ids = state->GetActorIds();
  ASSERT_EQ(std::vector<carla::ActorId>(ids.begin(), ids.end()), expected_ids);
  for (size_t i = 0u; i < expected_ids.size(); ++i) {
    const auto index = state->FindIndex(expected_ids[i]);
    ASSERT_TRUE(index.has_value());
    ASSERT_EQ(*index, i);
  }
  for (const carla::ActorId missing : {0u, 8u, 89999u, 4000001u}) {
    ASSERT_FALSE(state->FindIndex(missing).has_value());
  }
}
