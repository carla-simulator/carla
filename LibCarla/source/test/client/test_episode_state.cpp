// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/client/ActorSnapshot.h>
#include <carla/client/detail/EpisodeState.h>

#include <array>
#include <cstring>
#include <new>

namespace {

  void ExpectEmptyVehicleState(const carla::client::ActorSnapshot &snapshot) {
    // Check the representation before converting packed bools: an
    // uninitialized snapshot can contain invalid bool representations.
    const std::array<unsigned char, sizeof(snapshot.state)> zero{};
    ASSERT_EQ(std::memcmp(&snapshot.state, zero.data(), zero.size()), 0);

    const carla::rpc::VehicleControl control = snapshot.state.vehicle_data.control;
    EXPECT_EQ(control, carla::rpc::VehicleControl{});
    EXPECT_FLOAT_EQ(snapshot.state.vehicle_data.speed_limit, 0.0f);
    EXPECT_FALSE(snapshot.state.vehicle_data.has_traffic_light);
    EXPECT_EQ(snapshot.state.vehicle_data.traffic_light_id, 0u);
    EXPECT_EQ(snapshot.state.vehicle_data.failure_state, carla::rpc::VehicleFailureState::None);
  }

} // namespace

TEST(episode_state, default_actor_snapshot_has_empty_vehicle_state) {
  using carla::client::ActorSnapshot;
  alignas(ActorSnapshot) unsigned char storage[sizeof(ActorSnapshot)];
  std::memset(storage, 0xa5, sizeof(storage));
  auto *snapshot = new (storage) ActorSnapshot;

  EXPECT_EQ(snapshot->id, 0u);
  EXPECT_EQ(snapshot->actor_state, carla::rpc::ActorState::Invalid);
  ExpectEmptyVehicleState(*snapshot);

  snapshot->~ActorSnapshot();
}

TEST(episode_state, missing_actor_snapshot_has_empty_vehicle_state) {
  const carla::client::detail::EpisodeState state(42u);
  const auto snapshot = state.GetActorSnapshot(7u);

  EXPECT_EQ(snapshot.id, 0u);
  EXPECT_EQ(snapshot.actor_state, carla::rpc::ActorState::Invalid);
  ExpectEmptyVehicleState(snapshot);
  EXPECT_FALSE(state.GetActorSnapshotIfPresent(7u).has_value());
}
