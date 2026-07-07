// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/middleware/Middleware.h"

namespace carla {
namespace ros2 {

/// DDS-free bridge for selecting the active middleware across the shared-library
/// boundary.
///
/// ROS2.cpp is the only ROS 2 translation unit compiled into carla-server; every
/// other ROS 2 source (including MiddlewareFactory.h and the vendor middleware
/// headers) is compiled exclusively into libcarla-ros2-native.so. This declaration
/// includes only Middleware.h, so carla-server can select the middleware without
/// any DDS header crossing the boundary. The definition (which forwards to
/// MiddlewareFactory::SetMiddleware) is compiled into the shared library.
void SetActiveMiddleware(Middleware middleware);

} // namespace ros2
} // namespace carla
