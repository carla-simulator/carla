// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace ros2 {

/// Enumeration of available DDS backends.
/// Passed to ROS2::Enable() to select the backend at startup.
/// Once set, the backend cannot be changed without restarting.
enum class DDSBackend {
  FastDDS,
  CycloneDDS
};

/// Convert a DDSBackend enum value to a readable string.
inline const char* DDSBackendToString(DDSBackend backend) {
  switch (backend) {
    case DDSBackend::FastDDS:    return "FastDDS";
    case DDSBackend::CycloneDDS: return "CycloneDDS";
  }
  return "Unknown";
}

/// Result of parsing a backend name string.
struct DDSBackendParseResult {
  bool valid;
  DDSBackend backend;
};

/// Parse a backend name string (lowercase). Returns {true, backend} on match,
/// {false, FastDDS} for unrecognized values.
inline DDSBackendParseResult DDSBackendFromString(const std::string& name) {
  if (name == "fastdds")    return {true, DDSBackend::FastDDS};
  if (name == "cyclonedds") return {true, DDSBackend::CycloneDDS};
  return {false, DDSBackend::FastDDS};
}

/// Convert a DDS type name from fastddsgen format ("sensor_msgs::msg::Image")
/// to the ROS2 DDS mangled format ("sensor_msgs::msg::dds_::Image_") required
/// for endpoint matching with native ROS2 subscribers (rviz, ros2 topic, etc.).
/// The ROS2 rosidl pipeline uses this convention in rosidl_typesupport_fastrtps.
inline std::string ToROS2DDSTypeName(const std::string& dds_type_name) {
  // Find the last "::" separator to split "pkg::msg::TypeName" into
  // "pkg::msg" + "TypeName", then insert "dds_::" and append "_".
  auto pos = dds_type_name.rfind("::");
  if (pos == std::string::npos) {
    // No namespace — just add the mangling
    return "dds_::" + dds_type_name + "_";
  }
  // "sensor_msgs::msg" + "::dds_::" + "Image" + "_"
  return dds_type_name.substr(0, pos) + "::dds_::" + dds_type_name.substr(pos + 2) + "_";
}

/// Return a readable list of backends compiled into this binary.
inline std::string GetAvailableBackendsString() {
  std::string result;
#if defined(CARLA_ROS2_DDS_FASTDDS)
  result += "FastDDS";
#endif
#if defined(CARLA_ROS2_DDS_CYCLONEDDS)
  if (!result.empty()) result += ", ";
  result += "CycloneDDS";
#endif
  if (result.empty()) result = "none";
  return result;
}

} // namespace ros2
} // namespace carla
