// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/client/detail/EpisodeState.h>
#include <carla/sensor/Deserializer.h>
#include <carla/sensor/SensorRegistry.h>
#include <carla/sensor/data/RawEpisodeState.h>
#include <carla/sensor/s11n/SensorHeaderSerializer.h>

#include <cstddef>
#include <cstring>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

using carla::client::detail::EpisodeState;
using carla::sensor::data::ActorDynamicState;
using carla::sensor::data::RawEpisodeState;
using carla::sensor::data::TrafficLightDynamicState;
using Serializer = carla::sensor::s11n::EpisodeStateSerializer;
using SensorHeader = carla::sensor::s11n::SensorHeaderSerializer::Header;
using TLS = carla::rpc::TrafficLightState;
using StatePtr = std::shared_ptr<const EpisodeState>;

/// What the server builds on every tick, and diffs: the episode header, the static traffic lights, the actors.
static carla::SharedBufferView MakeBody(
    uint64_t episode,
    const std::vector<TrafficLightDynamicState> &lights,
    const std::vector<ActorDynamicState> &actors) {
  Serializer::Header header{};
  header.episode_id = episode;
  header.platform_timestamp = 100.0;
  header.delta_seconds = 0.05f;
  // Static-actors version 3, no flags.
  header.simulation_state =
      static_cast<Serializer::SimulationState>(3u << Serializer::static_actors_version_shift);
  header.traffic_light_count = static_cast<uint32_t>(lights.size());
  const size_t lights_size = lights.size() * sizeof(TrafficLightDynamicState);
  const size_t actors_size = actors.size() * sizeof(ActorDynamicState);
  carla::Buffer buffer(static_cast<uint64_t>(sizeof(header) + lights_size + actors_size));
  auto *out = buffer.data();
  std::memcpy(out, &header, sizeof(header));
  if (lights_size > 0u) {
    std::memcpy(out + sizeof(header), lights.data(), lights_size);
  }
  if (actors_size > 0u) {
    std::memcpy(out + sizeof(header) + lights_size, actors.data(), actors_size);
  }
  return carla::BufferView::CreateFrom(std::move(buffer));
}

/// What a client gets when @a body is sent as the message of @a frame.
static carla::SharedPtr<carla::sensor::SensorData> Receive(uint64_t frame, const unsigned char *body, size_t size) {
  SensorHeader sensor{};
  sensor.sensor_type = carla::sensor::SensorRegistry::get<FWorldObserver *>::index;
  sensor.frame = frame;
  carla::Buffer buffer(static_cast<uint64_t>(sizeof(sensor) + size));
  std::memcpy(buffer.data(), &sensor, sizeof(sensor));
  std::memcpy(buffer.data() + sizeof(sensor), body, size);
  return carla::sensor::Deserializer::Deserialize(std::move(buffer));
}

/// A whole message of frame 42 and episode 7 whose header gives @a traffic_light_count, right or not.
static carla::SharedPtr<carla::sensor::SensorData> MakeEpisodeState(
    uint32_t traffic_light_count,
    const std::vector<TrafficLightDynamicState> &lights,
    const std::vector<ActorDynamicState> &actors) {
  const auto body = MakeBody(7u, lights, actors);
  std::vector<unsigned char> bytes(body->data(), body->data() + body->size());
  std::memcpy(
      bytes.data() + offsetof(Serializer::Header, traffic_light_count),
      &traffic_light_count,
      sizeof(traffic_light_count));
  return Receive(42u, bytes.data(), bytes.size());
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

static StatePtr Apply(uint64_t frame, const unsigned char *body, size_t size, const StatePtr &previous) {
  const auto data = Receive(frame, body, size);
  return EpisodeState::Make(static_cast<const RawEpisodeState &>(*data), previous);
}

template <typename T>
static StatePtr Apply(uint64_t frame, const T &body, const StatePtr &previous) {
  return Apply(frame, body.data(), body.size(), previous);
}

/// Compares through the wire record, so every byte a delta can carry is checked.
static void ExpectState(
    const StatePtr &state,
    const std::vector<TrafficLightDynamicState> &lights,
    const std::vector<ActorDynamicState> &actors) {
  ASSERT_NE(state, nullptr);
  ASSERT_EQ(state->size(), actors.size());
  const auto ids = state->GetActorIds();
  ASSERT_EQ(static_cast<size_t>(std::distance(ids.begin(), ids.end())), actors.size());
  for (const auto &expected : actors) {
    const carla::ActorId id = expected.id;
    const auto got = state->GetActorSnapshotIfPresent(id);
    ASSERT_TRUE(got.has_value()) << id;
    const ActorDynamicState record{
        got->id, got->actor_state, got->transform, got->velocity, got->angular_velocity, got->acceleration,
        got->state};
    ASSERT_EQ(std::memcmp(&record, &expected, sizeof(record)), 0) << id;
  }
  for (const auto &expected : lights) {
    const carla::ActorId id = expected.id;
    const auto got = state->GetTrafficLightState(id);
    ASSERT_TRUE(got.has_value()) << id;
    ASSERT_EQ(std::memcmp(&*got, &expected, sizeof(expected)), 0) << id;
  }
}

static std::vector<ActorDynamicState> MakeActors(size_t count, carla::ActorId first = 100u) {
  std::vector<ActorDynamicState> actors;
  for (size_t i = 0u; i < count; ++i) {
    actors.push_back(MakeActor(first + static_cast<carla::ActorId>(i), 1.0f + static_cast<float>(i)));
  }
  return actors;
}

static std::vector<TrafficLightDynamicState> MakeLights(size_t count, carla::ActorId first = 10u) {
  std::vector<TrafficLightDynamicState> lights;
  for (size_t i = 0u; i < count; ++i) {
    lights.push_back({first + static_cast<carla::ActorId>(i), 0.5f * static_cast<float>(i), false, TLS::Red});
  }
  return lights;
}

static unsigned char *Bytes(ActorDynamicState &actor) {
  return reinterpret_cast<unsigned char *>(&actor);
}

TEST(episode_state_delta, every_mask_and_the_lights) {
  const auto lights = MakeLights(2u);
  const auto before = MakeActors(3u);
  const auto base_body = MakeBody(7u, lights, before);
  const auto base = Apply(1u, *base_body, nullptr);
  ExpectState(base, lights, before);
  for (unsigned mask = 0u; mask < 256u; ++mask) {
    auto after = before;
    auto after_lights = lights;
    size_t expected_size = sizeof(Serializer::Header);
    if (mask % 3u == 0u) {
      after_lights[1u].state = TLS::Green;
      expected_size += sizeof(TrafficLightDynamicState);
    }
    const size_t mask_at = expected_size + sizeof(carla::ActorId);
    for (size_t range = 0u; range < 8u; ++range) {
      if ((mask & (1u << range)) != 0u) {
        // The last byte of the range, to catch a range that is read one byte short.
        Bytes(after[1u])[Serializer::delta_ranges[range + 1u] - 1u] ^= 0xA5u;
        expected_size += Serializer::delta_ranges[range + 1u] - Serializer::delta_ranges[range];
      }
    }
    expected_size += (mask != 0u) ? sizeof(carla::ActorId) + 1u : 0u;
    carla::Buffer delta;
    ASSERT_TRUE(Serializer::MakeDelta(*base_body, *MakeBody(7u, after_lights, after), delta)) << mask;
    ASSERT_EQ(delta.size(), expected_size) << mask;
    if (mask != 0u) {
      ASSERT_EQ(delta.data()[mask_at], mask);
    }
    const auto data = Receive(2u, delta.data(), delta.size());
    const auto &raw = static_cast<const RawEpisodeState &>(*data);
    ASSERT_TRUE(raw.IsDelta());
    ASSERT_EQ(raw.size(), 0u);
    const auto next = EpisodeState::Make(raw, base);
    ExpectState(next, after_lights, after);
    ASSERT_EQ(next->GetFrame(), 2u);
    ASSERT_EQ(next->GetStaticActorsVersion(), 3u);
    // The state it was applied on is not touched: a client may still hold it.
    ExpectState(base, lights, before);
  }
}

TEST(episode_state_delta, no_delta_when_the_actors_are_not_the_same) {
  const auto lights = MakeLights(2u);
  const auto actors = MakeActors(4u);
  const auto body = MakeBody(7u, lights, actors);
  carla::Buffer delta;
  auto changed = actors;
  changed.push_back(MakeActor(900u, 9.0f));                       // spawned
  ASSERT_FALSE(Serializer::MakeDelta(*body, *MakeBody(7u, lights, changed), delta));
  changed = actors;
  changed.erase(changed.begin() + 1);                              // destroyed
  ASSERT_FALSE(Serializer::MakeDelta(*body, *MakeBody(7u, lights, changed), delta));
  changed = actors;
  changed[2u].id = 900u;                                           // destroyed, and its slot reused
  ASSERT_FALSE(Serializer::MakeDelta(*body, *MakeBody(7u, lights, changed), delta));
  changed = actors;
  std::swap(changed[0u], changed[3u]);                             // another order
  ASSERT_FALSE(Serializer::MakeDelta(*body, *MakeBody(7u, lights, changed), delta));
  auto other_lights = lights;
  other_lights[1u].id = 900u;                                      // another light
  ASSERT_FALSE(Serializer::MakeDelta(*body, *MakeBody(7u, other_lights, actors), delta));
  ASSERT_FALSE(Serializer::MakeDelta(*body, *MakeBody(8u, lights, actors), delta));   // another episode
  // The same size with other counts: 119 lights take what 10 actors take.
  ASSERT_FALSE(Serializer::MakeDelta(
      *MakeBody(7u, MakeLights(119u), {}), *MakeBody(7u, {}, MakeActors(10u)), delta));
  ASSERT_TRUE(Serializer::MakeDelta(*body, *MakeBody(7u, lights, actors), delta));
}

TEST(episode_state_delta, no_delta_when_it_would_not_be_smaller) {
  const auto before = MakeActors(20u);
  auto after = before;
  for (auto &actor : after) {
    for (size_t i = sizeof(carla::ActorId); i < sizeof(ActorDynamicState); ++i) {
      Bytes(actor)[i] ^= 0xFFu;
    }
  }
  carla::Buffer delta;
  // Every entry is one byte longer than its record.
  ASSERT_FALSE(Serializer::MakeDelta(*MakeBody(7u, {}, before), *MakeBody(7u, {}, after), delta));
  // All of them but the last one: 19 * 120 + 40 fits in 20 * 119 + 40.
  after.back() = before.back();
  ASSERT_TRUE(Serializer::MakeDelta(*MakeBody(7u, {}, before), *MakeBody(7u, {}, after), delta));
  ExpectState(Apply(2u, delta, Apply(1u, *MakeBody(7u, {}, before), nullptr)), {}, after);
}

TEST(episode_state_delta, a_delta_that_does_not_follow_is_refused) {
  const auto lights = MakeLights(2u);
  const auto before = MakeActors(3u);
  auto after = before;
  after[1u].acceleration = carla::geom::Vector3D(1.0f, 2.0f, 3.0f);
  auto after_lights = lights;
  after_lights[0u].state = TLS::Green;
  const auto body = MakeBody(7u, lights, before);
  carla::Buffer delta;
  ASSERT_TRUE(Serializer::MakeDelta(*body, *MakeBody(7u, after_lights, after), delta));
  const auto base = Apply(10u, *body, nullptr);
  ExpectState(Apply(11u, delta, base), after_lights, after);
  ASSERT_EQ(Apply(11u, delta, nullptr), nullptr);                                    // no state yet
  ASSERT_EQ(Apply(12u, delta, base), nullptr);                                       // a frame was missed
  ASSERT_EQ(Apply(10u, delta, base), nullptr);                                       // the same frame
  ASSERT_EQ(Apply(11u, delta, Apply(10u, *MakeBody(8u, lights, before), nullptr)), nullptr);   // another episode
  // An actor or a light the client does not have.
  auto fewer = before;
  fewer.erase(fewer.begin() + 1);
  ASSERT_EQ(Apply(11u, delta, Apply(10u, *MakeBody(7u, lights, fewer), nullptr)), nullptr);
  ASSERT_EQ(Apply(11u, delta, Apply(10u, *MakeBody(7u, MakeLights(2u, 50u), before), nullptr)), nullptr);
  // Cut anywhere inside its only actor entry: never read past the end, never applied by halves.
  for (size_t size = sizeof(Serializer::Header) + sizeof(TrafficLightDynamicState) + 1u; size < delta.size(); ++size) {
    ASSERT_EQ(Apply(11u, delta.data(), size, base), nullptr) << size;
  }
  // A light count the message cannot hold.
  std::vector<unsigned char> broken(delta.data(), delta.data() + delta.size());
  const uint32_t count = 1000u;
  std::memcpy(broken.data() + offsetof(Serializer::Header, traffic_light_count), &count, sizeof(count));
  ASSERT_EQ(Apply(11u, broken.data(), broken.size(), base), nullptr);
  // Whatever was refused, the state it was meant for is as it was.
  ExpectState(base, lights, before);
}

/// What FWorldObserver::BroadcastTick and MultiStreamState::WriteDelta do, with clients that miss messages.
TEST(episode_state_delta, fuzz_every_client_always_holds_the_world) {
  std::mt19937 rng(20261124u);
  const auto chance = [&rng](unsigned percent) { return (rng() % 100u) < percent; };
  struct Client {
    bool missed = true;
    StatePtr last;
    size_t whole = 0u;
    size_t deltas = 0u;
  };
  std::vector<Client> clients(4u);
  std::vector<TrafficLightDynamicState> lights = MakeLights(6u);
  std::vector<ActorDynamicState> actors = MakeActors(12u);
  carla::ActorId next_id = 1000u;
  uint64_t episode = 7u;
  uint64_t frame = 1u;
  uint64_t last_frame = 0u;
  carla::SharedBufferView last_full;
  const auto randomize = [&rng](ActorDynamicState &actor, size_t begin, size_t end) {
    for (size_t i = begin; i < end; ++i) {
      Bytes(actor)[i] = static_cast<unsigned char>(rng());
    }
  };
  for (size_t step = 0u; step < 4000u; ++step) {
    for (auto &actor : actors) {
      if (chance(40u)) {
        const size_t range = rng() % 8u;
        randomize(actor, Serializer::delta_ranges[range], Serializer::delta_ranges[range + 1u]);
      }
      if (chance(5u)) {
        randomize(actor, sizeof(carla::ActorId), sizeof(ActorDynamicState));
      }
    }
    for (auto &light : lights) {
      if (chance(30u)) {
        light.elapsed_time += 0.05f;
        light.state = static_cast<TLS>(rng() % 4u);
        light.time_is_frozen = chance(10u);
      }
    }
    if (chance(4u)) {
      actors.push_back(MakeActor(next_id++, 1.0f));
    }
    if (chance(4u) && !actors.empty()) {
      actors.erase(actors.begin() + static_cast<long>(rng() % actors.size()));
    }
    if (chance(3u) && !actors.empty()) {
      actors[rng() % actors.size()] = MakeActor(next_id++, 2.0f);   // a slot reused by a new actor
    }
    if (chance(1u)) {
      lights.push_back({next_id++, 0.0f, false, TLS::Red});
    }
    if (chance(1u)) {
      episode += 1u;
      frame = 1u + rng() % 50u;
      actors = MakeActors(1u + rng() % 15u, next_id);
      next_id += 20u;
    }
    frame += chance(3u) ? 2u : 1u;   // a frame that was not broadcast
    const auto full = MakeBody(episode, lights, actors);
    carla::Buffer delta;
    const bool is_delta = (last_full != nullptr) && (frame == last_frame + 1u) &&
        Serializer::MakeDelta(*last_full, *full, delta);
    for (auto &client : clients) {
      if (chance(2u)) {
        client.missed = true;   // reconnected: a new session, the client keeps its old state
      }
      const bool missed = std::exchange(client.missed, false);
      if (chance(10u)) {
        client.missed = true;   // this message is dropped for it
        continue;
      }
      const bool whole = missed || !is_delta;
      const auto next = whole ? Apply(frame, *full, client.last) : Apply(frame, delta, client.last);
      ASSERT_NE(next, nullptr) << "step " << step;
      client.last = next;
      (whole ? client.whole : client.deltas) += 1u;
      ExpectState(next, lights, actors);
      ASSERT_FALSE(::testing::Test::HasFatalFailure()) << "step " << step;
      ASSERT_EQ(next->GetFrame(), frame);
      ASSERT_EQ(next->GetEpisodeId(), episode);
    }
    last_full = full;
    last_frame = frame;
  }
  for (const auto &client : clients) {
    // Most of what a client got were deltas, or the test proves nothing.
    ASSERT_GT(client.deltas, client.whole);
    ASSERT_GT(client.whole, 100u);
  }
}
