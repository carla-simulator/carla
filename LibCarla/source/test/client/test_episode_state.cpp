// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/client/detail/EpisodeState.h>
#include <carla/sensor/Deserializer.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/RawEpisodeState.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>

#include <cstring>
#include <vector>

using carla::client::detail::EpisodeState;
using carla::sensor::data::ActorDynamicState;
using carla::sensor::data::RawEpisodeState;
using carla::sensor::data::TrafficLightDynamicState;
using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
using SensorHeader = carla::sensor::s11n::SensorHeaderSerializer::Header;
using TLS = carla::rpc::TrafficLightState;

/// The message the server sends on every tick: the sensor header, the episode
/// header, the static traffic lights and then the other actors.
static carla::SharedPtr<carla::sensor::SensorData> MakeEpisodeState(
    uint32_t traffic_light_count,
    const std::vector<TrafficLightDynamicState> &lights,
    const std::vector<ActorDynamicState> &actors) {
  SensorHeader sensor{};
  sensor.sensor_type = carla::sensor::SensorRegistry::get<FWorldObserver *>::index;
  sensor.frame = 42u;
  sensor.timestamp = 2.5;
  Serializer::Header episode{};
  episode.episode_id = 7u;
  episode.platform_timestamp = 100.0;
  episode.delta_seconds = 0.05f;
  // Static-actors version 3, no flags.
  episode.simulation_state =
      static_cast<Serializer::SimulationState>(3u << Serializer::static_actors_version_shift);
  episode.traffic_light_count = traffic_light_count;

  const size_t lights_size = lights.size() * sizeof(TrafficLightDynamicState);
  const size_t actors_size = actors.size() * sizeof(ActorDynamicState);
  carla::Buffer buffer(
      static_cast<uint64_t>(sizeof(sensor) + sizeof(episode) + lights_size + actors_size));
  auto *out = buffer.data();
  std::memcpy(out, &sensor, sizeof(sensor));
  out += sizeof(sensor);
  std::memcpy(out, &episode, sizeof(episode));
  out += sizeof(episode);
  if (lights_size > 0u) {
    std::memcpy(out, lights.data(), lights_size);
    out += lights_size;
  }
  if (actors_size > 0u) {
    std::memcpy(out, actors.data(), actors_size);
  }
  return carla::sensor::Deserializer::Deserialize(std::move(buffer));
}

static ActorDynamicState MakeActor(carla::ActorId id, float x) {
  ActorDynamicState actor{};
  actor.id = id;
  actor.actor_state = carla::rpc::ActorState::Active;
  actor.transform = carla::geom::Transform(
      carla::geom::Location(x, 2.0f * x, 3.0f * x),
      carla::geom::Rotation(1.0f, 10.0f * x, 3.0f));
  actor.velocity = carla::geom::Vector3D(x, 0.0f, -x);
  actor.state.vehicle_data.speed_limit = 30.0f + x;
  return actor;
}

TEST(episode_state, record_sizes) {
  // Client and server share these structs byte for byte.
  ASSERT_EQ(sizeof(Serializer::Header), 40u);
  ASSERT_EQ(sizeof(TrafficLightDynamicState), 10u);
  ASSERT_EQ(sizeof(ActorDynamicState), 119u);
}

TEST(episode_state, traffic_lights_travel_in_front_of_the_actors) {
  const std::vector<TrafficLightDynamicState> lights = {
      {11u, 1.25f, false, TLS::Green},
      {12u, 0.0f, true, TLS::Yellow}};
  const std::vector<ActorDynamicState> actors = {
      MakeActor(101u, 1.0f), MakeActor(102u, 2.0f), MakeActor(103u, 3.0f)};
  const auto data = MakeEpisodeState(2u, lights, actors);
  const auto &raw = static_cast<const RawEpisodeState &>(*data);

  ASSERT_EQ(raw.size(), 3u);
  ASSERT_EQ(raw.GetTrafficLights().size(), 2u);
  ASSERT_EQ(raw.GetEpisodeId(), 7u);
  ASSERT_EQ(raw.GetFrame(), 42u);

  const EpisodeState state(raw);
  ASSERT_EQ(state.GetEpisodeId(), 7u);
  ASSERT_EQ(state.GetFrame(), 42u);
  ASSERT_EQ(state.GetStaticActorsVersion(), 3u);
  ASSERT_FALSE(state.HasMapChanged());

  // The actors are the records after the traffic lights, read whole.
  ASSERT_EQ(state.size(), 3u);
  for (const auto &expected : actors) {
    const carla::ActorId id = expected.id;
    const auto actor = state.GetActorSnapshotIfPresent(id);
    ASSERT_TRUE(actor.has_value());
    ASSERT_EQ(actor->id, id);
    ASSERT_EQ(actor->actor_state, carla::rpc::ActorState::Active);
    const carla::geom::Transform transform = expected.transform;
    ASSERT_EQ(actor->transform, transform);
    const carla::geom::Vector3D velocity = expected.velocity;
    ASSERT_EQ(actor->velocity, velocity);
    const float speed_limit = expected.state.vehicle_data.speed_limit;
    ASSERT_EQ(float(actor->state.vehicle_data.speed_limit), speed_limit);
  }

  // A static traffic light is not one of them: only its changing state is here.
  ASSERT_FALSE(state.ContainsActorSnapshot(11u));
  ASSERT_FALSE(state.ContainsActorSnapshot(12u));
  const auto green = state.GetTrafficLightState(11u);
  ASSERT_TRUE(green.has_value());
  ASSERT_EQ(float(green->elapsed_time), 1.25f);
  ASSERT_FALSE(green->time_is_frozen);
  ASSERT_EQ(TLS(green->state), TLS::Green);
  const auto yellow = state.GetTrafficLightState(12u);
  ASSERT_TRUE(yellow.has_value());
  ASSERT_EQ(float(yellow->elapsed_time), 0.0f);
  ASSERT_TRUE(yellow->time_is_frozen);
  ASSERT_EQ(TLS(yellow->state), TLS::Yellow);

  // An actor has no traffic-light state, and an unknown id has nothing.
  ASSERT_FALSE(state.GetTrafficLightState(101u).has_value());
  ASSERT_FALSE(state.GetTrafficLightState(999u).has_value());
  ASSERT_FALSE(state.ContainsActorSnapshot(999u));
}

TEST(episode_state, no_traffic_lights) {
  const std::vector<ActorDynamicState> actors = {MakeActor(5u, 1.0f), MakeActor(6u, 2.0f)};
  const auto data = MakeEpisodeState(0u, {}, actors);
  const auto &raw = static_cast<const RawEpisodeState &>(*data);
  ASSERT_EQ(raw.size(), 2u);
  ASSERT_EQ(raw.GetTrafficLights().size(), 0u);
  const EpisodeState state(raw);
  ASSERT_EQ(state.size(), 2u);
  ASSERT_TRUE(state.ContainsActorSnapshot(5u));
  ASSERT_TRUE(state.ContainsActorSnapshot(6u));
  ASSERT_FALSE(state.GetTrafficLightState(5u).has_value());
}

TEST(episode_state, only_traffic_lights) {
  const std::vector<TrafficLightDynamicState> lights = {{3u, 4.5f, false, TLS::Red}};
  const auto data = MakeEpisodeState(1u, lights, {});
  const auto &raw = static_cast<const RawEpisodeState &>(*data);
  ASSERT_EQ(raw.size(), 0u);
  ASSERT_EQ(raw.GetTrafficLights().size(), 1u);
  const EpisodeState state(raw);
  ASSERT_EQ(state.size(), 0u);
  ASSERT_TRUE(state.GetTrafficLightState(3u).has_value());
  ASSERT_EQ(TLS(state.GetTrafficLightState(3u)->state), TLS::Red);
}

TEST(episode_state, a_count_the_message_cannot_hold_reads_as_empty) {
  // A message from another build: the count does not match what follows.
  // Nothing may be read past the end of the message.
  const std::vector<TrafficLightDynamicState> lights = {{3u, 4.5f, false, TLS::Red}};
  const std::vector<ActorDynamicState> actors = {MakeActor(5u, 1.0f)};
  for (const uint32_t count : {1000u, 2u, 0u, 0xFFFFFFFFu}) {
    const auto data = MakeEpisodeState(count, lights, actors);
    const auto &raw = static_cast<const RawEpisodeState &>(*data);
    ASSERT_EQ(raw.size(), 0u) << count;
    ASSERT_EQ(raw.GetTrafficLights().size(), 0u) << count;
    const EpisodeState state(raw);
    ASSERT_EQ(state.size(), 0u) << count;
    ASSERT_FALSE(state.GetTrafficLightState(3u).has_value()) << count;
    ASSERT_EQ(state.GetFrame(), 42u) << count;
  }
}
