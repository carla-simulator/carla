// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaGNSSPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaGNSSPublisher::Write(int32_t seconds, uint32_t nanoseconds, const double* data) {

  _impl->GetMessage()->header().stamp().sec(seconds);
  _impl->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl->GetMessage()->header().frame_id(GetFrameId());

  _impl->GetMessage()->latitude(*data++);
  _impl->GetMessage()->longitude(*data++);
  _impl->GetMessage()->altitude(*data++);

  return true;
}

}  // namespace ros2
}  // namespace carla
