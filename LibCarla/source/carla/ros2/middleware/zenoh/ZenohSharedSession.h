// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifndef CARLA_ROS2_MIDDLEWARE_TESTING
#ifdef CARLA_ROS2_MIDDLEWARE_ZENOH

#include <zenoh.h>

#include <string>

namespace carla {
namespace ros2 {

/// Return a loaned pointer to the process-wide shared Zenoh Session, opened
/// lazily on the first call and kept alive for the process lifetime. All
/// Zenoh publishers and subscribers attach to this Session; the carla_bridge
/// node liveliness token is declared as a side effect so rmw_zenoh peers
/// list the node in their graph cache.
const z_loaned_session_t* zenoh_get_shared_session();

/// Hex string form of the shared Session's ZID, suitable for rmw_zenoh
/// liveliness keyexprs. Cached after first call.
const std::string& zenoh_session_zid();

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_ZENOH
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
