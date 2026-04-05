// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstddef>

#include "carla/ros2/types/msg/AckermannDrive.h"
#include "carla/ros2/types/msg/AckermannDriveStamped.h"
#include "carla/ros2/types/msg/CameraInfo.h"
#include "carla/ros2/types/msg/CarlaCollisionEvent.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/msg/CarlaLineInvasion.h"
#include "carla/ros2/types/msg/Clock.h"
#include "carla/ros2/types/msg/Float32.h"
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Image.h"
#include "carla/ros2/types/msg/Imu.h"
#include "carla/ros2/types/msg/NavSatFix.h"
#include "carla/ros2/types/msg/NavSatStatus.h"
#include "carla/ros2/types/msg/Odometry.h"
#include "carla/ros2/types/msg/Point.h"
#include "carla/ros2/types/msg/Point32.h"
#include "carla/ros2/types/msg/PointCloud2.h"
#include "carla/ros2/types/msg/PointField.h"
#include "carla/ros2/types/msg/Pose.h"
#include "carla/ros2/types/msg/PoseWithCovariance.h"
#include "carla/ros2/types/msg/Quaternion.h"
#include "carla/ros2/types/msg/RegionOfInterest.h"
#include "carla/ros2/types/msg/String.h"
#include "carla/ros2/types/msg/TF2Error.h"
#include "carla/ros2/types/msg/TFMessage.h"
#include "carla/ros2/types/msg/Time.h"
#include "carla/ros2/types/msg/Transform.h"
#include "carla/ros2/types/msg/TransformStamped.h"
#include "carla/ros2/types/msg/Twist.h"
#include "carla/ros2/types/msg/TwistWithCovariance.h"
#include "carla/ros2/types/msg/Vector3.h"

namespace carla {
namespace ros2 {

/// Per-type metadata needed by the DDS middleware layer.
///
/// type_name()           — ROS2-compatible DDS type name string used when
///                         registering the type with a DomainParticipant.
///                         Follows the "pkg::msg::dds_::TypeName_" pattern.
///
/// max_serialized_size() — Initial preallocation hint for the CDR payload size
///                         in bytes, excluding the 4-byte DDS encapsulation
///                         header. Used by FastDDS to pre-allocate payload
///                         buffers. For types with variable-length fields
///                         (strings, vectors) this is a minimum hint, not a
///                         hard limit. The actual size per message instance is
///                         computed dynamically by cdr_serialized_size().
///
/// Primary template is intentionally undefined — only specializations are
/// valid.
template<typename T> struct CdrTopicInfo;

// ==========================================================================
// Specializations — ordered alphabetically by C++ type name
// ==========================================================================

template<> struct CdrTopicInfo<msg::AckermannDrive> {
  static const char* type_name() {
    return "ackermann_msgs::msg::dds_::AckermannDrive_";
  }
  static size_t max_serialized_size() { return 20u; }
};

template<> struct CdrTopicInfo<msg::AckermannDriveStamped> {
  static const char* type_name() {
    return "ackermann_msgs::msg::dds_::AckermannDriveStamped_";
  }
  static size_t max_serialized_size() { return 288u; }
};

template<> struct CdrTopicInfo<msg::CameraInfo> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::CameraInfo_";
  }
  static size_t max_serialized_size() { return 3793u; }
};

template<> struct CdrTopicInfo<msg::CarlaCollisionEvent> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaCollisionEvent_";
  }
  static size_t max_serialized_size() { return 296u; }
};

template<> struct CdrTopicInfo<msg::CarlaEgoVehicleControl> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaEgoVehicleControl_";
  }
  static size_t max_serialized_size() { return 289u; }
};

template<> struct CdrTopicInfo<msg::CarlaLineInvasion> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::LaneInvasionEvent_";
  }
  static size_t max_serialized_size() { return 672u; }
};

template<> struct CdrTopicInfo<msg::Clock> {
  static const char* type_name() {
    return "rosgraph_msgs::msg::dds_::Clock_";
  }
  // Clock holds one Time (8 bytes = 2 × int32).
  static size_t max_serialized_size() { return 8u; }
};

template<> struct CdrTopicInfo<msg::Float32> {
  static const char* type_name() {
    return "std_msgs::msg::dds_::Float32_";
  }
  static size_t max_serialized_size() { return 4u; }
};

template<> struct CdrTopicInfo<msg::Header> {
  static const char* type_name() {
    return "std_msgs::msg::dds_::Header_";
  }
  static size_t max_serialized_size() { return 268u; }
};

template<> struct CdrTopicInfo<msg::Image> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::Image_";
  }
  static size_t max_serialized_size() { return 648u; }
};

template<> struct CdrTopicInfo<msg::Imu> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::Imu_";
  }
  static size_t max_serialized_size() { return 568u; }
};

template<> struct CdrTopicInfo<msg::NavSatFix> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::NavSatFix_";
  }
  static size_t max_serialized_size() { return 369u; }
};

template<> struct CdrTopicInfo<msg::NavSatStatus> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::NavSatStatus_";
  }
  static size_t max_serialized_size() { return 4u; }
};

template<> struct CdrTopicInfo<msg::Odometry> {
  static const char* type_name() {
    return "nav_msgs::msg::dds_::Odometry_";
  }
  static size_t max_serialized_size() { return 1208u; }
};

template<> struct CdrTopicInfo<msg::Point> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Point_";
  }
  static size_t max_serialized_size() { return 24u; }
};

template<> struct CdrTopicInfo<msg::Point32> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Point32_";
  }
  static size_t max_serialized_size() { return 12u; }
};

template<> struct CdrTopicInfo<msg::PointCloud2> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::PointCloud2_";
  }
  static size_t max_serialized_size() { return 27597u; }
};

template<> struct CdrTopicInfo<msg::PointField> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::PointField_";
  }
  static size_t max_serialized_size() { return 272u; }
};

template<> struct CdrTopicInfo<msg::Pose> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Pose_";
  }
  static size_t max_serialized_size() { return 56u; }
};

template<> struct CdrTopicInfo<msg::PoseWithCovariance> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::PoseWithCovariance_";
  }
  static size_t max_serialized_size() { return 344u; }
};

template<> struct CdrTopicInfo<msg::Quaternion> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Quaternion_";
  }
  static size_t max_serialized_size() { return 32u; }
};

template<> struct CdrTopicInfo<msg::RegionOfInterest> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::RegionOfInterest_";
  }
  static size_t max_serialized_size() { return 17u; }
};

template<> struct CdrTopicInfo<msg::String> {
  static const char* type_name() {
    return "std_msgs::msg::dds_::String_";
  }
  static size_t max_serialized_size() { return 260u; }
};

template<> struct CdrTopicInfo<msg::TF2Error> {
  static const char* type_name() {
    return "tf2_msgs::msg::dds_::TF2Error_";
  }
  static size_t max_serialized_size() { return 264u; }
};

template<> struct CdrTopicInfo<msg::TFMessage> {
  static const char* type_name() {
    return "tf2_msgs::msg::dds_::TFMessage_";
  }
  static size_t max_serialized_size() { return 58408u; }
};

template<> struct CdrTopicInfo<msg::Time> {
  static const char* type_name() {
    return "builtin_interfaces::msg::dds_::Time_";
  }
  static size_t max_serialized_size() { return 8u; }
};

template<> struct CdrTopicInfo<msg::Transform> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Transform_";
  }
  static size_t max_serialized_size() { return 56u; }
};

template<> struct CdrTopicInfo<msg::TransformStamped> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::TransformStamped_";
  }
  static size_t max_serialized_size() { return 584u; }
};

template<> struct CdrTopicInfo<msg::Twist> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Twist_";
  }
  static size_t max_serialized_size() { return 48u; }
};

template<> struct CdrTopicInfo<msg::TwistWithCovariance> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::TwistWithCovariance_";
  }
  static size_t max_serialized_size() { return 336u; }
};

template<> struct CdrTopicInfo<msg::Vector3> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Vector3_";
  }
  static size_t max_serialized_size() { return 24u; }
};

} // namespace ros2
} // namespace carla
