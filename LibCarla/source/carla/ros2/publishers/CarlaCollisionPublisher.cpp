// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaCollisionPublisher.h"

namespace carla {
namespace ros2 {

bool CarlaCollisionPublisher::Write(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, geom::Vector3D impulse) {

  _impl->GetMessage()->header().stamp().sec(seconds);
  _impl->GetMessage()->header().stamp().nanosec(nanoseconds);
  _impl->GetMessage()->header().frame_id(GetFrameId());

  _impl->GetMessage()->other_actor_id(actor_id);

  _impl->GetMessage()->normal_impulse().x(impulse.x);
  _impl->GetMessage()->normal_impulse().y(impulse.y);
  _impl->GetMessage()->normal_impulse().z(impulse.z);

  return true;
}

}  // namespace ros2
}  // namespace carla
