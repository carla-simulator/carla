// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaMapPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaMapPublisher::Write(const std::string& open_drive) {
  _impl->GetMessage()->data = open_drive;

  return true;
}

}  // namespace ros2
}  // namespace carla
