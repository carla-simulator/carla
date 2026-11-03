// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Buffer.h>
#include <carla/multigpu/commands.h>
#include <carla/multigpu/frameDataCodec.h>
#include <carla/multigpu/frameTime.h>
#include <carla/multigpu/secondaryCommands.h>

#include <chrono>
#include <cstring>
#include <future>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

  using carla::multigpu::BoundedReader;
  using carla::multigpu::FrameDataReadResult;
  using carla::multigpu::FrameDataStatus;
  using carla::multigpu::FrameTime;
  using carla::multigpu::ReadFrameData;
  using carla::multigpu::WriteFrameDataHeader;
  using carla::multigpu::kFrameCounterPacketId;

  /// A packet this test treats as known: one uint32 value.
  constexpr uint8_t kValuePacketId = 2u;

  /// Runs @a task on a detached thread so that a reader that spins fails the
  /// test instead of hanging the whole suite.
  template <typename F>
  auto RunWithWatchdog(F task) -> std::optional<decltype(task())> {
    using Result = decltype(task());
    auto done = std::make_shared<std::promise<Result>>();
    auto future = done->get_future();
    std::thread([done, task = std::move(task)]() mutable {
      try {
        done->set_value(task());
      } catch (...) {
        done->set_exception(std::current_exception());
      }
    }).detach();
    if (future.wait_for(std::chrono::seconds(5)) != std::future_status::ready) {
      return std::nullopt;
    }
    return future.get();
  }

  template <typename T>
  void Put(std::ostream &out, const T &value) {
    out.write(reinterpret_cast<const char *>(&value), sizeof(T));
  }

  void PutValuePacket(std::ostream &out, uint32_t value) {
    WriteFrameDataHeader(out, kValuePacketId, sizeof(value));
    Put(out, value);
  }

  void PutFrameCounter(std::ostream &out, uint64_t frame) {
    WriteFrameDataHeader(out, kFrameCounterPacketId, sizeof(frame));
    Put(out, frame);
  }

  struct Parsed {
    FrameDataReadResult result;
    std::vector<uint32_t> values;
    std::optional<FrameTime> time;
  };

  /// Parses @a bytes the way FFrameData does: value packets and FrameTime are
  /// known, every other id is not.
  Parsed Parse(std::string bytes) {
    auto parsed = RunWithWatchdog([bytes = std::move(bytes)]() {
      Parsed out;
      out.result = ReadFrameData(
          reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size(),
          [&](uint8_t id, std::istream &in, uint32_t size) {
            switch (id) {
              case kValuePacketId: {
                uint32_t value = 0u;
                in.read(reinterpret_cast<char *>(&value), sizeof(value));
                if (!in.fail()) {
                  out.values.push_back(value);
                }
                return true;
              }
              case FrameTime::kPacketId: {
                FrameTime time;
                if (time.Read(in, size)) {
                  out.time = time;
                }
                return true;
              }
              default:
                return false;
            }
          });
      return out;
    });
    EXPECT_TRUE(parsed.has_value()) << "the frame reader did not terminate";
    return parsed.value_or(Parsed{});
  }

  FrameTime SampleFrameTime() {
    FrameTime time;
    time.frame = 223316u;
    time.delta_seconds = 0.05f;
    time.elapsed_game_time = 11.39318;
    time.visual_game_time = 11.39318 + 0.5;
    time.world_time_seconds = 100.125;
    time.world_unpaused_time_seconds = 101.25;
    time.world_real_time_seconds = 102.5;
    time.world_audio_time_seconds = 103.75;
    time.synchronous_mode = true;
    time.no_rendering_mode = true;
    time.fixed_delta_seconds = 0.05;
    time.substepping = false;
    time.max_substep_delta_time = 0.002;
    time.max_substeps = 16;
    time.max_culling_distance = 12.5f;
    time.deterministic_ragdolls = false;
    time.tile_streaming_distance = 123456.0f;
    time.actor_active_distance = 65432.0f;
    time.spectator_as_ego = false;
    time.adjust_signs_height_to_ground = true;
    time.time_dilation = 0.5f;
    time.full_resync = true;
    return time;
  }

} // namespace

TEST(MultiGpuFrameData, complete_frame_is_read_in_order) {
  std::ostringstream out;
  PutValuePacket(out, 7u);
  PutValuePacket(out, 8u);
  PutFrameCounter(out, 42u);
  const Parsed parsed = Parse(out.str());
  EXPECT_TRUE(parsed.result.IsComplete());
  ASSERT_TRUE(parsed.result.frame.has_value());
  EXPECT_EQ(*parsed.result.frame, 42u);
  EXPECT_EQ(parsed.values, (std::vector<uint32_t>{7u, 8u}));
  EXPECT_EQ(parsed.result.skipped, 0u);
  EXPECT_EQ(parsed.result.grown, 0u);
}

TEST(MultiGpuFrameData, unknown_packet_mid_stream_is_skipped) {
  std::ostringstream out;
  PutValuePacket(out, 1u);
  WriteFrameDataHeader(out, 200u, 3u);
  out.write("abc", 3);
  PutValuePacket(out, 2u);
  PutFrameCounter(out, 9u);
  const Parsed parsed = Parse(out.str());
  EXPECT_TRUE(parsed.result.IsComplete());
  EXPECT_EQ(parsed.values, (std::vector<uint32_t>{1u, 2u}));
  EXPECT_EQ(parsed.result.skipped, 1u);
  EXPECT_EQ(parsed.result.first_skipped_id, 200u);
}

TEST(MultiGpuFrameData, unknown_packet_as_the_last_packet_terminates) {
  std::ostringstream out;
  PutFrameCounter(out, 3u);
  WriteFrameDataHeader(out, 201u, 0u);
  const Parsed parsed = Parse(out.str());
  EXPECT_TRUE(parsed.result.IsComplete());
  EXPECT_EQ(parsed.result.skipped, 1u);
}

TEST(MultiGpuFrameData, frame_without_frame_counter_is_incomplete) {
  std::ostringstream out;
  PutValuePacket(out, 5u);
  const Parsed parsed = Parse(out.str());
  EXPECT_EQ(parsed.result.status, FrameDataStatus::MissingFrameCounter);
  EXPECT_FALSE(parsed.result.IsComplete());
  EXPECT_EQ(Parse(std::string{}).result.status, FrameDataStatus::MissingFrameCounter);
}

TEST(MultiGpuFrameData, truncated_header_stops_the_read) {
  std::ostringstream out;
  PutFrameCounter(out, 1u);
  out.write("\x02\x04", 2);
  const Parsed parsed = Parse(out.str());
  EXPECT_EQ(parsed.result.status, FrameDataStatus::TruncatedHeader);
}

TEST(MultiGpuFrameData, truncated_payload_stops_the_read) {
  std::ostringstream out;
  WriteFrameDataHeader(out, kValuePacketId, 4000u);
  Put(out, uint32_t{1u});
  const Parsed parsed = Parse(out.str());
  EXPECT_EQ(parsed.result.status, FrameDataStatus::TruncatedPayload);
  EXPECT_EQ(parsed.result.packet_size, 4000u);
  EXPECT_TRUE(parsed.values.empty());
}

TEST(MultiGpuFrameData, short_known_packet_fails_inside_its_own_packet) {
  std::ostringstream out;
  WriteFrameDataHeader(out, kValuePacketId, 2u);
  out.write("\x01\x02", 2);
  PutValuePacket(out, 77u);
  PutFrameCounter(out, 1u);
  const Parsed parsed = Parse(out.str());
  EXPECT_EQ(parsed.result.status, FrameDataStatus::ShortPacket);
  EXPECT_EQ(parsed.result.packet_id, kValuePacketId);
  EXPECT_EQ(parsed.result.offset, 0u);
  // The reader could not consume the next packet's bytes.
  EXPECT_TRUE(parsed.values.empty());
}

TEST(MultiGpuFrameData, longer_known_packet_is_counted_and_skipped) {
  std::ostringstream out;
  WriteFrameDataHeader(out, kValuePacketId, 8u);
  Put(out, uint32_t{11u});
  Put(out, uint32_t{0xDEADBEEFu});
  PutValuePacket(out, 12u);
  PutFrameCounter(out, 1u);
  const Parsed parsed = Parse(out.str());
  EXPECT_TRUE(parsed.result.IsComplete());
  EXPECT_EQ(parsed.result.grown, 1u);
  EXPECT_EQ(parsed.values, (std::vector<uint32_t>{11u, 12u}));
}

TEST(MultiGpuFrameData, short_frame_counter_is_a_short_packet) {
  std::ostringstream out;
  WriteFrameDataHeader(out, kFrameCounterPacketId, 4u);
  Put(out, uint32_t{1u});
  EXPECT_EQ(Parse(out.str()).result.status, FrameDataStatus::ShortPacket);
}

TEST(MultiGpuFrameData, reader_exception_is_reported) {
  std::ostringstream out;
  PutValuePacket(out, 1u);
  PutFrameCounter(out, 1u);
  const std::string bytes = out.str();
  const auto result = RunWithWatchdog([bytes]() {
    return ReadFrameData(
        reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size(),
        [](uint8_t, std::istream &, uint32_t) -> bool { throw std::length_error("bad count"); });
  });
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->status, FrameDataStatus::ReaderError);
}

TEST(MultiGpuFrameData, nested_counts_read_off_the_wire_terminate) {
  // A recorder-style reader: an outer count of 65535 records, each with an
  // inner count, over a 3-byte payload. Loops of the form (i < Total) && In
  // must end as soon as the payload runs out.
  std::ostringstream out;
  WriteFrameDataHeader(out, kValuePacketId, 3u);
  out.write("\xFF\xFF\x07", 3);
  PutFrameCounter(out, 1u);
  const std::string bytes = out.str();
  const auto result = RunWithWatchdog([bytes]() {
    return ReadFrameData(
        reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size(),
        [](uint8_t, std::istream &in, uint32_t) {
          uint16_t total = 0u;
          in.read(reinterpret_cast<char *>(&total), sizeof(total));
          for (uint16_t i = 0u; (i < total) && in; ++i) {
            uint32_t inner = 0u;
            in.read(reinterpret_cast<char *>(&inner), sizeof(inner));
            for (uint32_t j = 0u; (j < inner) && in; ++j) {
              uint64_t value = 0u;
              in.read(reinterpret_cast<char *>(&value), sizeof(value));
            }
          }
          return true;
        });
  });
  ASSERT_TRUE(result.has_value()) << "the nested reader did not terminate";
  EXPECT_EQ(result->status, FrameDataStatus::ShortPacket);
}

TEST(MultiGpuFrameData, bounded_reader_rejects_lists_larger_than_the_packet) {
  std::ostringstream out;
  Put(out, uint32_t{1000u});
  Put(out, uint16_t{8u});
  Put(out, uint64_t{1u});
  const std::string bytes = out.str();
  std::istringstream in(bytes);
  BoundedReader reader(in, static_cast<uint32_t>(bytes.size()));
  uint32_t count = 0u;
  uint16_t record_size = 0u;
  EXPECT_FALSE(reader.ReadFixedListHeader(8u, count, record_size));
  EXPECT_TRUE(reader.Failed());
  EXPECT_TRUE(in.fail());
}

TEST(MultiGpuFrameData, bounded_reader_skips_what_a_record_grew) {
  std::ostringstream out;
  Put(out, uint32_t{2u});
  Put(out, uint16_t{6u});  // records of 6 bytes; this build reads 4
  Put(out, uint32_t{10u});
  Put(out, uint16_t{0xAAAAu});
  Put(out, uint32_t{20u});
  Put(out, uint16_t{0xBBBBu});
  const std::string bytes = out.str();
  std::istringstream in(bytes);
  BoundedReader reader(in, static_cast<uint32_t>(bytes.size()));
  uint32_t count = 0u;
  uint16_t record_size = 0u;
  ASSERT_TRUE(reader.ReadFixedListHeader(4u, count, record_size));
  std::vector<uint32_t> values;
  for (uint32_t i = 0u; i < count; ++i) {
    ASSERT_TRUE(reader.ReadRecord(record_size, [&](BoundedReader &record) {
      uint32_t value = 0u;
      if (!record.Read(value)) {
        return false;
      }
      values.push_back(value);
      return true;
    }));
  }
  EXPECT_EQ(values, (std::vector<uint32_t>{10u, 20u}));
  EXPECT_EQ(reader.Remaining(), 0u);
  EXPECT_FALSE(reader.Failed());
}

TEST(MultiGpuFrameData, bounded_reader_skips_unknown_fields) {
  std::ostringstream out;
  carla::multigpu::WriteField(out, 9u, std::string("unknown"));
  std::ostringstream known;
  Put(known, uint32_t{1234u});
  carla::multigpu::WriteField(out, 1u, known.str());
  const std::string bytes = out.str();
  std::istringstream in(bytes);
  BoundedReader reader(in, static_cast<uint32_t>(bytes.size()));
  std::optional<uint32_t> value;
  EXPECT_TRUE(reader.ReadFields([&](uint8_t kind, BoundedReader &field) {
    if (kind == 1u) {
      uint32_t v = 0u;
      if (!field.Read(v)) {
        return false;
      }
      value = v;
    }
    return true;
  }));
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, 1234u);
}

TEST(MultiGpuFrameData, bounded_reader_cannot_read_past_its_field) {
  std::ostringstream out;
  carla::multigpu::WriteField(out, 1u, std::string("ab"));
  Put(out, uint32_t{99u});
  const std::string bytes = out.str();
  std::istringstream in(bytes);
  BoundedReader reader(in, static_cast<uint32_t>(bytes.size()));
  EXPECT_FALSE(reader.ReadFields([](uint8_t, BoundedReader &field) {
    uint32_t v = 0u;
    return field.Read(v);  // the field holds 2 bytes only
  }));
}

TEST(MultiGpuFrameTime, round_trips_every_field) {
  const FrameTime sent = SampleFrameTime();
  std::ostringstream out;
  sent.Write(out);
  PutFrameCounter(out, sent.frame);
  const Parsed parsed = Parse(out.str());
  ASSERT_TRUE(parsed.result.IsComplete());
  ASSERT_TRUE(parsed.time.has_value());
  const FrameTime &got = *parsed.time;
  EXPECT_EQ(got.protocol_version, carla::multigpu::kMultiGpuProtocolVersion);
  EXPECT_EQ(got.frame, sent.frame);
  EXPECT_EQ(got.delta_seconds, sent.delta_seconds);
  EXPECT_EQ(got.elapsed_game_time, sent.elapsed_game_time);
  EXPECT_EQ(got.visual_game_time, sent.visual_game_time);
  EXPECT_EQ(got.world_time_seconds, sent.world_time_seconds);
  EXPECT_EQ(got.world_unpaused_time_seconds, sent.world_unpaused_time_seconds);
  EXPECT_EQ(got.world_real_time_seconds, sent.world_real_time_seconds);
  EXPECT_EQ(got.world_audio_time_seconds, sent.world_audio_time_seconds);
  EXPECT_EQ(got.synchronous_mode, sent.synchronous_mode);
  EXPECT_EQ(got.no_rendering_mode, sent.no_rendering_mode);
  EXPECT_EQ(got.fixed_delta_seconds, sent.fixed_delta_seconds);
  EXPECT_EQ(got.substepping, sent.substepping);
  EXPECT_EQ(got.max_substep_delta_time, sent.max_substep_delta_time);
  EXPECT_EQ(got.max_substeps, sent.max_substeps);
  EXPECT_EQ(got.max_culling_distance, sent.max_culling_distance);
  EXPECT_EQ(got.deterministic_ragdolls, sent.deterministic_ragdolls);
  EXPECT_EQ(got.tile_streaming_distance, sent.tile_streaming_distance);
  EXPECT_EQ(got.actor_active_distance, sent.actor_active_distance);
  EXPECT_EQ(got.spectator_as_ego, sent.spectator_as_ego);
  EXPECT_EQ(got.adjust_signs_height_to_ground, sent.adjust_signs_height_to_ground);
  EXPECT_EQ(got.time_dilation, sent.time_dilation);
  EXPECT_EQ(got.full_resync, sent.full_resync);
}

TEST(MultiGpuFrameTime, unset_fixed_delta_round_trips) {
  FrameTime sent = SampleFrameTime();
  sent.fixed_delta_seconds.reset();
  sent.full_resync = false;
  std::ostringstream out;
  sent.Write(out);
  PutFrameCounter(out, sent.frame);
  const Parsed parsed = Parse(out.str());
  ASSERT_TRUE(parsed.time.has_value());
  EXPECT_FALSE(parsed.time->fixed_delta_seconds.has_value());
  EXPECT_FALSE(parsed.time->full_resync);
}

TEST(MultiGpuFrameTime, v1_layout_is_pinned) {
  const FrameTime sent = SampleFrameTime();
  std::ostringstream out;
  sent.Write(out);
  const std::string bytes = out.str();
  ASSERT_EQ(bytes.size(), carla::multigpu::kFrameDataHeaderSize + FrameTime::kPayloadSizeV1);
  ASSERT_EQ(bytes.size(), 103u);
  const char *payload = bytes.data() + carla::multigpu::kFrameDataHeaderSize;
  EXPECT_EQ(static_cast<uint8_t>(bytes[0]), FrameTime::kPacketId);
  uint64_t frame = 0u;
  std::memcpy(&frame, payload + 2, sizeof(frame));
  EXPECT_EQ(frame, sent.frame);
  float delta = 0.0f;
  std::memcpy(&delta, payload + 10, sizeof(delta));
  EXPECT_EQ(delta, sent.delta_seconds);
  double world_time = 0.0;
  std::memcpy(&world_time, payload + 30, sizeof(world_time));
  EXPECT_EQ(world_time, sent.world_time_seconds);
  double fixed_delta = 0.0;
  std::memcpy(&fixed_delta, payload + 62, sizeof(fixed_delta));
  EXPECT_EQ(fixed_delta, *sent.fixed_delta_seconds);
  float dilation = 0.0f;
  std::memcpy(&dilation, payload + 94, sizeof(dilation));
  EXPECT_EQ(dilation, sent.time_dilation);
}

TEST(MultiGpuFrameTime, shorter_than_v1_is_rejected) {
  std::ostringstream out;
  WriteFrameDataHeader(out, FrameTime::kPacketId, 20u);
  out.write(std::string(20u, '\0').data(), 20);
  PutFrameCounter(out, 1u);
  const Parsed parsed = Parse(out.str());
  EXPECT_EQ(parsed.result.status, FrameDataStatus::ShortPacket);
  EXPECT_FALSE(parsed.time.has_value());
}

TEST(MultiGpuFrameTime, longer_is_reported_as_grown) {
  const FrameTime sent = SampleFrameTime();
  std::ostringstream v1;
  sent.Write(v1);
  std::string bytes = v1.str();
  // Rewrite it as a newer build would: the same fields plus 6 more bytes.
  const uint32_t grown_size = FrameTime::kPayloadSizeV1 + 6u;
  std::memcpy(&bytes[1], &grown_size, sizeof(grown_size));
  bytes.append(6u, '\x5A');
  std::ostringstream out;
  out << bytes;
  PutFrameCounter(out, sent.frame);
  const Parsed parsed = Parse(out.str());
  EXPECT_TRUE(parsed.result.IsComplete());
  EXPECT_EQ(parsed.result.grown, 1u);
  ASSERT_TRUE(parsed.time.has_value());
  EXPECT_EQ(parsed.time->time_dilation, sent.time_dilation);
}

TEST(MultiGpuFrameTime, legacy_frame_without_frame_time_is_complete) {
  std::ostringstream out;
  PutValuePacket(out, 1u);
  PutFrameCounter(out, 5u);
  const Parsed parsed = Parse(out.str());
  EXPECT_TRUE(parsed.result.IsComplete());
  EXPECT_FALSE(parsed.time.has_value());
}

namespace {

  struct Delivered {
    std::vector<carla::multigpu::MultiGPUCommand> ids;
    std::vector<std::string> payloads;
  };

  /// Feeds @a message to SecondaryCommands::process_command and records what
  /// reaches the engine's callback.
  Delivered Process(const std::string &message) {
    Delivered delivered;
    carla::multigpu::SecondaryCommands commands;
    commands.set_callback([&](carla::multigpu::MultiGPUCommand id, carla::Buffer data) {
      delivered.ids.push_back(id);
      delivered.payloads.emplace_back(reinterpret_cast<const char *>(data.data()), data.size());
    });
    commands.process_command(carla::Buffer(
        reinterpret_cast<const unsigned char *>(message.data()), message.size()));
    return delivered;
  }

  std::string Command(carla::multigpu::MultiGPUCommand id, uint32_t declared, const std::string &payload) {
    carla::multigpu::CommandHeader header{id, declared};
    std::string message(reinterpret_cast<const char *>(&header), sizeof(header));
    return message + payload;
  }

} // namespace

TEST(MultiGpuCommands, process_command_delivers_exactly_the_declared_payload) {
  const Delivered delivered = Process(Command(carla::multigpu::SEND_FRAME, 3u, "abc"));
  ASSERT_EQ(delivered.ids.size(), 1u);
  EXPECT_EQ(delivered.ids[0], carla::multigpu::SEND_FRAME);
  EXPECT_EQ(delivered.payloads[0], "abc");
}

TEST(MultiGpuCommands, process_command_drops_a_message_shorter_than_its_header) {
  EXPECT_TRUE(Process(std::string("\x01\x00\x00", 3)).ids.empty());
}

TEST(MultiGpuCommands, process_command_drops_a_payload_larger_than_the_message) {
  EXPECT_TRUE(Process(Command(carla::multigpu::LOAD_MAP, 4096u, "Town10HD")).ids.empty());
}

TEST(MultiGpuCommands, process_command_ignores_trailing_bytes) {
  const Delivered delivered = Process(Command(carla::multigpu::GET_TOKEN, 2u, "xyz"));
  ASSERT_EQ(delivered.ids.size(), 1u);
  EXPECT_EQ(delivered.payloads[0], "xy");
}
