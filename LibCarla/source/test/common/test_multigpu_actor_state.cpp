// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/multigpu/actorStateBatch.h>

#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace carla::multigpu;

namespace {

  /// Encodes @a state and returns the payload, checking the frame data header.
  std::string Encode(ActorStateEncoder &encoder, const ActorStateFrame &state,
      bool force_keyframe = false, ActorStateEncodeInfo *info = nullptr) {
    std::ostringstream out;
    const ActorStateEncodeInfo written = encoder.Encode(state, out, force_keyframe);
    const std::string bytes = out.str();
    EXPECT_EQ(bytes.size(), written.bytes);
    EXPECT_GE(bytes.size(), kFrameDataHeaderSize);
    EXPECT_EQ(static_cast<uint8_t>(bytes[0]), kActorStatePacketId);
    uint32_t size = 0u;
    std::memcpy(&size, bytes.data() + 1, sizeof(size));
    EXPECT_EQ(size, bytes.size() - kFrameDataHeaderSize);
    if (info != nullptr) {
      *info = written;
    }
    return bytes.substr(kFrameDataHeaderSize);
  }

  ActorStateDecodeResult Decode(ActorStateDecoder &decoder, const std::string &payload) {
    return decoder.Decode(reinterpret_cast<const uint8_t *>(payload.data()), payload.size());
  }

  /// Bit for bit equality of two states.
  ::testing::AssertionResult SameState(const ActorStateFrame &a, const ActorStateFrame &b) {
    if (a.frame != b.frame) {
      return ::testing::AssertionFailure() << "frame " << a.frame << " != " << b.frame;
    }
    if ((a.ids != b.ids) || (a.kinds != b.kinds)) {
      return ::testing::AssertionFailure() << "different slots";
    }
    if (a.sections.size() != b.sections.size()) {
      return ::testing::AssertionFailure() << "different section count";
    }
    for (auto ia = a.sections.begin(), ib = b.sections.begin(); ia != a.sections.end(); ++ia, ++ib) {
      const auto &x = ia->second;
      const auto &y = ib->second;
      if ((ia->first != x.Kind()) || (x.Kind() != y.Kind()) || (x.Stride() != y.Stride())) {
        return ::testing::AssertionFailure() << "different section " << int(x.Kind());
      }
      for (size_t i = 0u; i < a.SlotCount(); ++i) {
        if (!x.SameRecord(i, y)) {
          return ::testing::AssertionFailure()
              << "section " << int(x.Kind()) << " slot " << i << " differs";
        }
      }
    }
    return ::testing::AssertionSuccess();
  }

  double BitsToDouble(uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
  }

  PoseRecord MakePose(double x, double y, double z) {
    return PoseRecord{{x, y, z}, {0.0, 0.0, 0.0, 1.0}};
  }

  /// Three vehicles and a walker, with every section in use.
  ActorStateFrame MakeState(uint64_t frame) {
    ActorStateFrame state;
    state.frame = frame;
    state.ResetSlots({4u, 9u, 12u, 30u}, {1u, 1u, 1u, 2u});
    auto &pose = state.Section(ActorStateSection::Pose);
    auto &kinematics = state.Section(ActorStateSection::Kinematics);
    auto &control = state.Section(ActorStateSection::VehicleControl);
    auto &light = state.Section(ActorStateSection::VehicleLight);
    auto &wheels = state.Section(ActorStateSection::Wheels);
    auto &doors = state.Section(ActorStateSection::Doors);
    auto &walker = state.Section(ActorStateSection::Walker);
    for (size_t i = 0u; i < 4u; ++i) {
      pose.Set(i, MakePose(100.0 * double(i), -50.0, 10.0));
    }
    for (size_t i = 0u; i < 3u; ++i) {
      kinematics.Set(i, KinematicsRecord{{1.0, 2.0, 3.0}, {0.1, 0.2, 0.3}});
      control.Set(i, VehicleControlRecord{0.1f, 0.5f, 0.0f, 0u, 2});
      light.Set(i, VehicleLightRecord{3u});
      const WheelRecord four[4] = {{1.f, 2.f, 3.f}, {4.f, 5.f, 6.f}, {7.f, 8.f, 9.f}, {1.f, 1.f, 1.f}};
      wheels.SetArray(i, four, 4u);
    }
    const DoorRecord open{2u, {1.0, 2.0, 3.0}, {0.0, 0.0, 0.7071, 0.7071}};
    doors.SetArray(1u, &open, 1u);
    walker.Set(3u, WalkerRecord{1.4f});
    return state;
  }

} // namespace

TEST(MultiGpuActorState, keyframe_round_trip_bit_exact) {
  ActorStateFrame state = MakeState(10u);
  // Values whose bits a float or a text round trip would change.
  const double quiet_nan_with_payload = BitsToDouble(0x7ff8000000001234ull);
  const double signalling_nan = BitsToDouble(0x7ff0000000000001ull);
  const double denormal = std::numeric_limits<double>::denorm_min();
  auto &pose = state.Section(ActorStateSection::Pose);
  pose.Set(0u, PoseRecord{{quiet_nan_with_payload, -0.0, std::numeric_limits<double>::infinity()},
                          {signalling_nan, denormal, -denormal, 1.0}});
  pose.Set(1u, MakePose(700012.3456789012, -650098.7654321098, 1234.5678901234));

  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  ActorStateEncodeInfo info;
  const auto payload = Encode(encoder, state, false, &info);
  ASSERT_TRUE(info.keyframe);
  const auto result = Decode(decoder, payload);
  ASSERT_TRUE(result.IsOk()) << ActorStateDecodeStatusName(result.status);
  ASSERT_TRUE(result.keyframe);
  ASSERT_EQ(*result.frame, 10u);
  ASSERT_TRUE(SameState(decoder.State(), state));

  const auto got = decoder.State().FindSection(ActorStateSection::Pose)->Get<PoseRecord>(0u);
  ASSERT_TRUE(got.has_value());
  const auto sent = pose.Get<PoseRecord>(0u);
  ASSERT_TRUE(sent.has_value());
  ASSERT_EQ(std::memcmp(&*got, &*sent, sizeof(PoseRecord)), 0);
  ASSERT_TRUE(std::signbit(got->location[1]));

  const auto wheels = decoder.State().FindSection(ActorStateSection::Wheels)->GetArray<WheelRecord>(2u);
  ASSERT_EQ(wheels.size(), 4u);
  ASSERT_EQ(wheels[1].spin, 5.f);
  ASSERT_FALSE(decoder.State().FindSection(ActorStateSection::Doors)->IsPresent(0u));
  ASSERT_EQ(decoder.State().FindSection(ActorStateSection::Doors)->GetArray<DoorRecord>(1u).size(), 1u);
}

TEST(MultiGpuActorState, delta_carries_only_what_changed_bit_for_bit) {
  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  ActorStateFrame state = MakeState(1u);
  ActorStateEncodeInfo keyframe;
  ASSERT_TRUE(Decode(decoder, Encode(encoder, state, false, &keyframe)).IsOk());

  // Nothing changed: a delta with no records, a few bytes per section.
  state.frame = 2u;
  ActorStateEncodeInfo still;
  const auto still_payload = Encode(encoder, state, false, &still);
  ASSERT_FALSE(still.keyframe);
  ASSERT_EQ(still.records, 0u);
  ASSERT_EQ(still.bytes, kFrameDataHeaderSize + 32u + 7u * 8u);
  ASSERT_TRUE(Decode(decoder, still_payload).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));

  // +0 to -0, and one NaN payload to another, are changes.
  state.frame = 3u;
  auto &pose = state.Section(ActorStateSection::Pose);
  pose.Set(0u, MakePose(-0.0, -50.0, 10.0));
  auto nan_pose = MakePose(0.0, 0.0, 0.0);
  nan_pose.rotation[0] = BitsToDouble(0x7ff8000000000001ull);
  pose.Set(2u, nan_pose);
  ActorStateEncodeInfo moved;
  const auto moved_payload = Encode(encoder, state, false, &moved);
  ASSERT_FALSE(moved.keyframe);
  ASSERT_EQ(moved.records, 2u);
  ASSERT_TRUE(Decode(decoder, moved_payload).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));

  state.frame = 4u;
  nan_pose.rotation[0] = BitsToDouble(0x7ff8000000000002ull);
  pose.Set(2u, nan_pose);
  ActorStateEncodeInfo nan_changed;
  const auto nan_payload = Encode(encoder, state, false, &nan_changed);
  ASSERT_EQ(nan_changed.records, 1u);
  ASSERT_TRUE(Decode(decoder, nan_payload).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));

  // A record that appears, one that goes, and a variable one that grows.
  state.frame = 5u;
  state.Section(ActorStateSection::Kinematics).Clear(1u);
  state.Section(ActorStateSection::Biker).Set(3u, BikerRecord{2.0f, 30.0f});
  const DoorRecord doors[2] = {{0u, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 1.0}},
                               {3u, {1.0, 1.0, 1.0}, {0.0, 0.0, 0.0, 1.0}}};
  state.Section(ActorStateSection::Doors).SetArray(1u, doors, 2u);
  ActorStateEncodeInfo reshaped;
  const auto reshaped_payload = Encode(encoder, state, false, &reshaped);
  // A new section changes the table: a keyframe.
  ASSERT_TRUE(reshaped.keyframe);
  ASSERT_TRUE(Decode(decoder, reshaped_payload).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));

  state.frame = 6u;
  state.Section(ActorStateSection::Kinematics).Set(1u, KinematicsRecord{{9.0, 9.0, 9.0}, {0.0, 0.0, 0.0}});
  state.Section(ActorStateSection::Biker).Clear(3u);
  state.Section(ActorStateSection::Doors).Clear(1u);
  ActorStateEncodeInfo toggled;
  const auto toggled_payload = Encode(encoder, state, false, &toggled);
  ASSERT_FALSE(toggled.keyframe);
  ASSERT_EQ(toggled.records, 1u);
  ASSERT_TRUE(Decode(decoder, toggled_payload).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));
}

TEST(MultiGpuActorState, random_chains_of_10000_frames) {
  std::mt19937 rng(20261120u);
  std::uniform_real_distribution<double> uniform(0.0, 1.0);
  ActorStateEncoder encoder(250u);
  ActorStateDecoder decoder;

  uint32_t next_id = 1u;
  std::vector<uint32_t> ids;
  for (int i = 0; i < 40; ++i) {
    ids.push_back(next_id++);
  }
  ActorStateFrame state;
  auto reset_slots = [&]() {
    std::vector<uint8_t> kinds;
    for (uint32_t id : ids) {
      kinds.push_back(static_cast<uint8_t>(id % 3u));
    }
    state.ResetSlots(ids, kinds);
  };
  reset_slots();
  state.Section(ActorStateSection::Pose);
  state.Section(ActorStateSection::Kinematics);
  state.Section(ActorStateSection::VehicleControl);
  state.Section(ActorStateSection::Wheels);

  auto randomize = [&](size_t slot, double p) {
    auto &pose = state.Section(ActorStateSection::Pose);
    if (!pose.IsPresent(slot) || uniform(rng) < p) {
      // Every bit pattern, NaN and denormals included.
      PoseRecord record;
      for (double *value : {&record.location[0], &record.location[1], &record.location[2],
                            &record.rotation[0], &record.rotation[1], &record.rotation[2],
                            &record.rotation[3]}) {
        *value = BitsToDouble((uint64_t(rng()) << 32u) | rng());
      }
      pose.Set(slot, record);
    }
    auto &kinematics = state.Section(ActorStateSection::Kinematics);
    if (uniform(rng) < p) {
      if (uniform(rng) < 0.2) {
        kinematics.Clear(slot);
      } else {
        kinematics.Set(slot, KinematicsRecord{{uniform(rng), 0.0, -0.0}, {0.0, uniform(rng), 0.0}});
      }
    }
    auto &control = state.Section(ActorStateSection::VehicleControl);
    if ((state.kinds[slot] == 1u) && (uniform(rng) < p)) {
      control.Set(slot, VehicleControlRecord{float(uniform(rng)), 1.0f, 0.0f, 0u, int32_t(rng() % 6u)});
    }
    auto &wheels = state.Section(ActorStateSection::Wheels);
    if (uniform(rng) < p) {
      std::vector<WheelRecord> records(rng() % 5u);
      for (auto &wheel : records) {
        wheel = WheelRecord{float(uniform(rng)), float(uniform(rng)), 0.0f};
      }
      wheels.SetArray(slot, records.data(), records.size());
    }
  };
  for (size_t i = 0u; i < ids.size(); ++i) {
    randomize(i, 1.0);
  }

  size_t keyframes = 0u;
  size_t deltas = 0u;
  size_t delta_bytes = 0u;
  size_t keyframe_bytes = 0u;
  for (uint64_t frame = 1u; frame <= 10000u; ++frame) {
    state.frame = frame;
    if (uniform(rng) < 0.01) {
      // Spawns and destroys keep the slots of the survivors' records.
      ActorStateFrame previous = state;
      if (!ids.empty() && (uniform(rng) < 0.5)) {
        ids.erase(ids.begin() + static_cast<long>(rng() % ids.size()));
      } else {
        ids.push_back(next_id++);
      }
      reset_slots();
      for (size_t i = 0u; i < ids.size(); ++i) {
        const auto old = std::find(previous.ids.begin(), previous.ids.end(), ids[i]);
        if (old == previous.ids.end()) {
          randomize(i, 1.0);
          continue;
        }
        const size_t from = static_cast<size_t>(old - previous.ids.begin());
        for (auto &item : state.sections) {
          const auto &source = previous.sections.at(item.first);
          if (source.IsPresent(from)) {
            item.second.SetBytes(i, source.Data(from), source.Size(from));
          }
        }
      }
    }
    for (size_t i = 0u; i < ids.size(); ++i) {
      // Half the actors are parked: they never change after spawning.
      randomize(i, (ids[i] % 2u) == 0u ? 0.0 : 0.3);
    }
    ActorStateEncodeInfo info;
    const auto payload = Encode(encoder, state, false, &info);
    const auto result = Decode(decoder, payload);
    ASSERT_TRUE(result.IsOk()) << "frame " << frame << ": " << ActorStateDecodeStatusName(result.status);
    ASSERT_EQ(result.keyframe, info.keyframe);
    ASSERT_TRUE(SameState(decoder.State(), state)) << "frame " << frame;
    if (info.keyframe) {
      ++keyframes;
      keyframe_bytes += info.bytes;
    } else {
      ++deltas;
      delta_bytes += info.bytes;
    }
  }
  ASSERT_GT(deltas, 9000u);
  ASSERT_GE(keyframes, 10000u / 250u);
  // Deltas are smaller than keyframes on average.
  ASSERT_LT(delta_bytes / deltas, keyframe_bytes / keyframes);
}

TEST(MultiGpuActorState, a_lost_frame_is_refused_until_a_keyframe) {
  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  ActorStateFrame state = MakeState(100u);
  ASSERT_TRUE(Decode(decoder, Encode(encoder, state)).IsOk());

  // Frame 101 is lost on the way.
  state.frame = 101u;
  state.Section(ActorStateSection::Pose).Set(0u, MakePose(1.0, 1.0, 1.0));
  Encode(encoder, state);
  const ActorStateFrame good = decoder.State();

  for (uint64_t frame = 102u; frame < 106u; ++frame) {
    state.frame = frame;
    state.Section(ActorStateSection::Pose).Set(1u, MakePose(double(frame), 0.0, 0.0));
    const auto result = Decode(decoder, Encode(encoder, state));
    ASSERT_EQ(result.status, ActorStateDecodeStatus::NeedResync);
    ASSERT_EQ(*result.frame, frame);
    ASSERT_TRUE(decoder.WaitingResync());
    ASSERT_EQ(*decoder.LastGoodFrame(), 100u);
    ASSERT_TRUE(SameState(decoder.State(), good));
  }

  // The primary answers the resync with a keyframe.
  state.frame = 106u;
  const auto result = Decode(decoder, Encode(encoder, state, true));
  ASSERT_TRUE(result.IsOk());
  ASSERT_TRUE(result.keyframe);
  ASSERT_FALSE(decoder.WaitingResync());
  ASSERT_TRUE(SameState(decoder.State(), state));

  state.frame = 107u;
  ASSERT_TRUE(Decode(decoder, Encode(encoder, state)).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));
}

TEST(MultiGpuActorState, a_late_decoder_waits_for_a_keyframe) {
  ActorStateEncoder encoder;
  ActorStateFrame state = MakeState(1u);
  Encode(encoder, state);
  state.frame = 2u;
  const auto delta = Encode(encoder, state);
  ActorStateDecoder late;
  ASSERT_EQ(Decode(late, delta).status, ActorStateDecodeStatus::NeedResync);
  ASSERT_FALSE(late.LastGoodFrame().has_value());
}

TEST(MultiGpuActorState, keyframe_when_needed_and_on_the_interval) {
  ActorStateEncoder encoder(3u);
  ActorStateFrame state = MakeState(1u);
  std::vector<bool> keyframes;
  for (uint64_t frame = 1u; frame <= 7u; ++frame) {
    state.frame = frame;
    ActorStateEncodeInfo info;
    Encode(encoder, state, false, &info);
    keyframes.push_back(info.keyframe);
  }
  ASSERT_EQ(keyframes, (std::vector<bool>{true, false, false, true, false, false, true}));

  // A frame that does not follow the previous one, and a new table.
  ActorStateEncodeInfo info;
  state.frame = 9u;
  Encode(encoder, state, false, &info);
  ASSERT_TRUE(info.keyframe);
  state.frame = 10u;
  state.ResetSlots({4u, 9u, 12u, 31u}, {1u, 1u, 1u, 2u});
  Encode(encoder, state, false, &info);
  ASSERT_TRUE(info.keyframe);
  state.frame = 11u;
  Encode(encoder, state, false, &info);
  ASSERT_FALSE(info.keyframe);
  encoder.Reset();
  state.frame = 12u;
  Encode(encoder, state, false, &info);
  ASSERT_TRUE(info.keyframe);
}

TEST(MultiGpuActorState, unknown_sections_are_kept_as_bytes) {
  ActorStateFrame state = MakeState(1u);
  auto &future = state.Section(200u, 3u);
  const uint8_t bytes[3] = {1u, 2u, 3u};
  future.SetBytes(2u, bytes, 3u);
  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  ASSERT_TRUE(Decode(decoder, Encode(encoder, state)).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));
}

TEST(MultiGpuActorState, empty_frame) {
  ActorStateFrame state;
  state.frame = 1u;
  state.Section(ActorStateSection::Pose);
  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  ASSERT_TRUE(Decode(decoder, Encode(encoder, state)).IsOk());
  state.frame = 2u;
  ASSERT_TRUE(Decode(decoder, Encode(encoder, state)).IsOk());
  ASSERT_TRUE(SameState(decoder.State(), state));
}

TEST(MultiGpuActorState, golden_bytes) {
  // Fixes the wire layout: two slots, a pose each, wheels on the first.
  ActorStateFrame state;
  state.frame = 7u;
  state.ResetSlots({5u, 6u}, {1u, 2u});
  state.Section(ActorStateSection::Pose).Set(0u, MakePose(1.0, 2.0, 3.0));
  state.Section(ActorStateSection::Pose).Set(1u, MakePose(4.0, 5.0, 6.0));
  const WheelRecord wheel{1.0f, 2.0f, 3.0f};
  state.Section(ActorStateSection::Wheels).SetArray(0u, &wheel, 1u);

  std::string expected;
  auto put = [&expected](const auto &value) {
    expected.append(reinterpret_cast<const char *>(&value), sizeof(value));
  };
  // Keyframe.
  put(uint8_t{35u}); put(uint32_t{32u + 10u + 8u + 1u + 112u + 8u + 1u + 16u});
  put(uint8_t{1u}); put(uint8_t{1u}); put(uint16_t{2u}); put(uint32_t{2u});
  put(uint64_t{7u}); put(uint64_t{7u}); put(state.TableHash());
  put(uint32_t{5u}); put(uint32_t{6u}); put(uint8_t{1u}); put(uint8_t{2u});
  put(uint8_t{1u}); put(uint8_t{0u}); put(uint16_t{56u}); put(uint32_t{1u + 112u});
  put(uint8_t{0x03u});
  put(MakePose(1.0, 2.0, 3.0)); put(MakePose(4.0, 5.0, 6.0));
  put(uint8_t{5u}); put(uint8_t{0u}); put(uint16_t{0u}); put(uint32_t{1u + 16u});
  put(uint8_t{0x01u});
  put(uint32_t{12u}); put(wheel);

  ActorStateEncoder encoder;
  std::ostringstream keyframe;
  encoder.Encode(state, keyframe);
  ASSERT_EQ(keyframe.str(), expected);

  // Delta: the second pose moves.
  state.frame = 8u;
  state.Section(ActorStateSection::Pose).Set(1u, MakePose(4.0, 5.0, 7.0));
  expected.clear();
  put(uint8_t{35u}); put(uint32_t{32u + 8u + 1u + 56u + 8u});
  put(uint8_t{1u}); put(uint8_t{0u}); put(uint16_t{2u}); put(uint32_t{2u});
  put(uint64_t{8u}); put(uint64_t{7u}); put(state.TableHash());
  put(uint8_t{1u}); put(uint8_t{1u}); put(uint16_t{56u}); put(uint32_t{1u + 56u});
  put(uint8_t{0x02u});
  put(MakePose(4.0, 5.0, 7.0));
  put(uint8_t{5u}); put(uint8_t{3u}); put(uint16_t{0u}); put(uint32_t{0u});
  std::ostringstream delta;
  encoder.Encode(state, delta);
  ASSERT_EQ(delta.str(), expected);
}

TEST(MultiGpuActorState, unknown_layout_is_refused) {
  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  auto payload = Encode(encoder, MakeState(1u));
  payload[0] = static_cast<char>(kActorStateLayout + 1u);
  ASSERT_EQ(Decode(decoder, payload).status, ActorStateDecodeStatus::UnknownLayout);
  ASSERT_FALSE(decoder.HasState());
}

TEST(MultiGpuActorState, truncated_and_corrupted_packets_never_break_the_state) {
  ActorStateEncoder encoder;
  ActorStateFrame state = MakeState(1u);
  const auto keyframe = Encode(encoder, state);
  state.frame = 2u;
  state.Section(ActorStateSection::Pose).Set(1u, MakePose(3.0, 3.0, 3.0));
  state.Section(ActorStateSection::Kinematics).Clear(0u);
  const auto delta = Encode(encoder, state);

  ActorStateDecoder reference;
  ASSERT_TRUE(Decode(reference, keyframe).IsOk());
  const ActorStateFrame first = reference.State();

  // Every truncation of either packet is refused and keeps the state held.
  for (const auto *packet : {&keyframe, &delta}) {
    for (size_t size = 0u; size < packet->size(); ++size) {
      ActorStateDecoder decoder;
      ASSERT_TRUE(Decode(decoder, keyframe).IsOk());
      const auto result = decoder.Decode(reinterpret_cast<const uint8_t *>(packet->data()), size);
      ASSERT_FALSE(result.IsOk()) << "size " << size;
      ASSERT_TRUE(SameState(decoder.State(), first));
    }
  }

  // Random corruption: never a crash, and a refused packet keeps the state.
  std::mt19937 rng(35u);
  for (int round = 0; round < 20000; ++round) {
    ActorStateDecoder decoder;
    ASSERT_TRUE(Decode(decoder, keyframe).IsOk());
    std::string packet = (round % 2 == 0) ? keyframe : delta;
    const int flips = 1 + static_cast<int>(rng() % 4u);
    for (int f = 0; f < flips; ++f) {
      packet[rng() % packet.size()] = static_cast<char>(rng());
    }
    const auto result = Decode(decoder, packet);
    if (!result.IsOk()) {
      ASSERT_TRUE(SameState(decoder.State(), first));
    }
  }
}

TEST(MultiGpuActorState, packet_inside_a_frame_round_trips_through_the_frame_codec) {
  // As a primary sends it: other packets, packet 35 right before the frame
  // counter. A secondary keeps the payload the codec hands over and decodes it.
  ActorStateFrame state = MakeState(41u);
  // A vehicle whose wheel list is empty but present, as packet 32 can carry.
  state.Section(ActorStateSection::Wheels).SetArray<WheelRecord>(0u, nullptr, 0u);
  ActorStateEncoder encoder;
  ActorStateDecoder decoder;
  for (uint64_t frame = 41u; frame <= 43u; ++frame) {
    state.frame = frame;
    state.Section(ActorStateSection::Pose).Set(2u, MakePose(double(frame), 0.0, 0.0));
    std::ostringstream out;
    WriteFrameDataHeader(out, 34u, 4u);
    detail::WritePod(out, uint32_t(7u));
    const ActorStateEncodeInfo info = encoder.Encode(state, out);
    ASSERT_EQ(info.keyframe, frame == 41u);
    WriteFrameDataHeader(out, kFrameCounterPacketId, sizeof(uint64_t));
    detail::WritePod(out, frame);
    const std::string bytes = out.str();

    std::vector<uint8_t> payload;
    bool has_payload = false;
    const FrameDataReadResult read = ReadFrameData(
        reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size(),
        [&](uint8_t id, std::istream &in, uint32_t size) {
          if (id != kActorStatePacketId) {
            return false;
          }
          payload.resize(size);
          in.read(reinterpret_cast<char *>(payload.data()), size);
          has_payload = true;
          return true;
        });
    ASSERT_TRUE(read.IsComplete());
    ASSERT_EQ(*read.frame, frame);
    ASSERT_EQ(read.skipped, 1u);
    ASSERT_EQ(read.grown, 0u);
    ASSERT_TRUE(has_payload);
    const ActorStateDecodeResult result = decoder.Decode(payload.data(), payload.size());
    ASSERT_TRUE(result.IsOk()) << ActorStateDecodeStatusName(result.status);
    ASSERT_TRUE(SameState(decoder.State(), state));
    const auto *wheels = decoder.State().FindSection(ActorStateSection::Wheels);
    ASSERT_TRUE(wheels->IsPresent(0u));
    ASSERT_EQ(wheels->Size(0u), 0u);
    ASSERT_FALSE(wheels->IsPresent(3u));
  }
}
