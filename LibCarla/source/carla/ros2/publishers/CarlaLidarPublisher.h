// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaPointCloudPublisher.h"

#include <cstdint>
#include <vector>

namespace carla {
namespace ros2 {

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

private:
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
};

}  // namespace ros2
}  // namespace carla
