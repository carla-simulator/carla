// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/ROS2.h"


namespace carla {
namespace ros2 {

void ROS2::SetTimestamp(double timestamp) {
    if (IsEnabled()){
        double integral;
        const double fractional = modf(timestamp, &integral);
        const double multiplier = 1000000000.0;
        _seconds = static_cast<int32_t>(integral);
        _nanoseconds = static_cast<uint32_t>(fractional * multiplier);
    }
}

// static fields
std::shared_ptr<ROS2> ROS2::_instance;

} // namespace ros2
} // namespace carla
