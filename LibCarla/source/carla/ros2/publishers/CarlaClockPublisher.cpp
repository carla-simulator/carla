// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaClockPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaClockPublisher::Write(int32_t seconds, uint32_t nanoseconds) {
  _impl->GetMessage()->clock().sec(seconds);
  _impl->GetMessage()->clock().nanosec(nanoseconds);

  return true;
}

}  // namespace ros2
}  // namespace carla
