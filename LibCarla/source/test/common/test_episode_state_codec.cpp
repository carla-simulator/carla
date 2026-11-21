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
