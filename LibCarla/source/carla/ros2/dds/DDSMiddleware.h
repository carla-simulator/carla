// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

/// Enumeration of available DDS middleware implementations.
/// Passed to ROS2::Enable() to select the middleware at startup.
/// Once set, the middleware cannot be changed without restarting.
enum class DDSMiddleware {
  FastDDS
};

/// Convert a DDSMiddleware enum value to a readable string.
inline const char* DDSMiddlewareToString(DDSMiddleware middleware) {
  switch (middleware) {
    case DDSMiddleware::FastDDS:
      return "FastDDS";
  }
  return "Unknown";
}

/// Result of parsing a middleware name string.
struct DDSMiddlewareParseResult {
  bool valid;
  DDSMiddleware middleware;
};

/// Parse a middleware name string (lowercase). Returns {true, middleware} on match,
/// {false, FastDDS} for unrecognized values.
inline DDSMiddlewareParseResult DDSMiddlewareFromString(const std::string& name) {
  if (name == "fastdds") {
    return {true, DDSMiddleware::FastDDS};
  }
  return {false, DDSMiddleware::FastDDS};
}

/// Return a readable list of middleware implementations compiled into this binary.
inline std::string GetAvailableMiddlewareString() {
  std::string result;
#if defined(CARLA_ROS2_DDS_FASTDDS)
  result += "FastDDS";
#endif
  if (result.empty()) {
    result = "none";
  }
  return result;
}

} // namespace ros2
} // namespace carla
