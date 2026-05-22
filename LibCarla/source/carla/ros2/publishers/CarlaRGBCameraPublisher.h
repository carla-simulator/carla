// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaCameraPublisher.h"

#include <string>
#include <utility>

namespace carla {
namespace ros2 {

class CarlaRGBCameraPublisher : public CarlaCameraPublisher {
public:
  CarlaRGBCameraPublisher(std::string base_topic_name, std::string frame_id)
    : CarlaCameraPublisher(std::move(base_topic_name), std::move(frame_id)) {}

protected:
  [[nodiscard]] uint8_t GetChannels() const override { return 4; }
  [[nodiscard]] std::string GetEncoding() const override { return "bgra8"; }
};

}  // namespace ros2
}  // namespace carla
