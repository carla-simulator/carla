// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#include "carla/ros2/middleware/Middleware.h"
#include "carla/ros2/middleware/MiddlewareConfig.h"

namespace carla {
namespace ros2 {

/// DDS-free bridge for selecting the active middleware across the shared-library
/// boundary.
///
/// ROS2.cpp is the only ROS 2 translation unit compiled into carla-server; every
/// other ROS 2 source (including MiddlewareFactory.h and the vendor middleware
/// headers) is compiled exclusively into libcarla-ros2-native.so. These
/// declarations include only Middleware.h, so carla-server can select the
/// middleware without any DDS header crossing the boundary. The definitions are
/// compiled into the shared library.

/// Resolve and select the requested middleware.
/// @return true if the middleware was compiled into the shared library and is now
/// active, false if it is unavailable (in which case the active middleware is
/// left unchanged).
bool SetActiveMiddleware(Middleware requested);

/// @return A readable list of the middleware implementations compiled into the
/// shared library. Must be resolved inside the shared library because the
/// CARLA_ROS2_MIDDLEWARE_* macros are defined only on the carla-ros2-native
/// target, not on carla-server.
std::string GetAvailableMiddleware();

/// Store the domain id in the shared library's MiddlewareConfig and return the
/// effective domain id resolved there, so carla-server can log the value the
/// middlewares will actually use. Pass kUnsetDomainId to keep each middleware's
/// native default. Like the other exports here, this is DDS-free and its
/// definition is compiled into the shared library, so MiddlewareConfig's
/// process-wide storage lives on the shared-library side that the middlewares
/// read.
ResolvedDomainId SetActiveDomainId(int domain_id);

} // namespace ros2
} // namespace carla
