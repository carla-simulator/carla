// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaCameraPublisher.h"

namespace carla {
namespace ros2 {

class CarlaRGBCameraPublisher : public CarlaCameraPublisher {
  public:
    CarlaRGBCameraPublisher(std::string base_topic_name, std::string frame_id):
      CarlaCameraPublisher(base_topic_name, frame_id) {}

    uint8_t GetChannels() override { return 4; }
  
  private:
    std::string GetEncoding() override { return "bgra8"; }
};

}  // namespace ros2
}  // namespace carla
