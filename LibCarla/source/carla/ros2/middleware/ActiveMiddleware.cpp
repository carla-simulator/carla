// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/middleware/ActiveMiddleware.h"
#include "carla/ros2/middleware/MiddlewareFactory.h"

namespace carla {
namespace ros2 {

bool SetActiveMiddleware(Middleware requested) {
  const auto resolution = MiddlewareFactory::ResolveMiddleware(requested);
  if (resolution.success) {
    MiddlewareFactory::SetMiddleware(resolution.middleware);
  }
  return resolution.success;
}

std::string GetAvailableMiddleware() {
  return MiddlewareFactory::GetAvailableMiddlewareString();
}

ResolvedDomainId SetActiveDomainId(int domain_id) {
  MiddlewareConfig::SetDomainId(domain_id);
  return MiddlewareConfig::ResolveEffective();
}

} // namespace ros2
} // namespace carla
