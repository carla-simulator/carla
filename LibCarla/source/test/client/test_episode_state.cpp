// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/client/WorldSnapshot.h>
#include <carla/client/detail/EpisodeState.h>
#include <carla/client/detail/SnapshotStreamDecoder.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/ActorDynamicState.h>
#include <carla/sensor/data/RawEpisodeState.h>
#include <carla/sensor/s11n/EpisodeStateCodec.h>
#include <carla/sensor/s11n/EpisodeStateSerializer.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>

#include <cstring>
#include <memory>
#include <random>
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

namespace {

  /// A world-snapshot stream message: sensor header, then a body that the
  /// encoder completed (registry order), as the server sends it.
  struct Message {
    std::vector<unsigned char> keyframe;
    std::vector<unsigned char> delta;  // empty if there is none
  };

  Message MakeMessage(
      carla::sensor::s11n::episode_codec::DeltaEncoder &encoder,
      uint64_t frame,
      const std::vector<ActorDynamicState> &records) {
    namespace s11n = carla::sensor::s11n;
    s11n::SensorHeaderSerializer::Header sensor_header{};
    sensor_header.sensor_type =
        carla::sensor::SensorRegistry::get<FWorldObserver *>::index;
    sensor_header.frame = frame;
    sensor_header.timestamp = 0.05 * static_cast<double>(frame);
    s11n::EpisodeStateSerializer::Header episode_header{};
    episode_header.episode_id = 3u;
    episode_header.delta_seconds = 0.05f;
    std::vector<unsigned char> body(sizeof(episode_header));
    std::memcpy(body.data(), &episode_header, sizeof(episode_header));
    for (const auto &record : records) {
      const auto *begin = reinterpret_cast<const unsigned char *>(&record);
      body.insert(body.end(), begin, begin + sizeof(record));
    }
    encoder.Prepare(body.data(), body.size(), frame);
    Message message;
    const auto *sensor_begin = reinterpret_cast<const unsigned char *>(&sensor_header);
    message.keyframe.assign(sensor_begin, sensor_begin + sizeof(sensor_header));
    message.keyframe.insert(message.keyframe.end(), body.begin(), body.end());
    if (encoder.HasDelta()) {
      message.delta.assign(sensor_begin, sensor_begin + sizeof(sensor_header));
      message.delta.resize(sizeof(sensor_header) + encoder.GetDeltaSize());
      encoder.WriteDelta(body.data(), message.delta.data() + sizeof(sensor_header));
    }
    return message;
  }

  std::shared_ptr<EpisodeState> Publish(carla::Buffer buffer) {
    auto data = carla::sensor::SensorRegistry::Deserialize(std::move(buffer));
    auto raw = std::static_pointer_cast<carla::sensor::data::RawEpisodeState>(data);
    return std::make_shared<EpisodeState>(*raw);
  }

  void ExpectSameState(const EpisodeState &lhs, const EpisodeState &rhs, uint64_t frame) {
    ASSERT_EQ(lhs.GetFrame(), rhs.GetFrame());
    ASSERT_EQ(lhs.GetEpisodeId(), rhs.GetEpisodeId());
    ASSERT_EQ(lhs.GetTimestamp().elapsed_seconds, rhs.GetTimestamp().elapsed_seconds);
    ASSERT_EQ(lhs.size(), rhs.size()) << "frame " << frame;
    const auto lhs_ids = lhs.GetActorIds();
    const auto rhs_ids = rhs.GetActorIds();
    ASSERT_TRUE(std::equal(lhs_ids.begin(), lhs_ids.end(), rhs_ids.begin(), rhs_ids.end()));
    for (size_t i = 0u; i < lhs.size(); ++i) {
      const auto &a = lhs.GetActorSnapshotAt(i);
      const auto &b = rhs.GetActorSnapshotAt(i);
      ASSERT_EQ(a.id, b.id);
      ASSERT_EQ(a.actor_state, b.actor_state);
      ASSERT_EQ(a.transform, b.transform) << "frame " << frame << " actor " << a.id;
      ASSERT_EQ(a.velocity, b.velocity);
      ASSERT_EQ(a.angular_velocity, b.angular_velocity);
      ASSERT_EQ(a.acceleration, b.acceleration);
      ASSERT_EQ(std::memcmp(&a.state, &b.state, sizeof(a.state)), 0);
      ASSERT_EQ(lhs.GetLocationAt(i), rhs.GetLocationAt(i));
    }
  }

} // namespace

// The client publishes, from deltas, the same EpisodeState as from the whole
// snapshots, actor by actor, while actors move, come and go.
TEST(episode_state, state_from_deltas_equals_state_from_keyframes) {
  carla::sensor::s11n::episode_codec::DeltaEncoder encoder;
  carla::client::detail::SnapshotStreamDecoder by_keyframe;
  carla::client::detail::SnapshotStreamDecoder by_delta;
  std::mt19937 rng(11u);
  std::uniform_real_distribution<float> step(-1.0f, 1.0f);

  std::vector<ActorDynamicState> actors;
  carla::ActorId next_id = 1u;
  auto spawn = [&]() {
    ActorDynamicState state;
    std::memset(&state, 0, sizeof(state));
    state.id = next_id++;
    state.actor_state = carla::rpc::ActorState::Active;
    state.location[0u] = 1000.0 * state.id;
    state.state.vehicle_data.speed_limit = 30.0f;
    actors.push_back(state);
  };
  for (int i = 0; i < 50; ++i) {
    spawn();
  }

  size_t deltas = 0u;
  for (uint64_t frame = 1u; frame <= 300u; ++frame) {
    for (size_t k = 0u; k < actors.size(); k += 2u) {
      actors[k].location[0u] += step(rng);
      actors[k].velocity = carla::geom::Vector3D{step(rng), step(rng), 0.0f};
      carla::rpc::VehicleControl control;
      control.throttle = 0.5f + 0.1f * step(rng);
      actors[k].state.vehicle_data.control = control;
    }
    if (frame % 7u == 0u) {
      actors.erase(actors.begin() + static_cast<long>(rng() % actors.size()));
    }
    if (frame % 5u == 0u) {
      spawn();
    }
    if (frame % 50u == 0u) {
      std::shuffle(actors.begin(), actors.end(), rng);  // registry order
    }
    const Message message = MakeMessage(encoder, frame, actors);

    carla::Buffer full(message.keyframe.data(), message.keyframe.size());
    ASSERT_EQ(by_keyframe.Decode(full), carla::client::detail::SnapshotStreamDecoder::Action::Publish);
    const auto expected = Publish(std::move(full));

    const auto &sent = message.delta.empty() ? message.keyframe : message.delta;
    carla::Buffer received(sent.data(), sent.size());
    ASSERT_EQ(by_delta.Decode(received), carla::client::detail::SnapshotStreamDecoder::Action::Publish);
    const auto decoded = Publish(std::move(received));
    ExpectSameState(*expected, *decoded, frame);
    deltas += message.delta.empty() ? 0u : 1u;
  }
  EXPECT_GT(deltas, 290u);
  const auto stats = by_delta.GetStats();
  EXPECT_EQ(stats.deltas, deltas);
  EXPECT_EQ(stats.keyframes, 300u - deltas);
  EXPECT_EQ(stats.resyncs, 0u);
  EXPECT_LT(stats.wire_bytes, by_keyframe.GetStats().wire_bytes / 2u);
  EXPECT_EQ(stats.published_bytes, by_keyframe.GetStats().published_bytes);
}

// A delta that does not follow what the client has asks for a resync and
// publishes nothing; a server of another wire format is skipped.
TEST(episode_state, decoder_asks_for_a_resync_or_skips) {
  carla::sensor::s11n::episode_codec::DeltaEncoder encoder;
  std::vector<ActorDynamicState> actors(3u);
  for (size_t k = 0u; k < actors.size(); ++k) {
    std::memset(&actors[k], 0, sizeof(ActorDynamicState));
    actors[k].id = static_cast<carla::ActorId>(k + 1u);
  }
  const Message first = MakeMessage(encoder, 1u, actors);
  actors[0].location[0u] = 5.0;
  const Message second = MakeMessage(encoder, 2u, actors);
  ASSERT_FALSE(second.delta.empty());

  using Action = carla::client::detail::SnapshotStreamDecoder::Action;
  carla::client::detail::SnapshotStreamDecoder decoder;
  carla::Buffer delta(second.delta.data(), second.delta.size());
  EXPECT_EQ(decoder.Decode(delta), Action::Resync);
  EXPECT_EQ(decoder.GetResyncCount(), 1u);
  carla::Buffer keyframe(first.keyframe.data(), first.keyframe.size());
  EXPECT_EQ(decoder.Decode(keyframe), Action::Publish);
  carla::Buffer delta_again(second.delta.data(), second.delta.size());
  EXPECT_EQ(decoder.Decode(delta_again), Action::Publish);

  // The previous wire format: the body starts with the episode id.
  std::vector<unsigned char> old_message(first.keyframe.begin(), first.keyframe.begin() + 48);
  old_message.resize(48u + 36u + 131u, 0u);
  old_message[48] = 3u;
  carla::Buffer old_buffer(old_message.data(), old_message.size());
  EXPECT_EQ(decoder.Decode(old_buffer), Action::Skip);
  EXPECT_EQ(decoder.GetStats().incompatible, 1u);
}
