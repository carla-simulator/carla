// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/rpc/ActorUpdateBatch.h>

#include <cmath>
#include <cstring>
#include <limits>

using namespace carla::rpc;

namespace {

  std::vector<ActorUpdateSection> Parse(const std::vector<uint8_t> &blob, std::string *error_out = nullptr) {
    std::vector<ActorUpdateSection> sections;
    std::string error;
    const bool ok = ParseActorUpdateBatch(blob.data(), blob.size(), sections, error);
    if (error_out != nullptr) {
      *error_out = error;
    }
    EXPECT_EQ(ok, error.empty());
    return sections;
  }

  bool Rejects(const std::vector<uint8_t> &blob) {
    std::vector<ActorUpdateSection> sections;
    std::string error;
    const bool ok = ParseActorUpdateBatch(blob.data(), blob.size(), sections, error);
    return !ok && !error.empty() && sections.empty();
  }

  template <typename T>
  void Poke(std::vector<uint8_t> &blob, size_t offset, T value) {
    std::memcpy(blob.data() + offset, &value, sizeof(T));
  }

  std::vector<uint8_t> SampleBlob() {
    ActorUpdateBatchBuilder builder;
    const double location[3] = {7000.123456789, -6500.000000001, 2.5};
    const float rotation[3] = {1.0f, -90.5f, 0.25f};
    const float velocity[3] = {3.0f, 0.0f, -1.0f};
    builder.AddTargetVelocity(9u, velocity);
    builder.AddTransform(5u, location, rotation);
    builder.AddTransform(6u, location, rotation);
    builder.AddSimulatePhysics(5u, false);
    builder.AddVehicleControl(5u, 0.5f, -0.25f, 0.0f, true, false, true, -1);
    builder.AddTrafficLightState(40u, 2u);
    return builder.Finish();
  }

} // namespace

TEST(actor_update_batch, round_trip_keeps_values_and_kind_order) {
  const auto blob = SampleBlob();
  const auto sections = Parse(blob);
  ASSERT_EQ(sections.size(), 5u);

  // Application order is the order of the kinds, not the order of the calls.
  EXPECT_EQ(sections[0].kind, ActorUpdateKind::SimulatePhysics);
  EXPECT_EQ(sections[1].kind, ActorUpdateKind::Transform);
  EXPECT_EQ(sections[2].kind, ActorUpdateKind::TargetVelocity);
  EXPECT_EQ(sections[3].kind, ActorUpdateKind::VehicleControl);
  EXPECT_EQ(sections[4].kind, ActorUpdateKind::TrafficLightState);

  EXPECT_EQ(sections[0].count, 1u);
  EXPECT_EQ(sections[0].Id(0u), 5u);
  EXPECT_EQ(sections[0].Record(0u)[0], 0u);

  // Positions keep every bit of their double, far from the origin.
  const auto &transforms = sections[1];
  ASSERT_EQ(transforms.count, 2u);
  EXPECT_EQ(transforms.Id(0u), 5u);
  EXPECT_EQ(transforms.Id(1u), 6u);
  for (uint32_t i = 0u; i < 2u; ++i) {
    const uint8_t *record = transforms.Record(i);
    EXPECT_EQ(ReadActorUpdateField<double>(record, 0u), 7000.123456789);
    EXPECT_EQ(ReadActorUpdateField<double>(record, 8u), -6500.000000001);
    EXPECT_EQ(ReadActorUpdateField<double>(record, 16u), 2.5);
    EXPECT_EQ(ReadActorUpdateField<float>(record, 24u), 1.0f);
    EXPECT_EQ(ReadActorUpdateField<float>(record, 28u), -90.5f);
    EXPECT_EQ(ReadActorUpdateField<float>(record, 32u), 0.25f);
  }

  const uint8_t *control = sections[3].Record(0u);
  EXPECT_EQ(ReadActorUpdateField<float>(control, 0u), 0.5f);
  EXPECT_EQ(ReadActorUpdateField<float>(control, 4u), -0.25f);
  EXPECT_EQ(ReadActorUpdateField<float>(control, 8u), 0.0f);
  EXPECT_EQ(control[12], 1u);
  EXPECT_EQ(control[13], 0u);
  EXPECT_EQ(control[14], 1u);
  EXPECT_EQ(ReadActorUpdateField<int32_t>(control, 16u), -1);

  EXPECT_EQ(sections[4].Id(0u), 40u);
  EXPECT_EQ(sections[4].Record(0u)[0], 2u);
}

TEST(actor_update_batch, special_floats_are_kept_bit_for_bit) {
  ActorUpdateBatchBuilder builder;
  const double location[3] = {
      std::numeric_limits<double>::quiet_NaN(), -0.0, std::numeric_limits<double>::denorm_min()};
  const float rotation[3] = {
      std::numeric_limits<float>::infinity(), -0.0f, std::numeric_limits<float>::denorm_min()};
  builder.AddTransform(1u, location, rotation);
  const auto blob = builder.Finish();
  const auto sections = Parse(blob);
  ASSERT_EQ(sections.size(), 1u);
  const uint8_t *record = sections[0].Record(0u);
  EXPECT_EQ(std::memcmp(record, location, sizeof(location)), 0);
  EXPECT_EQ(std::memcmp(record + sizeof(location), rotation, sizeof(rotation)), 0);
}

TEST(actor_update_batch, builder_is_empty_after_finish) {
  ActorUpdateBatchBuilder builder;
  const float velocity[3] = {1.0f, 2.0f, 3.0f};
  builder.AddTargetVelocity(1u, velocity);
  EXPECT_EQ(builder.size(), 1u);
  builder.Finish();
  EXPECT_TRUE(builder.empty());
  const auto blob = builder.Finish();
  EXPECT_EQ(blob.size(), kActorUpdateHeaderSize);
  EXPECT_TRUE(Parse(blob).empty());
}

TEST(actor_update_batch, add_records_matches_typed_adders) {
  ActorUpdateBatchBuilder typed;
  const float a[3] = {1.0f, 2.0f, 3.0f};
  const float b[3] = {-4.0f, 5.5f, 0.0f};
  typed.AddTargetVelocity(3u, a);
  typed.AddTargetVelocity(4u, b);

  ActorUpdateBatchBuilder bulk;
  const uint32_t ids[2] = {3u, 4u};
  float records[6];
  std::memcpy(records, a, sizeof(a));
  std::memcpy(records + 3, b, sizeof(b));
  bulk.AddRecords(
      ActorUpdateKind::TargetVelocity, ids, reinterpret_cast<const uint8_t *>(records), 2u);
  EXPECT_EQ(typed.Finish(), bulk.Finish());
}

TEST(actor_update_batch, rejects_malformed_blobs) {
  const auto good = SampleBlob();
  ASSERT_FALSE(Parse(good).empty());

  // Every truncation.
  for (size_t size = 0u; size < good.size(); ++size) {
    std::vector<uint8_t> truncated(good.begin(), good.begin() + static_cast<std::ptrdiff_t>(size));
    EXPECT_TRUE(Rejects(truncated)) << "size " << size;
    // Also with a header that claims the truncated size.
    if (size >= kActorUpdateHeaderSize) {
      Poke<uint32_t>(truncated, 8u, static_cast<uint32_t>(size));
      EXPECT_TRUE(Rejects(truncated)) << "size " << size << ", patched header";
    }
  }

  {
    auto bad = good;
    bad[0] = 'X';
    EXPECT_TRUE(Rejects(bad));
  }
  {
    auto bad = good;
    Poke<uint16_t>(bad, 4u, kActorUpdateVersion + 1u);
    EXPECT_TRUE(Rejects(bad));
  }
  {
    // Trailing bytes, with the header size matching them.
    auto bad = good;
    bad.push_back(0u);
    Poke<uint32_t>(bad, 8u, static_cast<uint32_t>(bad.size()));
    EXPECT_TRUE(Rejects(bad));
  }
  {
    // First section (SimulatePhysics): unknown kind, wrong stride, a count
    // that runs past the end and one that would overflow 32 bits.
    const size_t section = kActorUpdateHeaderSize;
    auto unknown = good;
    Poke<uint16_t>(unknown, section, 99u);
    EXPECT_TRUE(Rejects(unknown));
    auto stride = good;
    Poke<uint16_t>(stride, section + 2u, 2u);
    EXPECT_TRUE(Rejects(stride));
    auto count = good;
    Poke<uint32_t>(count, section + 4u, 1000u);
    EXPECT_TRUE(Rejects(count));
    auto huge = good;
    Poke<uint32_t>(huge, section + 4u, 0xffffffffu);
    EXPECT_TRUE(Rejects(huge));
  }
  {
    // The same kind twice, or out of order.
    auto bad = good;
    const size_t first = kActorUpdateHeaderSize;
    const size_t second = first + kActorUpdateSectionHeaderSize + sizeof(uint32_t) + 1u;
    Poke<uint16_t>(bad, second, static_cast<uint16_t>(ActorUpdateKind::SimulatePhysics));
    Poke<uint16_t>(bad, second + 2u, ActorUpdateStride(ActorUpdateKind::SimulatePhysics));
    EXPECT_TRUE(Rejects(bad));
  }
  {
    // More sections announced than present.
    auto bad = good;
    Poke<uint16_t>(bad, 6u, 6u);
    EXPECT_TRUE(Rejects(bad));
  }
}
