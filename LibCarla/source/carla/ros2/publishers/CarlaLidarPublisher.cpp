// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaLidarPublisher.h"

#include <cmath>
#include <cstring>

namespace carla {
namespace ros2 {

namespace {

// Wire-compatible POD mirror of sensor::data::LidarDetection: {x, y, z,
// intensity} as four contiguous float32s. The real struct lives behind
// carla/sensor/data/LidarData.h, which transitively includes
// carla/MsgPack.h + carla/Buffer.h and therefore pulls Boost — not
// available on the carla-ros2-native ExternalProject include path. The
// POD shape and stride (16 B per point) are pinned by kLidarFields in
// PointCloudFieldsLayout.h and by test_point_cloud_fields.cpp.
struct LidarPoint {
  float x;
  float y;
  float z;
  float intensity;
};

static_assert(
    sizeof(LidarPoint) == 16u,
    "LidarPoint must be tightly packed (4 float32s) for the wire layout");

}  // namespace

std::size_t CarlaLidarPublisher::GetPointSize() const {
  return _extended ? sizeof(LidarPointEx) : sizeof(LidarPoint);
}

const PointFieldDescriptor *CarlaLidarPublisher::GetFieldDescriptors() const {
  return _extended ? kLidarFieldsEx.data() : kLidarFields.data();
}

std::size_t CarlaLidarPublisher::GetFieldDescriptorCount() const {
  return _extended ? kLidarFieldsEx.size() : kLidarFields.size();
}

bool CarlaLidarPublisher::WriteExtendedPointCloud(
    std::int32_t seconds,
    std::uint32_t nanoseconds,
    std::uint32_t height,
    std::uint32_t width,
    const std::uint8_t *data,
    const std::uint32_t *channel_point_counts,
    std::size_t channel_count,
    const std::vector<float> &vertical_angles) {
  _extended = true;
  _channel_point_counts.assign(
      channel_point_counts, channel_point_counts + channel_count);
  _vertical_angles = vertical_angles;
  return CarlaPointCloudPublisher::WritePointCloud(
      seconds, nanoseconds, height, width, data);
}

std::vector<std::uint8_t> CarlaLidarPublisher::ComputePointCloud(
    std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const {
  const std::size_t total_points =
      static_cast<std::size_t>(height) * static_cast<std::size_t>(width);

  if (!_extended) {
    const std::size_t total_bytes = total_points * sizeof(LidarPoint);

    std::vector<std::uint8_t> bytes(total_bytes);
    std::memcpy(bytes.data(), data, total_bytes);

    // Mirror the Y axis to land in the ROS right-handed frame. The buffer is
    // a contiguous array of LidarPoint PODs by contract; aliasing into the
    // copy is safe.
    auto *points = reinterpret_cast<LidarPoint *>(bytes.data());
    for (std::size_t i = 0; i < total_points; ++i) {
      points[i].y *= -1.0f;
    }
    return bytes;
  }

  // Extended path (tier4 SetDataEx semantics).
  const auto points = ExpandPoints(total_points, data);
  std::vector<std::uint8_t> bytes(points.size() * sizeof(LidarPointEx));
  std::memcpy(bytes.data(), points.data(), bytes.size());
  return bytes;
}

std::vector<LidarPointEx> CarlaLidarPublisher::ExpandPoints(
    std::size_t total_points, const std::uint8_t *data) const {
  // Expand each XYZI detection into a LidarPointEx.
  const auto *input = reinterpret_cast<const LidarPoint *>(data);

  std::vector<LidarPointEx> points(total_points);

  for (std::size_t i = 0; i < total_points; ++i) {
    auto &point = points[i];
    point.x = input[i].x;
    point.y = -input[i].y;  // ROS right-handed frame, same flip as XYZI path.
    // tier4 truncates CARLA's [0, 1] float intensity straight into the uint8
    // slot (so it is effectively 0/1); replicated verbatim for wire parity.
    point.intensity = static_cast<std::uint8_t>(input[i].intensity);
    point.z = input[i].z;
    // return_type stays 0 (tier4 leaves it as dummy data).
    // Azimuth/distance are derived from the (already mirrored) Cartesian
    // coordinates, matching tier4's post-conversion computation.
    point.distance = std::hypot(point.x, point.y, point.z);
    point.azimuth = std::atan2(point.y, point.x);
    // Autoware's distortion corrector reads time_stamp as a nanosecond offset
    // from header.stamp. CARLA casts every ray of a scan at the same sim
    // instant, so the offset is zero for all points (driver mode sets it per
    // point when it assembles the rotation).
    point.time_stamp = 0u;
  }

  // Channel index and per-channel elevation from the LidarData header's
  // per-channel point counts. Points are serialized channel-major, so the
  // running offset recovers each point's channel.
  std::size_t accumulated = 0u;
  for (std::size_t channel = 0; channel < _channel_point_counts.size(); ++channel) {
    const std::size_t count = _channel_point_counts[channel];
    const float elevation =
        channel < _vertical_angles.size() ? _vertical_angles[channel] : 0.0f;
    for (std::size_t i = 0; i < count && accumulated + i < total_points; ++i) {
      points[accumulated + i].channel = static_cast<std::uint16_t>(channel);
      points[accumulated + i].elevation = elevation;
    }
    accumulated += count;
  }

  return points;
}

bool CarlaLidarPublisher::WriteDriverScan(
    const LidarDriverOptions &options,
    std::uint32_t width,
    const std::uint8_t *data,
    const std::uint32_t *channel_point_counts,
    std::size_t channel_count,
    const std::vector<float> &vertical_angles) {
  _extended = true;
  _channel_point_counts.assign(
      channel_point_counts, channel_point_counts + channel_count);
  _vertical_angles = vertical_angles;

  auto points = ExpandPoints(width, data);
  for (auto &point : points) {
    // Lidar driver azimuth (nebula): clockwise from +Y, growing with the
    // firing order of this clockwise sensor.
    constexpr float kTwoPi = 6.28318530717958647692f;
    const float azimuth = std::atan2(point.x, point.y);
    point.azimuth = azimuth < 0.0f ? azimuth + kTwoPi : azimuth;
  }

  if (_assembler == nullptr || _cut_angle != options.cut_angle) {
    _assembler = std::make_unique<LidarScanAssembler<LidarPointEx>>(options.cut_angle);
    _cut_angle = options.cut_angle;
  }
  auto scan = _assembler->Add(options.slice, std::move(points));
  if (!scan.has_value()) {
    return false;
  }

  std::vector<std::uint8_t> bytes(scan->points.size() * sizeof(LidarPointEx));
  std::memcpy(bytes.data(), scan->points.data(), bytes.size());
  const double seconds = std::floor(scan->stamp);
  auto nanoseconds = static_cast<std::uint32_t>(std::llround((scan->stamp - seconds) * 1e9));
  auto whole_seconds = static_cast<std::int32_t>(seconds);
  if (nanoseconds >= 1000000000u) {
    nanoseconds -= 1000000000u;
    ++whole_seconds;
  }
  return CarlaPointCloudPublisher::WritePointCloud(
      whole_seconds, nanoseconds, 1u,
      static_cast<std::uint32_t>(scan->points.size()), std::move(bytes));
}

}  // namespace ros2
}  // namespace carla
