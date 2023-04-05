// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Logging.h"
#include "carla/ros2/ROS2.h"

namespace carla {
namespace ros2 {

// static fields
std::shared_ptr<ROS2> ROS2::_instance;

void ROS2::Enable(bool enable) { 
  _enabled = enable; 
  log_info("ROS2 enabled: ", _enabled);
}

void ROS2::SetFrame(uint64_t frame) {
  _frame = frame;
  // log_info("ROS2 new frame: ", _frame);
}

void ROS2::ProcessDataFromSensor(uint64_t sensor_type, carla::streaming::detail::stream_id_type stream_id, carla::Buffer &buffer) {
  log_info("Sensor to ROS data: frame.", _frame, "sensor.", sensor_type, "stream.", stream_id);
}

} // namespace ros2
} // namespace carla
