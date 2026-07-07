// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

/// Enumeration of available middleware implementations.
/// Passed to ROS2::Enable() to select the middleware at startup.
/// Once set, the middleware cannot be changed without restarting.
enum class Middleware {
  FastDDS,
  CycloneDDS
};

/// Convert a Middleware enum value to a readable string.
inline const char* MiddlewareToString(Middleware middleware) {
  switch (middleware) {
    case Middleware::FastDDS:
      return "FastDDS";
    case Middleware::CycloneDDS:
      return "CycloneDDS";
  }
  return "Unknown";
}

/// Result of parsing a middleware name string.
struct MiddlewareParseResult {
  bool valid;
  Middleware middleware;
};

/// Parse a middleware name string (lowercase). Returns {true, middleware} on match,
/// {false, FastDDS} for unrecognized values.
inline MiddlewareParseResult MiddlewareFromString(const std::string& name) {
  if (name == "fastdds") {
    return {true, Middleware::FastDDS};
  }
  if (name == "cyclonedds") {
    return {true, Middleware::CycloneDDS};
  }
  return {false, Middleware::FastDDS};
}

/// Return a readable list of middleware implementations compiled into this binary.
inline std::string GetAvailableMiddlewareString() {
  std::string result;
#if defined(CARLA_ROS2_MIDDLEWARE_FASTDDS)
  result += "FastDDS";
#endif
#if defined(CARLA_ROS2_MIDDLEWARE_CYCLONEDDS)
  if (!result.empty()) {
    result += ", ";
  }
  result += "CycloneDDS";
#endif
  if (result.empty()) {
    result = "none";
  }
  return result;
}

/// Mangle a type name into the ROS2-compatible format.
/// "sensor_msgs::msg::Image" becomes "sensor_msgs::msg::dds_::Image_".
/// A bare name like "Image" becomes "dds_::Image_".
inline std::string ToROS2TypeName(const std::string& type_name) {
  auto pos = type_name.rfind("::");
  if (pos == std::string::npos) {
    return "dds_::" + type_name + "_";
  }
  return type_name.substr(0, pos) +
      "::dds_::" + type_name.substr(pos + 2) + "_";
}

} // namespace ros2
} // namespace carla
