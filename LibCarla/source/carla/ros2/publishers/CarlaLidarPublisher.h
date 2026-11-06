// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaPointCloudPublisher.h"
#include "carla/ros2/publishers/LidarScanAssembler.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace carla {
namespace ros2 {

// Wire POD for the extended Autoware PointXYZIRCAEDT layout (tier4
// SetDataEx). Natural alignment yields no padding: the uint16 channel lands
// on offset 14 and the trailing uint32 closes the struct at 32 B, matching
// kLidarFieldsEx in PointCloudFieldsLayout.h byte for byte.
struct LidarPointEx {
  float x{0.0f};
  float y{0.0f};
  float z{0.0f};
  std::uint8_t intensity{0u};
  std::uint8_t return_type{0u};
  std::uint16_t channel{0u};
  float azimuth{0.0f};
  float elevation{0.0f};
  float distance{0.0f};
  std::uint32_t time_stamp{0u};
};

static_assert(
    sizeof(LidarPointEx) == 32u,
    "LidarPointEx must match the kLidarFieldsEx wire layout (32 B stride)");

class CarlaLidarPublisher : public CarlaPointCloudPublisher {
public:
  CarlaLidarPublisher(std::string base_topic_name, std::string frame_id, bool exact_topic = false)
    : CarlaPointCloudPublisher(std::move(base_topic_name), std::move(frame_id), exact_topic) {}

  // Plain XYZI path (4 float32 per point). Shadows the base method so a call
  // through a CarlaLidarPublisher pointer always resets the extended mode
  // before the base implementation consults the layout virtuals.
  bool WritePointCloud(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      const std::uint8_t *data) {
    _extended = false;
    return CarlaPointCloudPublisher::WritePointCloud(
        seconds, nanoseconds, height, width, data);
  }

  // Extended XYZIRCAEDT path (tier4 SetDataEx port): expands each 16 B XYZI
  // detection into the 32 B Autoware PointXYZIRCAEDT layout declared by
  // kLidarFieldsEx. `data` is the raw XYZI buffer (`width` points of 4
  // float32s), `channel_point_counts` holds `channel_count` per-channel point
  // counts (the LidarData header tail), and `vertical_angles` provides one
  // elevation (radians) per channel.
  bool WriteExtendedPointCloud(
      std::int32_t seconds,
      std::uint32_t nanoseconds,
      std::uint32_t height,
      std::uint32_t width,
      const std::uint8_t *data,
      const std::uint32_t *channel_point_counts,
      std::size_t channel_count,
      const std::vector<float> &vertical_angles);

  // Driver mode (ros_driver_mode): adds the step to the rotation being
  // assembled, in the extended layout with the azimuth convention of a lidar
  // driver (nebula: clockwise from +Y, [0, 2pi)). Returns true when a rotation
  // was completed and written (stamped at its first point): only then is there
  // a message to publish.
  bool WriteDriverScan(
      const LidarDriverOptions &options,
      std::uint32_t width,
      const std::uint8_t *data,
      const std::uint32_t *channel_point_counts,
      std::size_t channel_count,
      const std::vector<float> &vertical_angles);

private:
  // Expands the XYZI detections into the extended layout (time_stamp 0).
  [[nodiscard]] std::vector<LidarPointEx> ExpandPoints(
      std::size_t total_points, const std::uint8_t *data) const;

  [[nodiscard]] std::size_t GetPointSize() const override;
  [[nodiscard]] const PointFieldDescriptor *GetFieldDescriptors() const override;
  [[nodiscard]] std::size_t GetFieldDescriptorCount() const override;
  [[nodiscard]] std::vector<std::uint8_t> ComputePointCloud(
      std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const override;

  // Extended-mode state, set by WriteExtendedPointCloud before it delegates
  // to the base WritePointCloud (which consults the virtuals above).
  bool _extended = false;
  std::vector<std::uint32_t> _channel_point_counts;
  std::vector<float> _vertical_angles;

  // Driver-mode rotation assembly, created on the first driver-mode step.
  std::unique_ptr<LidarScanAssembler<LidarPointEx>> _assembler;
  float _cut_angle = 0.0f;
};

}  // namespace ros2
}  // namespace carla
