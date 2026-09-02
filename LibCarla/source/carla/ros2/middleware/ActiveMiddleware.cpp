// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/middleware/ActiveMiddleware.h"
#include "carla/ros2/middleware/MiddlewareFactory.h"

namespace carla {
namespace ros2 {

void SetActiveMiddleware(Middleware middleware) {
  MiddlewareFactory::SetMiddleware(middleware);
}

} // namespace ros2
} // namespace carla
