// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaTrafficLightInfoPublisher.h"

#include "carla/ros2/publishers/UeToRosConversions.h"

namespace carla {
namespace ros2 {

bool CarlaTrafficLightInfoPublisher::Write(
    const std::vector<TrafficLightInfo> &info) {
  auto message = _impl->GetMessage();

  message->traffic_lights.clear();
  message->traffic_lights.reserve(info.size());
  for (const auto &light : info) {
    msg::CarlaTrafficLightInfo light_info;
    light_info.id = light.id;

    light_info.transform.position.x = light.transform.location.x;
    light_info.transform.position.y = -light.transform.location.y;
    light_info.transform.position.z = light.transform.location.z;
    light_info.transform.orientation =
        ue_rotation_to_ros_quaternion(light.transform.rotation);

    // Center stays relative to the light transform, flipped to right-handed;
    // the size is the full box extent and carries no axis sign.
    light_info.trigger_volume.center = ue_vector_to_ros_vector(light.trigger_center);
    light_info.trigger_volume.size.x = light.trigger_extent.x * 2.0;
    light_info.trigger_volume.size.y = light.trigger_extent.y * 2.0;
    light_info.trigger_volume.size.z = light.trigger_extent.z * 2.0;

    message->traffic_lights.push_back(light_info);
  }

  return true;
}

}  // namespace ros2
}  // namespace carla
