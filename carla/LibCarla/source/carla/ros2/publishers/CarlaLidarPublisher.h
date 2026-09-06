// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaPointCloudPublisher.h"

namespace carla {
namespace ros2 {

class CarlaLidarPublisher : public CarlaPointCloudPublisher {
public:
  CarlaLidarPublisher(std::string base_topic_name, std::string frame_id)
    : CarlaPointCloudPublisher(std::move(base_topic_name), std::move(frame_id)) {}

private:
  [[nodiscard]] std::size_t GetPointSize() const override;
  [[nodiscard]] const PointFieldDescriptor *GetFieldDescriptors() const override;
  [[nodiscard]] std::size_t GetFieldDescriptorCount() const override;
  [[nodiscard]] std::vector<std::uint8_t> ComputePointCloud(
      std::uint32_t height, std::uint32_t width, const std::uint8_t *data) const override;
};

}  // namespace ros2
}  // namespace carla
