// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/FastDDSConversions.h"

// PubSubType headers for type registration
#include "carla/ros2/types/TimePubSubTypes.h"
#include "carla/ros2/types/HeaderPubSubTypes.h"
#include "carla/ros2/types/Vector3PubSubTypes.h"
#include "carla/ros2/types/QuaternionPubSubTypes.h"
#include "carla/ros2/types/PointPubSubTypes.h"
#include "carla/ros2/types/Point32PubSubTypes.h"
#include "carla/ros2/types/PosePubSubTypes.h"
#include "carla/ros2/types/PoseWithCovariancePubSubTypes.h"
#include "carla/ros2/types/TwistPubSubTypes.h"
#include "carla/ros2/types/TwistWithCovariancePubSubTypes.h"
#include "carla/ros2/types/TransformPubSubTypes.h"
#include "carla/ros2/types/TransformStampedPubSubTypes.h"
#include "carla/ros2/types/OdometryPubSubTypes.h"
#include "carla/ros2/types/RegionOfInterestPubSubTypes.h"
#include "carla/ros2/types/PointFieldPubSubTypes.h"
#include "carla/ros2/types/NavSatStatusPubSubTypes.h"
#include "carla/ros2/types/NavSatFixPubSubTypes.h"
#include "carla/ros2/types/ClockPubSubTypes.h"
#include "carla/ros2/types/Float32PubSubTypes.h"
#include "carla/ros2/types/StringPubSubTypes.h"
#include "carla/ros2/types/ImuPubSubTypes.h"
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
#include "carla/ros2/types/TFMessagePubSubTypes.h"
#include "carla/ros2/types/TF2ErrorPubSubTypes.h"
#include "carla/ros2/types/AckermannDrivePubSubTypes.h"
#include "carla/ros2/types/AckermannDriveStampedPubSubTypes.h"
#include "carla/ros2/types/CarlaCollisionEventPubSubTypes.h"
#include "carla/ros2/types/CarlaEgoVehicleControlPubSubTypes.h"
#include "carla/ros2/types/CarlaLineInvasionPubSubTypes.h"

namespace carla {
namespace ros2 {

/// Maps a message type to its FastDDS native type and PubSubType.
/// Primary template is intentionally undefined — only specializations are valid.
template<typename MsgType> struct FastDDSTypeMap;

// ============================================================
// Specializations (POD msg types -> FastDDS types)
// These map middleware-neutral POD structs to FastDDS-generated types.
// Conversion functions are in types/FastDDSConversions.h.
// ============================================================

template<> struct FastDDSTypeMap<msg::Time> {
  using fastdds_type = builtin_interfaces::msg::Time;
  using fastdds_pubsub_type = builtin_interfaces::msg::TimePubSubType;
};

template<> struct FastDDSTypeMap<msg::Header> {
  using fastdds_type = std_msgs::msg::Header;
  using fastdds_pubsub_type = std_msgs::msg::HeaderPubSubType;
};

template<> struct FastDDSTypeMap<msg::Vector3> {
  using fastdds_type = geometry_msgs::msg::Vector3;
  using fastdds_pubsub_type = geometry_msgs::msg::Vector3PubSubType;
};

template<> struct FastDDSTypeMap<msg::Quaternion> {
  using fastdds_type = geometry_msgs::msg::Quaternion;
  using fastdds_pubsub_type = geometry_msgs::msg::QuaternionPubSubType;
};

template<> struct FastDDSTypeMap<msg::Point> {
  using fastdds_type = geometry_msgs::msg::Point;
  using fastdds_pubsub_type = geometry_msgs::msg::PointPubSubType;
};

template<> struct FastDDSTypeMap<msg::Point32> {
  using fastdds_type = geometry_msgs::msg::Point32;
  using fastdds_pubsub_type = geometry_msgs::msg::Point32PubSubType;
};

template<> struct FastDDSTypeMap<msg::Pose> {
  using fastdds_type = geometry_msgs::msg::Pose;
  using fastdds_pubsub_type = geometry_msgs::msg::PosePubSubType;
};

template<> struct FastDDSTypeMap<msg::PoseWithCovariance> {
  using fastdds_type = geometry_msgs::msg::PoseWithCovariance;
  using fastdds_pubsub_type = geometry_msgs::msg::PoseWithCovariancePubSubType;
};

template<> struct FastDDSTypeMap<msg::Twist> {
  using fastdds_type = geometry_msgs::msg::Twist;
  using fastdds_pubsub_type = geometry_msgs::msg::TwistPubSubType;
};

template<> struct FastDDSTypeMap<msg::TwistWithCovariance> {
  using fastdds_type = geometry_msgs::msg::TwistWithCovariance;
  using fastdds_pubsub_type = geometry_msgs::msg::TwistWithCovariancePubSubType;
};

template<> struct FastDDSTypeMap<msg::Transform> {
  using fastdds_type = geometry_msgs::msg::Transform;
  using fastdds_pubsub_type = geometry_msgs::msg::TransformPubSubType;
};

template<> struct FastDDSTypeMap<msg::TransformStamped> {
  using fastdds_type = geometry_msgs::msg::TransformStamped;
  using fastdds_pubsub_type = geometry_msgs::msg::TransformStampedPubSubType;
};

template<> struct FastDDSTypeMap<msg::Odometry> {
  using fastdds_type = nav_msgs::msg::Odometry;
  using fastdds_pubsub_type = nav_msgs::msg::OdometryPubSubType;
};

template<> struct FastDDSTypeMap<msg::RegionOfInterest> {
  using fastdds_type = sensor_msgs::msg::RegionOfInterest;
  using fastdds_pubsub_type = sensor_msgs::msg::RegionOfInterestPubSubType;
};

template<> struct FastDDSTypeMap<msg::PointField> {
  using fastdds_type = sensor_msgs::msg::PointField;
  using fastdds_pubsub_type = sensor_msgs::msg::PointFieldPubSubType;
};

template<> struct FastDDSTypeMap<msg::NavSatStatus> {
  using fastdds_type = sensor_msgs::msg::NavSatStatus;
  using fastdds_pubsub_type = sensor_msgs::msg::NavSatStatusPubSubType;
};

template<> struct FastDDSTypeMap<msg::NavSatFix> {
  using fastdds_type = sensor_msgs::msg::NavSatFix;
  using fastdds_pubsub_type = sensor_msgs::msg::NavSatFixPubSubType;
};

template<> struct FastDDSTypeMap<msg::Clock> {
  using fastdds_type = rosgraph::msg::Clock;
  using fastdds_pubsub_type = rosgraph::msg::ClockPubSubType;
};

template<> struct FastDDSTypeMap<msg::Float32> {
  using fastdds_type = std_msgs::msg::Float32;
  using fastdds_pubsub_type = std_msgs::msg::Float32PubSubType;
};

template<> struct FastDDSTypeMap<msg::String> {
  using fastdds_type = std_msgs::msg::String;
  using fastdds_pubsub_type = std_msgs::msg::StringPubSubType;
};

template<> struct FastDDSTypeMap<msg::Imu> {
  using fastdds_type = sensor_msgs::msg::Imu;
  using fastdds_pubsub_type = sensor_msgs::msg::ImuPubSubType;
};

template<> struct FastDDSTypeMap<msg::Image> {
  using fastdds_type = sensor_msgs::msg::Image;
  using fastdds_pubsub_type = sensor_msgs::msg::ImagePubSubType;
};

template<> struct FastDDSTypeMap<msg::CameraInfo> {
  using fastdds_type = sensor_msgs::msg::CameraInfo;
  using fastdds_pubsub_type = sensor_msgs::msg::CameraInfoPubSubType;
};

template<> struct FastDDSTypeMap<msg::PointCloud2> {
  using fastdds_type = sensor_msgs::msg::PointCloud2;
  using fastdds_pubsub_type = sensor_msgs::msg::PointCloud2PubSubType;
};

template<> struct FastDDSTypeMap<msg::TFMessage> {
  using fastdds_type = tf2_msgs::msg::TFMessage;
  using fastdds_pubsub_type = tf2_msgs::msg::TFMessagePubSubType;
};

template<> struct FastDDSTypeMap<msg::TF2Error> {
  using fastdds_type = tf2_msgs::msg::TF2Error;
  using fastdds_pubsub_type = tf2_msgs::msg::TF2ErrorPubSubType;
};

template<> struct FastDDSTypeMap<msg::AckermannDrive> {
  using fastdds_type = ackermann_msgs::msg::AckermannDrive;
  using fastdds_pubsub_type = ackermann_msgs::msg::AckermannDrivePubSubType;
};

template<> struct FastDDSTypeMap<msg::AckermannDriveStamped> {
  using fastdds_type = ackermann_msgs::msg::AckermannDriveStamped;
  using fastdds_pubsub_type = ackermann_msgs::msg::AckermannDriveStampedPubSubType;
};

template<> struct FastDDSTypeMap<msg::CarlaCollisionEvent> {
  using fastdds_type = carla_msgs::msg::CarlaCollisionEvent;
  using fastdds_pubsub_type = carla_msgs::msg::CarlaCollisionEventPubSubType;
};

template<> struct FastDDSTypeMap<msg::CarlaEgoVehicleControl> {
  using fastdds_type = carla_msgs::msg::CarlaEgoVehicleControl;
  using fastdds_pubsub_type = carla_msgs::msg::CarlaEgoVehicleControlPubSubType;
};

template<> struct FastDDSTypeMap<msg::CarlaLineInvasion> {
  using fastdds_type = carla_msgs::msg::LaneInvasionEvent;
  using fastdds_pubsub_type = carla_msgs::msg::LaneInvasionEventPubSubType;
};

} // namespace ros2
} // namespace carla
