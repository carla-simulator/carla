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
  FastDDS,
  CycloneDDS
};

/// Convert a DDSMiddleware enum value to a readable string.
inline const char* DDSMiddlewareToString(DDSMiddleware middleware) {
  switch (middleware) {
    case DDSMiddleware::FastDDS:
      return "FastDDS";
    case DDSMiddleware::CycloneDDS:
      return "CycloneDDS";
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
  if (name == "cyclonedds") {
    return {true, DDSMiddleware::CycloneDDS};
  }
  return {false, DDSMiddleware::FastDDS};
}

/// Return a readable list of middleware implementations compiled into this binary.
inline std::string GetAvailableMiddlewareString() {
  std::string result;
#if defined(CARLA_ROS2_DDS_FASTDDS)
  result += "FastDDS";
#endif
#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
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

/// Mangle a DDS type name into the ROS2-compatible format.
/// "sensor_msgs::msg::Image" becomes "sensor_msgs::msg::dds_::Image_".
/// A bare name like "Image" becomes "dds_::Image_".
inline std::string ToROS2DDSTypeName(const std::string& dds_type_name) {
  auto pos = dds_type_name.rfind("::");
  if (pos == std::string::npos) {
    return "dds_::" + dds_type_name + "_";
  }
  return dds_type_name.substr(0, pos) +
      "::dds_::" + dds_type_name.substr(pos + 2) + "_";
}

} // namespace ros2
} // namespace carla
