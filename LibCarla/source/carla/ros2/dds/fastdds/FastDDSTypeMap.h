// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Identity-mapping version: msg_type IS the FastDDS type, so each
// specialization maps a type to itself.

#pragma once

// PubSubType headers for type registration
#include "carla/ros2/types/NavSatFixPubSubTypes.h"
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/types/ImuPubSubTypes.h"
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
#include "carla/ros2/types/ClockPubSubTypes.h"
#include "carla/ros2/types/TFMessagePubSubTypes.h"
#include "carla/ros2/types/CarlaCollisionEventPubSubTypes.h"
#include "carla/ros2/types/CarlaEgoVehicleControlPubSubTypes.h"
#include "carla/ros2/types/AckermannDriveStampedPubSubTypes.h"

namespace carla {
namespace ros2 {

/// Maps a message type to its FastDDS native type and PubSubType.
/// Primary template is intentionally undefined — only specializations are valid.
template<typename MsgType> struct FastDDSTypeMap;

// --- Identity specializations ---
// In the current codebase, msg_type is already the FastDDS-generated type.
// Each specialization maps the type to itself and its corresponding PubSubType.

template<> struct FastDDSTypeMap<sensor_msgs::msg::NavSatFix> {
  using fastdds_type = sensor_msgs::msg::NavSatFix;
  using fastdds_pubsub_type = sensor_msgs::msg::NavSatFixPubSubType;
};

template<> struct FastDDSTypeMap<sensor_msgs::msg::Image> {
  using fastdds_type = sensor_msgs::msg::Image;
  using fastdds_pubsub_type = sensor_msgs::msg::ImagePubSubType;
};

template<> struct FastDDSTypeMap<sensor_msgs::msg::CameraInfo> {
  using fastdds_type = sensor_msgs::msg::CameraInfo;
  using fastdds_pubsub_type = sensor_msgs::msg::CameraInfoPubSubType;
};

template<> struct FastDDSTypeMap<sensor_msgs::msg::Imu> {
  using fastdds_type = sensor_msgs::msg::Imu;
  using fastdds_pubsub_type = sensor_msgs::msg::ImuPubSubType;
};

template<> struct FastDDSTypeMap<sensor_msgs::msg::PointCloud2> {
  using fastdds_type = sensor_msgs::msg::PointCloud2;
  using fastdds_pubsub_type = sensor_msgs::msg::PointCloud2PubSubType;
};

template<> struct FastDDSTypeMap<rosgraph::msg::Clock> {
  using fastdds_type = rosgraph::msg::Clock;
  using fastdds_pubsub_type = rosgraph::msg::ClockPubSubType;
};

template<> struct FastDDSTypeMap<tf2_msgs::msg::TFMessage> {
  using fastdds_type = tf2_msgs::msg::TFMessage;
  using fastdds_pubsub_type = tf2_msgs::msg::TFMessagePubSubType;
};

template<> struct FastDDSTypeMap<carla_msgs::msg::CarlaCollisionEvent> {
  using fastdds_type = carla_msgs::msg::CarlaCollisionEvent;
  using fastdds_pubsub_type = carla_msgs::msg::CarlaCollisionEventPubSubType;
};

template<> struct FastDDSTypeMap<carla_msgs::msg::CarlaEgoVehicleControl> {
  using fastdds_type = carla_msgs::msg::CarlaEgoVehicleControl;
  using fastdds_pubsub_type = carla_msgs::msg::CarlaEgoVehicleControlPubSubType;
};

template<> struct FastDDSTypeMap<ackermann_msgs::msg::AckermannDriveStamped> {
  using fastdds_type = ackermann_msgs::msg::AckermannDriveStamped;
  using fastdds_pubsub_type = ackermann_msgs::msg::AckermannDriveStampedPubSubType;
};

/// Identity conversion: copy src to dst when both types are the same.
/// In the next phase, we will replace these with real POD-to-FastDDS conversions.
template<typename T>
inline void to_fastdds(const T& src, T& dst) {
  dst = src;
}

template<typename T>
inline void from_fastdds(const T& src, T& dst) {
  dst = src;
}

} // namespace ros2
} // namespace carla
