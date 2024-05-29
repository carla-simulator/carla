// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/ROS2Session.h"
#include "carla/ros2/ROS2.h"

namespace carla {
namespace ros2 {

/// Writes a message to the ROS2 publisher.
void ROS2Session::WriteMessage(std::shared_ptr<const carla::streaming::detail::Message> message) {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  ROS2->ProcessDataFromUeSensor(_stream_id, message);
}

void ROS2Session::EnableForROS() {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  ROS2->EnableForROS(_stream_id);
}

void ROS2Session::DisableForROS() {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  ROS2->DisableForROS(_stream_id);
}

bool ROS2Session::IsEnabledForROS() {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  return ROS2->IsEnabledForROS(_stream_id);
}
}  // namespace ros2
}  // namespace carla
