// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Uses only the header-only codec and raw bytes: test/common is built into the
// server test binary too, which does not link the client.

#include "test.h"

#include <carla/sensor/s11n/EpisodeStateCodec.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <vector>

namespace codec = carla::sensor::s11n::episode_codec;

using Bytes = std::vector<uint8_t>;

namespace {

  Bytes MakeRecord(uint32_t id, uint8_t fill) {
    Bytes record(codec::kRecordSize, fill);
    std::memcpy(record.data(), &id, sizeof(id));
    return record;
  }

  Bytes Concat(const std::vector<Bytes> &records) {
    Bytes frame;
    for (const auto &record : records) {
      frame.insert(frame.end(), record.begin(), record.end());
    }
    return frame;
  }

  size_t CountOf(const Bytes &frame) {
    return frame.size() / codec::kRecordSize;
  }

  size_t GroupOf(size_t byte) {
    for (size_t g = 0u; g < codec::kGroupCount; ++g) {
      if (byte >= codec::kGroups[g].begin && byte < codec::kGroups[g].end) {
        return g;
      }
    }
    return codec::kGroupCount;
  }

  /// A snapshot body as the server collects it: a header with the episode
  /// fields, then the records in registry order.
  Bytes MakeBody(const Bytes &records, uint64_t episode_id = 1u) {
    codec::Header header;
    header.episode_id = episode_id;
    header.platform_timestamp = 12.5;
    header.delta_seconds = 0.05f;
    header.map_origin = carla::geom::Vector3DInt{1, -2, 3};
    Bytes body(codec::kHeaderSize);
    codec::WriteHeader(body.data(), header);
    body.insert(body.end(), records.begin(), records.end());
    return body;
  }

  /// The two messages the server can send for one body: the keyframe, and
  /// the delta when there is one (empty otherwise).
  struct Encoded {
    Bytes keyframe;
    Bytes delta;
  };

  Encoded Encode(codec::DeltaEncoder &encoder, Bytes body, uint64_t sequence) {
    Encoded encoded;
    encoder.Prepare(body.data(), body.size(), sequence);
    if (encoder.HasDelta()) {
      encoded.delta.resize(encoder.GetDeltaSize());
      EXPECT_EQ(encoder.WriteDelta(body.data(), encoded.delta.data()), encoded.delta.size());
    }
    encoded.keyframe = std::move(body);
    return encoded;
  }

  struct Decoded {
    codec::DecodeStatus status;
    /// The body the client would publish.
    Bytes published;
  };

  Decoded Decode(codec::DeltaDecoder &decoder, const Bytes &message) {
    Bytes out;
    const auto result = decoder.Decode(message.data(), message.size(), [&out](size_t size) {
      out.resize(size);
      return out.data();
    });
    Decoded decoded{result.status, {}};
    if (result.status == codec::DecodeStatus::Keyframe) {
      decoded.published = message;
    } else if (result.status == codec::DecodeStatus::Delta) {
      decoded.published = std::move(out);
    }
    return decoded;
  }

  /// The records of a body, ascending by id.
  Bytes SortedRecords(const Bytes &body) {
    std::vector<Bytes> records;
    for (size_t at = codec::kHeaderSize; at + codec::kRecordSize <= body.size(); at += codec::kRecordSize) {
      records.emplace_back(body.begin() + static_cast<long>(at), body.begin() + static_cast<long>(at + codec::kRecordSize));
    }
    std::sort(records.begin(), records.end(), [](const Bytes &a, const Bytes &b) {
      return codec::ReadId(a.data()) < codec::ReadId(b.data());
    });
    return Concat(records);
  }

  template <typename T>
  T ReadAt(const Bytes &bytes, size_t offset) {
    T value;
    std::memcpy(&value, bytes.data() + offset, sizeof(T));
    return value;
  }

} // namespace

TEST(episode_state_codec, groups_cover_every_byte_after_the_id_once) {
  ASSERT_EQ(codec::kRecordSize, 131u);
  for (size_t byte = 0u; byte < codec::kIdSize; ++byte) {
    EXPECT_EQ(GroupOf(byte), codec::kGroupCount) << "id byte " << byte;
  }
  for (size_t byte = codec::kIdSize; byte < codec::kRecordSize; ++byte) {
    EXPECT_LT(GroupOf(byte), codec::kGroupCount) << "byte " << byte;
  }
  size_t total = 0u;
  for (const auto &group : codec::kGroups) {
    total += group.end - group.begin;
  }
  EXPECT_EQ(total, codec::kRecordSize - codec::kIdSize);
  EXPECT_EQ(codec::GroupBytes(codec::kAllGroups), codec::kRecordSize - codec::kIdSize);
  // A moving vehicle: pose, motion and control.
  EXPECT_EQ(codec::GroupBytes(0b001110), 36u + 36u + 19u);
}

TEST(episode_state_codec, each_byte_changes_only_its_group) {
  const Bytes before = MakeRecord(7u, 0x5a);
  for (size_t byte = 0u; byte < codec::kRecordSize; ++byte) {
    Bytes after = before;
    after[byte] ^= 0x01;
    const uint8_t mask = codec::ChangedGroups(before.data(), after.data());
    const size_t group = GroupOf(byte);
    if (group == codec::kGroupCount) {
      EXPECT_EQ(mask, 0u) << "id byte " << byte;
    } else {
      EXPECT_EQ(mask, 1u << group) << "byte " << byte;
    }
  }
}

TEST(episode_state_codec, floats_are_compared_bit_for_bit) {
  const size_t velocity = codec::kGroups[2].begin;
  Bytes a = MakeRecord(1u, 0u);
  Bytes b = a;
  const float positive_zero = 0.0f;
  const float negative_zero = -0.0f;
  std::memcpy(a.data() + velocity, &positive_zero, sizeof(float));
  std::memcpy(b.data() + velocity, &negative_zero, sizeof(float));
  EXPECT_EQ(codec::ChangedGroups(a.data(), b.data()), 1u << 2u);

  const float quiet_nan = std::numeric_limits<float>::quiet_NaN();
  uint32_t other_nan_bits;
  std::memcpy(&other_nan_bits, &quiet_nan, sizeof(float));
  other_nan_bits ^= 0x1u;  // still a NaN, other payload
  std::memcpy(a.data() + velocity, &quiet_nan, sizeof(float));
  std::memcpy(b.data() + velocity, &other_nan_bits, sizeof(float));
  EXPECT_EQ(codec::ChangedGroups(a.data(), b.data()), 1u << 2u);

  // The same NaN bits are no change, although NaN != NaN.
  std::memcpy(b.data() + velocity, &quiet_nan, sizeof(float));
  EXPECT_EQ(codec::ChangedGroups(a.data(), b.data()), 0u);
}

TEST(episode_state_codec, first_frame_has_no_base) {
  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  const Bytes frame = Concat({MakeRecord(30u, 1), MakeRecord(10u, 2), MakeRecord(20u, 3)});
  EXPECT_FALSE(differ.HasBase());
  EXPECT_EQ(differ.Next(frame.data(), CountOf(frame), diff), codec::FrameDiffer::Result::NoBase);
  EXPECT_TRUE(differ.HasBase());
  // Every actor is added, in ascending id order.
  EXPECT_EQ(diff.added, (std::vector<uint32_t>{1u, 2u, 0u}));
  EXPECT_TRUE(diff.removed.empty());
  EXPECT_TRUE(diff.survivors.empty());
  // The base is the frame in id order.
  EXPECT_EQ(differ.GetBase(), Concat({MakeRecord(10u, 2), MakeRecord(20u, 3), MakeRecord(30u, 1)}));
}

TEST(episode_state_codec, unchanged_frame_costs_only_headers_and_bitmap) {
  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  std::vector<Bytes> records;
  for (uint32_t id = 1u; id <= 1000u; ++id) {
    records.push_back(MakeRecord(id, static_cast<uint8_t>(id)));
  }
  const Bytes frame = Concat(records);
  differ.Next(frame.data(), CountOf(frame), diff);
  ASSERT_EQ(differ.Next(frame.data(), CountOf(frame), diff), codec::FrameDiffer::Result::Diffed);
  EXPECT_EQ(diff.survivors.size(), 1000u);
  EXPECT_TRUE(std::all_of(diff.masks.begin(), diff.masks.end(), [](uint8_t m) { return m == 0u; }));
  EXPECT_EQ(diff.DeltaSize(), codec::kHeaderSize + 8u + 125u);
  EXPECT_EQ(codec::KeyframeSize(1000u), codec::kHeaderSize + 131000u);

  // The registry may list the same actors in another order.
  std::reverse(records.begin(), records.end());
  const Bytes reordered = Concat(records);
  ASSERT_EQ(differ.Next(reordered.data(), CountOf(reordered), diff), codec::FrameDiffer::Result::Diffed);
  EXPECT_TRUE(diff.removed.empty());
  EXPECT_TRUE(diff.added.empty());
  EXPECT_TRUE(std::all_of(diff.masks.begin(), diff.masks.end(), [](uint8_t m) { return m == 0u; }));
}

TEST(episode_state_codec, removed_added_and_changed_actors) {
  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  const Bytes first = Concat({MakeRecord(1u, 0), MakeRecord(2u, 0), MakeRecord(3u, 0)});
  differ.Next(first.data(), CountOf(first), diff);

  Bytes moved = MakeRecord(3u, 0);
  moved[codec::kGroups[1].begin] = 1;  // location
  moved[codec::kGroups[3].begin] = 1;  // control
  const Bytes second = Concat({MakeRecord(4u, 9), moved, MakeRecord(1u, 0)});
  ASSERT_EQ(differ.Next(second.data(), CountOf(second), diff), codec::FrameDiffer::Result::Diffed);
  EXPECT_EQ(diff.removed, (std::vector<uint32_t>{2u}));
  EXPECT_EQ(diff.added, (std::vector<uint32_t>{0u}));         // id 4 is at index 0
  EXPECT_EQ(diff.survivors, (std::vector<uint32_t>{2u, 1u}));  // ids 1 and 3
  EXPECT_EQ(diff.masks, (std::vector<uint8_t>{0u, 0b001010}));
  EXPECT_EQ(diff.DeltaSize(),
      codec::kHeaderSize + 8u + 4u + 131u + 1u + (1u + 36u + 19u));
}

TEST(episode_state_codec, repeated_id_drops_the_base) {
  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  const Bytes good = Concat({MakeRecord(1u, 0), MakeRecord(2u, 0)});
  differ.Next(good.data(), CountOf(good), diff);
  const Bytes bad = Concat({MakeRecord(1u, 0), MakeRecord(1u, 1)});
  EXPECT_EQ(differ.Next(bad.data(), CountOf(bad), diff), codec::FrameDiffer::Result::DuplicateId);
  EXPECT_FALSE(differ.HasBase());
  EXPECT_EQ(differ.Next(good.data(), CountOf(good), diff), codec::FrameDiffer::Result::NoBase);
}

TEST(episode_state_codec, empty_frames) {
  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  EXPECT_EQ(differ.Next(nullptr, 0u, diff), codec::FrameDiffer::Result::NoBase);
  EXPECT_EQ(differ.Next(nullptr, 0u, diff), codec::FrameDiffer::Result::Diffed);
  EXPECT_EQ(diff.DeltaSize(), codec::kHeaderSize + 8u);
}

// Random frames, as the registry would give them: actors move, appear,
// disappear, skip one frame (World Partition), come back with a reused id,
// and the registry order changes. Applying each diff to the previous base
// must give exactly the new frame, and every mask must be the true change.
TEST(episode_state_codec, random_chain_diff_rebuilds_every_frame) {
  std::mt19937 rng(20261121u);
  std::uniform_int_distribution<int> byte(0, 255);
  std::uniform_real_distribution<double> unit(0.0, 1.0);

  std::map<uint32_t, Bytes> actors;
  uint32_t next_id = 1u;
  auto spawn = [&]() {
    Bytes record = MakeRecord(next_id, 0u);
    for (size_t b = codec::kIdSize; b < codec::kRecordSize; ++b) {
      record[b] = static_cast<uint8_t>(byte(rng));
    }
    actors[next_id++] = record;
  };
  for (int i = 0; i < 200; ++i) {
    spawn();
  }

  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  Bytes base;  // the previous frame, in id order, as the differ keeps it
  std::vector<uint32_t> registry_order;

  for (int frame_number = 0; frame_number < 2000; ++frame_number) {
    // Mutate the world.
    for (auto &[id, record] : actors) {
      const double r = unit(rng);
      if (r < 0.3) {
        for (size_t g = 0u; g < codec::kGroupCount; ++g) {
          if (unit(rng) < 0.5) {
            record[codec::kGroups[g].begin] = static_cast<uint8_t>(byte(rng));
          }
        }
      }
    }
    if (unit(rng) < 0.2 && !actors.empty()) {
      auto it = actors.begin();
      std::advance(it, static_cast<long>(rng() % actors.size()));
      actors.erase(it);
    }
    if (unit(rng) < 0.2) {
      spawn();
    }
    if (unit(rng) < 0.05 && !actors.empty()) {
      // Id reuse: an actor with an old id comes back with other content.
      const uint32_t reused = 1u + static_cast<uint32_t>(rng() % (next_id - 1u));
      Bytes record = MakeRecord(reused, static_cast<uint8_t>(byte(rng)));
      actors[reused] = record;
    }

    // The frame lists the actors in a registry order that changes sometimes,
    // and skips some for one frame.
    std::vector<uint32_t> ids;
    for (const auto &pair : actors) {
      ids.push_back(pair.first);
    }
    if (registry_order.size() != ids.size() || unit(rng) < 0.1) {
      registry_order = ids;
      std::shuffle(registry_order.begin(), registry_order.end(), rng);
    } else {
      registry_order.clear();
      registry_order = ids;
      std::shuffle(registry_order.begin(), registry_order.end(), std::mt19937(7u));
    }
    std::vector<Bytes> listed;
    for (const uint32_t id : registry_order) {
      if (unit(rng) < 0.02) {
        continue;  // streamed out this frame
      }
      listed.push_back(actors[id]);
    }
    const Bytes frame = Concat(listed);
    const size_t count = CountOf(frame);
    const auto result = differ.Next(frame.data(), count, diff);

    // Expected: the frame in id order.
    std::vector<Bytes> sorted = listed;
    std::sort(sorted.begin(), sorted.end(), [](const Bytes &a, const Bytes &b) {
      return codec::ReadId(a.data()) < codec::ReadId(b.data());
    });
    const Bytes expected = Concat(sorted);
    ASSERT_EQ(differ.GetBase(), expected) << "frame " << frame_number;

    if (frame_number == 0) {
      ASSERT_EQ(result, codec::FrameDiffer::Result::NoBase);
    } else {
      ASSERT_EQ(result, codec::FrameDiffer::Result::Diffed) << "frame " << frame_number;
      // Rebuild the frame from the previous base and the diff alone.
      std::map<uint32_t, Bytes> rebuilt;
      for (size_t k = 0u; k < CountOf(base); ++k) {
        const uint8_t *record = base.data() + k * codec::kRecordSize;
        rebuilt[codec::ReadId(record)] = Bytes(record, record + codec::kRecordSize);
      }
      for (const uint32_t id : diff.removed) {
        ASSERT_EQ(rebuilt.erase(id), 1u) << "removed id " << id << " was not in the base";
      }
      for (const uint32_t index : diff.added) {
        const uint8_t *record = frame.data() + index * codec::kRecordSize;
        ASSERT_TRUE(rebuilt.emplace(codec::ReadId(record), Bytes(record, record + codec::kRecordSize)).second);
      }
      ASSERT_EQ(diff.survivors.size(), diff.masks.size());
      for (size_t k = 0u; k < diff.survivors.size(); ++k) {
        const uint8_t *record = frame.data() + diff.survivors[k] * codec::kRecordSize;
        auto it = rebuilt.find(codec::ReadId(record));
        ASSERT_NE(it, rebuilt.end());
        ASSERT_EQ(diff.masks[k], codec::ChangedGroups(it->second.data(), record));
        for (size_t g = 0u; g < codec::kGroupCount; ++g) {
          if ((diff.masks[k] & (1u << g)) != 0u) {
            std::memcpy(it->second.data() + codec::kGroups[g].begin,
                record + codec::kGroups[g].begin,
                codec::kGroups[g].end - codec::kGroups[g].begin);
          }
        }
      }
      Bytes rebuilt_frame;
      for (const auto &pair : rebuilt) {
        rebuilt_frame.insert(rebuilt_frame.end(), pair.second.begin(), pair.second.end());
      }
      ASSERT_EQ(rebuilt_frame, expected) << "frame " << frame_number;
      ASSERT_LE(diff.DeltaSize(), codec::kHeaderSize + 8u + 4u * diff.removed.size() +
          codec::kRecordSize * diff.added.size() + diff.survivors.size() * codec::kRecordSize);
    }
    base = expected;
  }
}

// -- Hash ---------------------------------------------------------------------

// XXH64 with seed 0; the values come from the reference implementation
// (python xxhash 3.x), so that every toolchain is checked against it.
TEST(episode_state_codec, record_hash_is_xxh64) {
  const Bytes record = MakeRecord(7u, 0x5a);
  Bytes counting(codec::kRecordSize);
  for (size_t b = 0u; b < counting.size(); ++b) {
    counting[b] = static_cast<uint8_t>(b);
  }
  EXPECT_EQ(codec::RecordHash(record.data()), 0xa3d51313a2071b94ull);
  EXPECT_EQ(codec::RecordHash(counting.data()), 0x4a044ef7ee417a45ull);
  // The frame hash is the sum, in any order.
  const Bytes ab = Concat({record, counting});
  const Bytes ba = Concat({counting, record});
  EXPECT_EQ(codec::StateHash(ab.data(), 2u), 0xedd9620b904895d9ull);
  EXPECT_EQ(codec::StateHash(ba.data(), 2u), 0xedd9620b904895d9ull);
  EXPECT_EQ(codec::StateHash(nullptr, 0u), 0u);
}

TEST(episode_state_codec, differ_keeps_the_frame_hash) {
  codec::FrameDiffer differ;
  codec::FrameDiff diff;
  std::vector<Bytes> records = {MakeRecord(3u, 1), MakeRecord(1u, 2), MakeRecord(2u, 3)};
  Bytes frame = Concat(records);
  differ.Next(frame.data(), 3u, diff);
  EXPECT_EQ(differ.GetHash(), codec::StateHash(frame.data(), 3u));
  records[0][codec::kGroups[1].begin] ^= 0xff;
  records.push_back(MakeRecord(9u, 4));
  records.erase(records.begin() + 1);
  frame = Concat(records);
  differ.Next(frame.data(), 3u, diff);
  EXPECT_EQ(differ.GetHash(), codec::StateHash(frame.data(), 3u));
}

// -- Wire format --------------------------------------------------------------

TEST(episode_state_codec, header_layout_is_fixed) {
  codec::DeltaEncoder encoder;
  Bytes body = MakeBody(Concat({MakeRecord(5u, 1), MakeRecord(4u, 2)}), 77u);
  encoder.Prepare(body.data(), body.size(), 9u);
  EXPECT_EQ(Bytes(body.begin(), body.begin() + 4), (Bytes{'C', 'E', 'P', 'S'}));
  EXPECT_EQ(ReadAt<uint16_t>(body, 4u), 2u);                 // version
  EXPECT_EQ(body[6], 0u);                                    // kind: keyframe
  EXPECT_EQ(body[7], 0u);                                    // flags
  EXPECT_EQ(ReadAt<uint64_t>(body, 8u), 77u);                // episode id
  EXPECT_EQ(ReadAt<double>(body, 16u), 12.5);                // platform timestamp
  EXPECT_EQ(ReadAt<float>(body, 24u), 0.05f);                // delta seconds
  EXPECT_EQ(ReadAt<int32_t>(body, 28u), 1);                  // map origin
  EXPECT_EQ(ReadAt<int32_t>(body, 32u), -2);
  EXPECT_EQ(ReadAt<int32_t>(body, 36u), 3);
  EXPECT_EQ(ReadAt<uint32_t>(body, 40u), 0u);                // simulation state
  EXPECT_EQ(ReadAt<uint32_t>(body, 44u), 2u);                // actor count
  EXPECT_EQ(ReadAt<uint32_t>(body, 48u), 2u * 131u);         // payload size
  EXPECT_EQ(ReadAt<uint64_t>(body, 52u), 9u);                // sequence
  EXPECT_EQ(ReadAt<uint64_t>(body, 60u), 0u);                // base sequence
  EXPECT_EQ(ReadAt<uint64_t>(body, 68u), codec::StateHash(body.data() + 76u, 2u));
  // The records stay in registry order, untouched.
  EXPECT_EQ(Bytes(body.begin() + 76, body.end()), Concat({MakeRecord(5u, 1), MakeRecord(4u, 2)}));
}

TEST(episode_state_codec, delta_layout_is_fixed) {
  codec::DeltaEncoder encoder;
  Encode(encoder, MakeBody(Concat({MakeRecord(1u, 0), MakeRecord(2u, 0), MakeRecord(3u, 0)})), 1u);
  Bytes moved = MakeRecord(3u, 0);
  moved[codec::kGroups[1].begin] = 0x11;  // location
  moved[codec::kGroups[3].begin] = 0x33;  // control
  const auto encoded = Encode(encoder, MakeBody(Concat({MakeRecord(4u, 9), moved, MakeRecord(1u, 0)})), 2u);
  ASSERT_FALSE(encoded.delta.empty());

  Bytes expected(encoded.keyframe.begin(), encoded.keyframe.begin() + 76);
  expected[6] = 1u;                                          // kind: delta
  const uint32_t payload = 8u + 4u + 131u + 1u + 1u + 36u + 19u;
  std::memcpy(expected.data() + 48u, &payload, 4u);
  const uint64_t base_sequence = 1u;
  std::memcpy(expected.data() + 60u, &base_sequence, 8u);
  auto append = [&expected](const auto &value) {
    const auto *begin = reinterpret_cast<const uint8_t *>(&value);
    expected.insert(expected.end(), begin, begin + sizeof(value));
  };
  append(uint32_t(1u));                                      // removed
  append(uint32_t(1u));                                      // added
  append(uint32_t(2u));                                      // removed ids
  const Bytes added = MakeRecord(4u, 9);
  expected.insert(expected.end(), added.begin(), added.end());
  expected.push_back(0b10);                                  // survivors 1, 3: 3 changed
  expected.push_back(0b001010);                              // groups 1 and 3
  expected.insert(expected.end(), moved.begin() + 5, moved.begin() + 41);
  expected.insert(expected.end(), moved.begin() + 77, moved.begin() + 96);
  EXPECT_EQ(encoded.delta, expected);
  EXPECT_EQ(ReadAt<uint64_t>(encoded.delta, 68u), ReadAt<uint64_t>(encoded.keyframe, 68u));
}

// -- Encoder ------------------------------------------------------------------

TEST(episode_state_codec, encoder_offers_a_delta_only_against_the_previous_body) {
  const Bytes records = Concat({MakeRecord(1u, 1), MakeRecord(2u, 2), MakeRecord(3u, 3)});
  codec::DeltaEncoder encoder;
  EXPECT_TRUE(Encode(encoder, MakeBody(records), 1u).delta.empty());   // nothing before
  EXPECT_FALSE(Encode(encoder, MakeBody(records), 2u).delta.empty());
  EXPECT_TRUE(Encode(encoder, MakeBody(records), 4u).delta.empty());   // 3 is missing
  EXPECT_FALSE(Encode(encoder, MakeBody(records), 5u).delta.empty());
  EXPECT_TRUE(Encode(encoder, MakeBody(records, 2u), 6u).delta.empty()); // new episode
  EXPECT_FALSE(Encode(encoder, MakeBody(records, 2u), 7u).delta.empty());
  encoder.Reset();
  EXPECT_TRUE(Encode(encoder, MakeBody(records, 2u), 8u).delta.empty());
  EXPECT_FALSE(Encode(encoder, MakeBody(records, 2u), 9u).delta.empty());
  // A repeated id: no delta against it, nor from it.
  const Bytes repeated = Concat({MakeRecord(1u, 1), MakeRecord(1u, 2)});
  const auto bad = Encode(encoder, MakeBody(repeated, 2u), 10u);
  EXPECT_TRUE(bad.delta.empty());
  EXPECT_EQ(ReadAt<uint64_t>(bad.keyframe, 68u), codec::StateHash(bad.keyframe.data() + 76u, 2u));
  EXPECT_TRUE(Encode(encoder, MakeBody(records, 2u), 11u).delta.empty());
}

TEST(episode_state_codec, encoder_sends_no_delta_larger_than_the_keyframe) {
  codec::DeltaEncoder encoder;
  Encode(encoder, MakeBody(MakeRecord(1u, 1)), 1u);
  // One actor whose every group changed: 76 + 8 + 1 + 1 + 127 > 76 + 131.
  EXPECT_TRUE(Encode(encoder, MakeBody(MakeRecord(1u, 2)), 2u).delta.empty());
  // The chain goes on from that keyframe.
  EXPECT_FALSE(Encode(encoder, MakeBody(MakeRecord(1u, 2)), 3u).delta.empty());
}

// -- Decoder ------------------------------------------------------------------

TEST(episode_state_codec, every_group_mask_round_trips) {
  for (uint8_t mask = 1u; mask <= codec::kAllGroups; ++mask) {
    codec::DeltaEncoder encoder;
    codec::DeltaDecoder decoder;
    std::vector<Bytes> records = {MakeRecord(10u, 1), MakeRecord(20u, 2), MakeRecord(30u, 3)};
    const auto first = Encode(encoder, MakeBody(Concat(records)), 1u);
    ASSERT_EQ(Decode(decoder, first.keyframe).status, codec::DecodeStatus::Keyframe);
    for (size_t g = 0u; g < codec::kGroupCount; ++g) {
      if ((mask & (1u << g)) != 0u) {
        records[1][codec::kGroups[g].end - 1u] ^= 0xa5;
      }
    }
    const auto second = Encode(encoder, MakeBody(Concat(records)), 2u);
    ASSERT_FALSE(second.delta.empty()) << int(mask);
    EXPECT_EQ(second.delta.size(), 76u + 8u + 1u + 1u + codec::GroupBytes(mask)) << int(mask);
    const auto decoded = Decode(decoder, second.delta);
    ASSERT_EQ(decoded.status, codec::DecodeStatus::Delta) << int(mask);
    EXPECT_EQ(SortedRecords(decoded.published), Concat(records)) << int(mask);
    // The published body is a keyframe of the same frame.
    Bytes expected_header(second.keyframe.begin(), second.keyframe.begin() + 76);
    EXPECT_EQ(Bytes(decoded.published.begin(), decoded.published.begin() + 76), expected_header);
  }
}

TEST(episode_state_codec, signed_zero_and_nan_payloads_survive_a_delta) {
  codec::DeltaEncoder encoder;
  codec::DeltaDecoder decoder;
  Bytes record = MakeRecord(1u, 0);
  Bytes other = MakeRecord(2u, 0);
  ASSERT_EQ(Decode(decoder, Encode(encoder, MakeBody(Concat({record, other})), 1u).keyframe).status,
      codec::DecodeStatus::Keyframe);
  const size_t velocity = codec::kGroups[2].begin;
  const float negative_zero = -0.0f;
  uint32_t nan_bits = 0x7fc00001u;
  std::memcpy(record.data() + velocity, &negative_zero, sizeof(float));
  std::memcpy(record.data() + velocity + 4u, &nan_bits, sizeof(nan_bits));
  const auto encoded = Encode(encoder, MakeBody(Concat({record, other})), 2u);
  ASSERT_FALSE(encoded.delta.empty());
  const auto decoded = Decode(decoder, encoded.delta);
  ASSERT_EQ(decoded.status, codec::DecodeStatus::Delta);
  EXPECT_EQ(SortedRecords(decoded.published), Concat({record, other}));
}

TEST(episode_state_codec, a_delta_that_does_not_follow_the_base_needs_a_resync) {
  codec::DeltaEncoder encoder;
  std::vector<Encoded> messages;
  Bytes record = MakeRecord(1u, 0);
  for (uint64_t sequence = 1u; sequence <= 5u; ++sequence) {
    record[codec::kGroups[1].begin] = static_cast<uint8_t>(sequence);
    messages.push_back(Encode(encoder, MakeBody(Concat({record, MakeRecord(2u, 0), MakeRecord(3u, 0)})), sequence));
  }
  for (size_t k = 1u; k < messages.size(); ++k) {
    ASSERT_FALSE(messages[k].delta.empty());
  }

  // A decoder that never got a keyframe.
  codec::DeltaDecoder fresh;
  EXPECT_EQ(Decode(fresh, messages[1].delta).status, codec::DecodeStatus::NeedResync);

  codec::DeltaDecoder decoder;
  ASSERT_EQ(Decode(decoder, messages[0].keyframe).status, codec::DecodeStatus::Keyframe);
  // Skipped message: delta 3 against base 1.
  EXPECT_EQ(Decode(decoder, messages[2].delta).status, codec::DecodeStatus::NeedResync);
  EXPECT_FALSE(decoder.HasBase());
  // Without a base nothing applies, not even the right delta.
  EXPECT_EQ(Decode(decoder, messages[1].delta).status, codec::DecodeStatus::NeedResync);
  // A keyframe recovers.
  ASSERT_EQ(Decode(decoder, messages[1].keyframe).status, codec::DecodeStatus::Keyframe);
  ASSERT_EQ(Decode(decoder, messages[2].delta).status, codec::DecodeStatus::Delta);
  // The same delta twice.
  EXPECT_EQ(Decode(decoder, messages[2].delta).status, codec::DecodeStatus::NeedResync);
  // A delta of another episode with the right sequence numbers.
  ASSERT_EQ(Decode(decoder, messages[2].keyframe).status, codec::DecodeStatus::Keyframe);
  Bytes other_episode = messages[3].delta;
  const uint64_t episode_id = 99u;
  std::memcpy(other_episode.data() + 8u, &episode_id, sizeof(episode_id));
  EXPECT_EQ(Decode(decoder, other_episode).status, codec::DecodeStatus::NeedResync);
}

// The client's copy of an actor that the delta does not mention went wrong:
// the delta itself is valid, but the result must not be published.
TEST(episode_state_codec, a_wrong_base_is_caught_by_the_hash) {
  codec::DeltaEncoder encoder;
  const Bytes still = MakeRecord(1u, 4);
  Bytes moving = MakeRecord(2u, 0);
  const auto first = Encode(encoder, MakeBody(Concat({still, moving})), 1u);
  moving[codec::kGroups[1].begin] = 1u;
  const auto second = Encode(encoder, MakeBody(Concat({still, moving})), 2u);
  ASSERT_FALSE(second.delta.empty());

  // A keyframe whose unchanged actor differs, with a hash that matches it.
  Bytes wrong = first.keyframe;
  wrong[76u + 100u] ^= 0x01;  // inside the record of actor 1
  const uint64_t hash = codec::StateHash(wrong.data() + 76u, 2u);
  std::memcpy(wrong.data() + 68u, &hash, sizeof(hash));
  codec::DeltaDecoder decoder;
  ASSERT_EQ(Decode(decoder, wrong).status, codec::DecodeStatus::Keyframe);
  EXPECT_EQ(Decode(decoder, second.delta).status, codec::DecodeStatus::NeedResync);
  EXPECT_FALSE(decoder.HasBase());

  // A keyframe that does not match its own hash is not published either.
  Bytes corrupt = first.keyframe;
  corrupt[76u + 140u] ^= 0x01;
  codec::DeltaDecoder other;
  EXPECT_EQ(Decode(other, corrupt).status, codec::DecodeStatus::NeedResync);
}

TEST(episode_state_codec, another_wire_format_is_incompatible) {
  // A body of the previous format starts with the episode id.
  Bytes old_body(36u + 131u, 0u);
  const uint64_t episode_id = 1u;
  std::memcpy(old_body.data(), &episode_id, sizeof(episode_id));
  codec::DeltaDecoder decoder;
  EXPECT_EQ(Decode(decoder, old_body).status, codec::DecodeStatus::Incompatible);
  EXPECT_EQ(Decode(decoder, Bytes(3u, 0u)).status, codec::DecodeStatus::Incompatible);

  codec::DeltaEncoder encoder;
  Bytes body = Encode(encoder, MakeBody(MakeRecord(1u, 0)), 1u).keyframe;
  body[4] = 3u;  // version 3
  EXPECT_EQ(Decode(decoder, body).status, codec::DecodeStatus::Incompatible);
}

TEST(episode_state_codec, a_keyframe_with_a_repeated_id_is_published_but_is_no_base) {
  codec::DeltaEncoder encoder;
  const auto keyframe = Encode(encoder, MakeBody(Concat({MakeRecord(1u, 1), MakeRecord(1u, 2)})), 1u).keyframe;
  codec::DeltaDecoder decoder;
  EXPECT_EQ(Decode(decoder, keyframe).status, codec::DecodeStatus::Keyframe);
  EXPECT_FALSE(decoder.HasBase());
}

// Every truncation, and random corruption, of a delta: the decoder never
// reads out of bounds (run under ASan), never publishes a frame other than
// the true one, and never keeps a half-applied base.
TEST(episode_state_codec, truncated_and_corrupted_deltas) {
  std::mt19937 rng(7u);
  codec::DeltaEncoder encoder;
  std::vector<Bytes> records;
  for (uint32_t id = 1u; id <= 40u; ++id) {
    records.push_back(MakeRecord(id * 3u, static_cast<uint8_t>(id)));
  }
  const auto first = Encode(encoder, MakeBody(Concat(records)), 1u);
  records.erase(records.begin() + 5);
  records.erase(records.begin() + 17);
  records.push_back(MakeRecord(7u, 0xee));
  records.push_back(MakeRecord(500u, 0xdd));
  for (size_t k = 0u; k < records.size(); k += 3u) {
    records[k][codec::kGroups[k % codec::kGroupCount].begin] ^= 0x5a;
  }
  const auto second = Encode(encoder, MakeBody(Concat(records)), 2u);
  ASSERT_FALSE(second.delta.empty());
  const Bytes expected = SortedRecords(second.keyframe);

  codec::DeltaDecoder primed;
  ASSERT_EQ(Decode(primed, first.keyframe).status, codec::DecodeStatus::Keyframe);
  {
    codec::DeltaDecoder decoder = primed;
    const auto decoded = Decode(decoder, second.delta);
    ASSERT_EQ(decoded.status, codec::DecodeStatus::Delta);
    ASSERT_EQ(SortedRecords(decoded.published), expected);
  }
  for (size_t size = 0u; size < second.delta.size(); ++size) {
    codec::DeltaDecoder decoder = primed;
    // An exact-size copy, so that ASan sees any read past the end.
    std::unique_ptr<uint8_t[]> copy(new uint8_t[size == 0u ? 1u : size]);
    std::memcpy(copy.get(), second.delta.data(), size);
    Bytes out;
    const auto result = decoder.Decode(copy.get(), size, [&out](size_t n) {
      out.resize(n);
      return out.data();
    });
    ASSERT_NE(result.status, codec::DecodeStatus::Delta) << size;
    ASSERT_NE(result.status, codec::DecodeStatus::Keyframe) << size;
    if (result.status == codec::DecodeStatus::NeedResync) {
      ASSERT_FALSE(decoder.HasBase());
      ASSERT_TRUE(decoder.GetBase().empty());
    }
  }
  std::uniform_int_distribution<size_t> position(0u, second.delta.size() - 1u);
  for (int trial = 0; trial < 20000; ++trial) {
    codec::DeltaDecoder decoder = primed;
    Bytes corrupted = second.delta;
    const int flips = 1 + trial % 3;
    for (int f = 0; f < flips; ++f) {
      corrupted[position(rng)] ^= static_cast<uint8_t>(1u << (rng() % 8u));
    }
    const auto decoded = Decode(decoder, corrupted);
    if (decoded.status == codec::DecodeStatus::Delta) {
      // Only bytes that do not reach the records (the timestamps) may change.
      ASSERT_EQ(SortedRecords(decoded.published), expected) << trial;
    } else if (decoded.status == codec::DecodeStatus::NeedResync) {
      ASSERT_TRUE(decoder.GetBase().empty()) << trial;
    }
  }
}

// The whole chain, as a server would send it to two clients, one that always
// gets the keyframe and one that gets the delta whenever there is one: both
// publish exactly the server's frame, every frame.
TEST(episode_state_codec, random_chain_of_10000_frames_decodes_bit_for_bit) {
  std::mt19937 rng(20261122u);
  std::uniform_int_distribution<int> byte(0, 255);
  std::uniform_real_distribution<double> unit(0.0, 1.0);

  std::map<uint32_t, Bytes> actors;
  uint32_t next_id = 1u;
  auto spawn = [&]() {
    Bytes record = MakeRecord(next_id, 0u);
    for (size_t b = codec::kIdSize; b < codec::kRecordSize; ++b) {
      record[b] = static_cast<uint8_t>(byte(rng));
    }
    actors[next_id++] = record;
  };
  for (int i = 0; i < 150; ++i) {
    spawn();
  }

  codec::DeltaEncoder encoder;
  codec::DeltaDecoder by_keyframe;
  codec::DeltaDecoder by_delta;
  std::vector<uint32_t> registry_order;
  uint64_t episode_id = 1u;
  size_t deltas = 0u;
  size_t delta_bytes = 0u;
  size_t keyframe_bytes = 0u;

  for (uint64_t sequence = 1u; sequence <= 10000u; ++sequence) {
    for (auto &[id, record] : actors) {
      const double r = unit(rng);
      if (r < 0.3) {
        // Moving: pose and motion, sometimes control.
        record[codec::kGroups[1].begin + rng() % 36u] = static_cast<uint8_t>(byte(rng));
        record[codec::kGroups[2].begin + rng() % 36u] = static_cast<uint8_t>(byte(rng));
        if (unit(rng) < 0.3) {
          record[codec::kGroups[3].begin + rng() % 19u] = static_cast<uint8_t>(byte(rng));
        }
      } else if (r < 0.32) {
        const size_t g = rng() % codec::kGroupCount;
        record[codec::kGroups[g].begin] = static_cast<uint8_t>(byte(rng));
      }
    }
    if (unit(rng) < 0.1 && !actors.empty()) {
      auto it = actors.begin();
      std::advance(it, static_cast<long>(rng() % actors.size()));
      actors.erase(it);
    }
    if (unit(rng) < 0.1) {
      spawn();
    }
    if (unit(rng) < 0.02) {
      const uint32_t reused = 1u + static_cast<uint32_t>(rng() % (next_id - 1u));
      actors[reused] = MakeRecord(reused, static_cast<uint8_t>(byte(rng)));
    }
    if (unit(rng) < 0.002) {
      ++episode_id;  // load_world
    }
    std::vector<uint32_t> ids;
    for (const auto &pair : actors) {
      ids.push_back(pair.first);
    }
    if (registry_order.size() != ids.size() || unit(rng) < 0.05) {
      registry_order = ids;
      std::shuffle(registry_order.begin(), registry_order.end(), rng);
    }
    std::vector<Bytes> listed;
    for (const uint32_t id : registry_order) {
      if (actors.count(id) == 0u || unit(rng) < 0.01) {
        continue;  // gone, or streamed out this frame
      }
      listed.push_back(actors[id]);
    }
    const auto encoded = Encode(encoder, MakeBody(Concat(listed), episode_id), sequence);
    const Bytes expected = SortedRecords(encoded.keyframe);

    const auto from_keyframe = Decode(by_keyframe, encoded.keyframe);
    ASSERT_EQ(from_keyframe.status, codec::DecodeStatus::Keyframe) << sequence;
    ASSERT_EQ(from_keyframe.published, encoded.keyframe) << sequence;

    const bool send_delta = !encoded.delta.empty();
    const auto from_delta = Decode(by_delta, send_delta ? encoded.delta : encoded.keyframe);
    ASSERT_EQ(from_delta.status,
        send_delta ? codec::DecodeStatus::Delta : codec::DecodeStatus::Keyframe) << sequence;
    ASSERT_EQ(SortedRecords(from_delta.published), expected) << sequence;
    ASSERT_EQ(Bytes(from_delta.published.begin(), from_delta.published.begin() + 76),
              Bytes(encoded.keyframe.begin(), encoded.keyframe.begin() + 76)) << sequence;
    ASSERT_EQ(by_delta.GetBase(), by_keyframe.GetBase()) << sequence;
    if (send_delta) {
      ++deltas;
      delta_bytes += encoded.delta.size();
      keyframe_bytes += encoded.keyframe.size();
    }
  }
  // Most frames go as deltas, and they are smaller.
  EXPECT_GT(deltas, 9000u);
  EXPECT_LT(delta_bytes, keyframe_bytes / 2u);
}
