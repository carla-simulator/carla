// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/MsgPack.h>
#include <carla/rpc/CustomV2XBytes.h>
#include <carla/sensor/data/V2XData.h>
#include <carla/sensor/s11n/V2XSerializer.h>

#include <cstring>
#include <type_traits>

namespace csd = carla::sensor::data;
namespace css = carla::sensor::s11n;
using carla::rpc::CustomV2XBytes;

// The custom V2X sensor ships a fixed 100-byte binary blob across the RPC
// boundary via MsgPack (client -> server "send" path). These tests pin the
// blob contract, the in-memory message containers, and the byte-for-byte
// fidelity of the V2X serializers that feed the sensor data stream.

TEST(CustomV2XBytes, default_construction_is_empty_with_hundred_byte_cap) {
  CustomV2XBytes blob{};
  EXPECT_EQ(blob.data_size, 0u);
  EXPECT_EQ(blob.max_data_size(), 100u);
  EXPECT_EQ(blob.bytes.max_size(), 100u);
}

TEST(CustomV2XBytes, msgpack_roundtrip_preserves_payload) {
  CustomV2XBytes blob{};
  blob.data_size = 4u;
  blob.bytes[0] = 0xDE;
  blob.bytes[1] = 0xAD;
  blob.bytes[2] = 0xBE;
  blob.bytes[3] = 0xEF;

  auto buffer = carla::MsgPack::Pack(blob);
  auto restored = carla::MsgPack::UnPack<CustomV2XBytes>(buffer);

  EXPECT_EQ(restored.data_size, blob.data_size);
  EXPECT_EQ(restored.bytes, blob.bytes);
}

TEST(CustomV2XBytes, msgpack_roundtrip_preserves_full_capacity_payload) {
  CustomV2XBytes blob{};
  blob.data_size = blob.max_data_size();
  for (uint8_t i = 0; i < blob.data_size; ++i) {
    blob.bytes[i] = static_cast<unsigned char>(i);
  }

  auto buffer = carla::MsgPack::Pack(blob);
  auto restored = carla::MsgPack::UnPack<CustomV2XBytes>(buffer);

  EXPECT_EQ(restored.data_size, 100u);
  EXPECT_EQ(restored.bytes, blob.bytes);
}

TEST(CAMDataContainer, write_message_counts_and_reset_clears) {
  csd::CAMDataS data;
  EXPECT_EQ(data.GetMessageCount(), 0u);

  csd::CAMData first{};
  first.Power = 21.5f;
  first.Message.header.stationID = 7;
  data.WriteMessage(first);

  csd::CAMData second{};
  second.Power = -12.0f;
  second.Message.header.stationID = 9;
  data.WriteMessage(second);

  EXPECT_EQ(data.GetMessageCount(), 2u);

  data.Reset();
  EXPECT_EQ(data.GetMessageCount(), 0u);
}

TEST(CustomV2XDataContainer, write_message_counts_and_reset_clears) {
  csd::CustomV2XDataS data;
  EXPECT_EQ(data.GetMessageCount(), 0u);

  csd::CustomV2XData message{};
  message.Power = 5.0f;
  message.Message.header.stationID = 3;
  message.Message.data.data_size = 2u;
  message.Message.data.bytes[0] = 1u;
  message.Message.data.bytes[1] = 2u;
  data.WriteMessage(message);

  EXPECT_EQ(data.GetMessageCount(), 1u);

  data.Reset();
  EXPECT_EQ(data.GetMessageCount(), 0u);
}

TEST(CAMDataSerializer, serialize_copies_messages_byte_for_byte) {
  csd::CAMDataS data;

  csd::CAMData first{};
  first.Power = 21.5f;
  first.Message.header.stationID = 11;
  data.WriteMessage(first);

  csd::CAMData second{};
  second.Power = 33.25f;
  second.Message.header.stationID = 22;
  data.WriteMessage(second);

  carla::Buffer output = css::CAMDataSerializer::Serialize(0, data, carla::Buffer{});

  ASSERT_EQ(output.size(), 2u * sizeof(csd::CAMData));
  const auto *messages = reinterpret_cast<const csd::CAMData *>(output.data());
  EXPECT_EQ(messages[0].Power, 21.5f);
  EXPECT_EQ(messages[0].Message.header.stationID, 11);
  EXPECT_EQ(messages[1].Power, 33.25f);
  EXPECT_EQ(messages[1].Message.header.stationID, 22);
}

// The CAM wire type must carry the optional pathDeltaTime by value (with an
// availability flag) instead of a raw pointer: CAMDataSerializer memcpys the
// whole message onto the data stream, so a server-side pointer must never reach
// the wire.
TEST(PathPoint, delta_time_is_an_inline_value_not_a_pointer) {
  static_assert(
      std::is_same<decltype(ITSContainer::PathPoint_t::pathDeltaTime),
                   ITSContainer::PathDeltaTime_t>::value,
      "pathDeltaTime must be stored by value so no server pointer reaches the wire");
  static_assert(
      std::is_trivially_copyable<ITSContainer::PathPoint_t>::value,
      "PathPoint_t is copied byte-for-byte by the V2X serializer");

  ITSContainer::PathPoint_t point{};
  EXPECT_FALSE(point.pathDeltaTimeAvailable);
  EXPECT_EQ(point.pathDeltaTime, 0);
}

TEST(CAMDataSerializer, serialize_preserves_inline_path_delta_time) {
  csd::CAMDataS data;

  csd::CAMData message{};
  message.Power = 7.5f;
  message.Message.header.stationID = 99;
  auto &history =
      message.Message.cam.camParameters.lowFrequencyContainer
          .basicVehicleContainerLowFrequency.pathHistory;
  history.NumberOfPathPoint = 1;
  history.data[0].pathDeltaTime = 1234;
  history.data[0].pathDeltaTimeAvailable = true;
  data.WriteMessage(message);

  carla::Buffer output = css::CAMDataSerializer::Serialize(0, data, carla::Buffer{});

  ASSERT_EQ(output.size(), sizeof(csd::CAMData));
  const auto *restored = reinterpret_cast<const csd::CAMData *>(output.data());
  const auto &restored_history =
      restored->Message.cam.camParameters.lowFrequencyContainer
          .basicVehicleContainerLowFrequency.pathHistory;
  EXPECT_EQ(restored_history.NumberOfPathPoint, 1);
  EXPECT_TRUE(restored_history.data[0].pathDeltaTimeAvailable);
  EXPECT_EQ(restored_history.data[0].pathDeltaTime, 1234);
}

TEST(CustomV2XDataSerializer, serialize_preserves_binary_payload) {
  csd::CustomV2XDataS data;

  csd::CustomV2XData message{};
  message.Power = 18.0f;
  message.Message.header.stationID = 42;
  message.Message.data.data_size = 3u;
  message.Message.data.bytes[0] = 0xAA;
  message.Message.data.bytes[1] = 0xBB;
  message.Message.data.bytes[2] = 0xCC;
  data.WriteMessage(message);

  carla::Buffer output = css::CustomV2XDataSerializer::Serialize(0, data, carla::Buffer{});

  ASSERT_EQ(output.size(), sizeof(csd::CustomV2XData));
  const auto *restored = reinterpret_cast<const csd::CustomV2XData *>(output.data());
  EXPECT_EQ(restored->Power, 18.0f);
  EXPECT_EQ(restored->Message.header.stationID, 42);
  EXPECT_EQ(restored->Message.data.data_size, 3u);
  EXPECT_EQ(restored->Message.data.bytes[0], 0xAA);
  EXPECT_EQ(restored->Message.data.bytes[1], 0xBB);
  EXPECT_EQ(restored->Message.data.bytes[2], 0xCC);
}
