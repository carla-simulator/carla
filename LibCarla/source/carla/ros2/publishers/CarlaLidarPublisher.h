// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "CarlaPointCloudPublisher.h"

namespace carla {
namespace ros2 {

  class CarlaLidarPublisher : public CarlaPointCloudPublisher {
    public:
      CarlaLidarPublisher(std::string base_topic_name, std::string frame_id) :
        CarlaPointCloudPublisher(base_topic_name, frame_id) {}

    private:
      const size_t GetPointSize() override;
      std::vector<sensor_msgs::msg::PointField> GetFields() override;

      std::vector<uint8_t> ComputePointCloud(uint32_t height, uint32_t width, uint8_t *data) override;
  };

}  // namespace ros2
}  // namespace carla
