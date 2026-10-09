// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaStaticTransformPublisher.h"

#include "carla/ros2/publishers/UeToRosConversions.h"

namespace carla {
namespace ros2 {

bool CarlaStaticTransformPublisher::Write(
    int32_t seconds,
    uint32_t nanoseconds,
    std::string frame_id,
    std::string child_frame_id,
    const geom::Transform &transform) {
  msg::TransformStamped ts;

  ts.header.stamp.sec = seconds;
  ts.header.stamp.nanosec = nanoseconds;
  ts.header.frame_id = frame_id;
  ts.child_frame_id = child_frame_id;

  ts.transform.translation = ue_vector_to_ros_vector(transform.location);
  ts.transform.rotation = ue_rotation_to_ros_quaternion(transform.rotation);

  _impl->GetMessage()->transforms = {ts};

  return true;
}

}  // namespace ros2
}  // namespace carla
