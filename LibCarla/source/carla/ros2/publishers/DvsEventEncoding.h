// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace carla {
namespace ros2 {

// DVS event tuple consumed by the encoder. Mirrors sensor::data::DVSEvent's
// public fields exactly, including its `#pragma pack(push, 1)` so the
// in-memory layout matches the wire bytes published to ROS 2 (13 B per
// event). The seam header stays FastDDS-free so a GTest can build the
// payload directly.
#pragma pack(push, 1)
struct DvsEvent {
  std::uint16_t x;
  std::uint16_t y;
  std::int64_t t;
  bool pol;
};
#pragma pack(pop)
static_assert(sizeof(DvsEvent) == 13u, "DvsEvent must be packed to 13 bytes");

// Encodes a stream of DVS events into a height*width*3 BGR uint8 buffer:
// positive-polarity events light the Red channel at (x, y), negative-polarity
// events light the Blue channel. The encoding deliberately matches the wire
// format ROS 2 subscribers already consume on ue5-dev today:
//   index = (y * width + x) * 3 + (pol * 2)
//   buffer[index] = 255
// The 3-channel BGR layout (sensor_msgs/Image encoding "bgr8") is what the
// CarlaDVSCameraPublisher exposes to subscribers; restructuring the publisher
// in PR-4 must not change that wire format.
[[nodiscard]] inline std::vector<std::uint8_t> EncodeDvsEventsToBgr(
    std::uint32_t height,
    std::uint32_t width,
    const DvsEvent *events,
    std::size_t event_count) {
  std::vector<std::uint8_t> buffer(
      static_cast<std::size_t>(height) * static_cast<std::size_t>(width) * 3u, 0u);
  for (std::size_t i = 0; i < event_count; ++i) {
    const auto &event = events[i];
    if (event.x >= width || event.y >= height) {
      continue;
    }
    const std::size_t pixel_index =
        (static_cast<std::size_t>(event.y) * width + event.x) * 3u
        + (event.pol ? 2u : 0u);
    buffer[pixel_index] = 255u;
  }
  return buffer;
}

// Same encoding but starting from a raw byte buffer of packed
// sensor::data::DVSEvent records. Used by CarlaDVSCameraPublisher to bridge
// the void* boundary into the seam without pulling DVSEvent into the
// FastDDS-free header.
[[nodiscard]] inline std::vector<std::uint8_t> EncodeDvsEventsToBgr(
    std::uint32_t height,
    std::uint32_t width,
    const std::uint8_t *raw_events,
    std::size_t event_count,
    std::size_t event_stride) {
  std::vector<std::uint8_t> buffer(
      static_cast<std::size_t>(height) * static_cast<std::size_t>(width) * 3u, 0u);
  for (std::size_t i = 0; i < event_count; ++i) {
    DvsEvent event{};
    std::memcpy(&event, raw_events + i * event_stride, sizeof(DvsEvent));
    if (event.x >= width || event.y >= height) {
      continue;
    }
    const std::size_t pixel_index =
        (static_cast<std::size_t>(event.y) * width + event.x) * 3u
        + (event.pol ? 2u : 0u);
    buffer[pixel_index] = 255u;
  }
  return buffer;
}

// Packs a stream of DVS events into the sensor_msgs/PointCloud2 wire layout
// declared by kDvsFields: x (UINT16 @0), y (UINT16 @2), t (FLOAT64 @4),
// pol (INT8 @12), 13 B stride. The int64 event timestamp is converted to a
// real double in the t slot because sensor_msgs PointField has no 8-byte
// integer datatype, so a raw int64 byte copy would be misdecoded as a double.
[[nodiscard]] inline std::vector<std::uint8_t> EncodeDvsEventsToPointCloud(
    const DvsEvent *events,
    std::size_t event_count) {
  std::vector<std::uint8_t> bytes(event_count * sizeof(DvsEvent));
  for (std::size_t i = 0; i < event_count; ++i) {
    const DvsEvent &event = events[i];
    std::uint8_t *out = bytes.data() + i * sizeof(DvsEvent);
    const double t = static_cast<double>(event.t);
    const std::int8_t pol = event.pol ? 1 : 0;
    std::memcpy(out + 0u, &event.x, sizeof(event.x));
    std::memcpy(out + 2u, &event.y, sizeof(event.y));
    std::memcpy(out + 4u, &t, sizeof(t));
    std::memcpy(out + 12u, &pol, sizeof(pol));
  }
  return bytes;
}

// Same packing but starting from a raw byte buffer of packed
// sensor::data::DVSEvent records. Used by CarlaDVSPointCloudPublisher to bridge
// the void* boundary into the seam without pulling DVSEvent into the
// FastDDS-free header.
[[nodiscard]] inline std::vector<std::uint8_t> EncodeDvsEventsToPointCloud(
    const std::uint8_t *raw_events,
    std::size_t event_count,
    std::size_t event_stride) {
  std::vector<std::uint8_t> bytes(event_count * sizeof(DvsEvent));
  for (std::size_t i = 0; i < event_count; ++i) {
    DvsEvent event{};
    std::memcpy(&event, raw_events + i * event_stride, sizeof(DvsEvent));
    std::uint8_t *out = bytes.data() + i * sizeof(DvsEvent);
    const double t = static_cast<double>(event.t);
    const std::int8_t pol = event.pol ? 1 : 0;
    std::memcpy(out + 0u, &event.x, sizeof(event.x));
    std::memcpy(out + 2u, &event.y, sizeof(event.y));
    std::memcpy(out + 4u, &t, sizeof(t));
    std::memcpy(out + 12u, &pol, sizeof(pol));
  }
  return bytes;
}

}  // namespace ros2
}  // namespace carla
