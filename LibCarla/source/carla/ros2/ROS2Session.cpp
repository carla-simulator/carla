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

void ROS2Session::EnableForROS(carla::streaming::detail::actor_id_type actor_id) {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  ROS2->EnableForROS({.stream_id=_stream_id, .actor_id=actor_id});
}

void ROS2Session::DisableForROS(carla::streaming::detail::actor_id_type actor_id) {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  ROS2->DisableForROS({.stream_id=_stream_id, .actor_id=actor_id});
}

bool ROS2Session::IsEnabledForROS(carla::streaming::detail::actor_id_type actor_id) {
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  return ROS2->IsEnabledForROS({.stream_id=_stream_id, .actor_id=actor_id});
}
}  // namespace ros2
}  // namespace carla
