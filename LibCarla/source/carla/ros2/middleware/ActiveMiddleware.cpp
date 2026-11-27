// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/middleware/ActiveMiddleware.h"
#include "carla/ros2/middleware/MiddlewareFactory.h"

#if !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
#  if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS)
#    include "carla/ros2/middleware/fastdds/FastDDSSharedParticipant.h"
#  endif
#  if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS)
#    include "carla/ros2/middleware/cyclonedds/CycloneDDSSertype.h"
#  endif
#  if defined(CARLA_ROS2_MIDDLEWARE_ZENOH)
#    include "carla/ros2/middleware/zenoh/ZenohSharedSession.h"
#  endif
#endif

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

bool AcquireMiddlewareContext() {
  switch (MiddlewareFactory::GetMiddleware()) {
    case Middleware::FastDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
      return FastDDSSharedParticipant::acquire() != nullptr;
#else
      return false;
#endif
    case Middleware::CycloneDDS:
#if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
      // Process-lifetime participant: nothing to release.
      return carla_cdr_get_participant() >= 0;
#else
      return false;
#endif
    case Middleware::Zenoh:
#if defined(CARLA_ROS2_MIDDLEWARE_ZENOH) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
      // Process-lifetime session: nothing to release.
      return zenoh_shared_session_is_open();
#else
      return false;
#endif
  }
  return false;
}

void ReleaseMiddlewareContext() {
#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS) && !defined(CARLA_ROS2_MIDDLEWARE_TESTING)
  if (MiddlewareFactory::GetMiddleware() == Middleware::FastDDS) {
    FastDDSSharedParticipant::release();
  }
#endif
}

} // namespace ros2
} // namespace carla
