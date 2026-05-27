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
