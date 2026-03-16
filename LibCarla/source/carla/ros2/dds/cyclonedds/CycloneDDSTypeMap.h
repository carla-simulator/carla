// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/types/CycloneDDSConversions.h"

namespace carla {
namespace ros2 {

/// Maps a POD msg type to its CycloneDDS C type and topic descriptor.
/// Primary template is intentionally undefined — only specializations are valid.
template<typename MsgType> struct CycloneDDSTypeMap;

template<> struct CycloneDDSTypeMap<msg::Time> {
  using cdds_type = builtin_interfaces_msg_Time;
  static const dds_topic_descriptor_t* descriptor() { return &builtin_interfaces_msg_Time_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Header> {
  using cdds_type = std_msgs_msg_Header;
  static const dds_topic_descriptor_t* descriptor() { return &std_msgs_msg_Header_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Vector3> {
  using cdds_type = geometry_msgs_msg_Vector3;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Vector3_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Quaternion> {
  using cdds_type = geometry_msgs_msg_Quaternion;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Quaternion_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Point> {
  using cdds_type = geometry_msgs_msg_Point;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Point_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Point32> {
  using cdds_type = geometry_msgs_msg_Point32;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Point32_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Pose> {
  using cdds_type = geometry_msgs_msg_Pose;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Pose_desc; }
};
template<> struct CycloneDDSTypeMap<msg::PoseWithCovariance> {
  using cdds_type = geometry_msgs_msg_PoseWithCovariance;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_PoseWithCovariance_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Twist> {
  using cdds_type = geometry_msgs_msg_Twist;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Twist_desc; }
};
template<> struct CycloneDDSTypeMap<msg::TwistWithCovariance> {
  using cdds_type = geometry_msgs_msg_TwistWithCovariance;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_TwistWithCovariance_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Transform> {
  using cdds_type = geometry_msgs_msg_Transform;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_Transform_desc; }
};
template<> struct CycloneDDSTypeMap<msg::TransformStamped> {
  using cdds_type = geometry_msgs_msg_TransformStamped;
  static const dds_topic_descriptor_t* descriptor() { return &geometry_msgs_msg_TransformStamped_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Odometry> {
  using cdds_type = nav_msgs_msg_Odometry;
  static const dds_topic_descriptor_t* descriptor() { return &nav_msgs_msg_Odometry_desc; }
};
template<> struct CycloneDDSTypeMap<msg::RegionOfInterest> {
  using cdds_type = sensor_msgs_msg_RegionOfInterest;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_RegionOfInterest_desc; }
};
template<> struct CycloneDDSTypeMap<msg::PointField> {
  using cdds_type = sensor_msgs_msg_PointField;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_PointField_desc; }
};
template<> struct CycloneDDSTypeMap<msg::NavSatStatus> {
  using cdds_type = sensor_msgs_msg_NavSatStatus;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_NavSatStatus_desc; }
};
template<> struct CycloneDDSTypeMap<msg::NavSatFix> {
  using cdds_type = sensor_msgs_msg_NavSatFix;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_NavSatFix_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Clock> {
  using cdds_type = rosgraph_msgs_msg_Clock;
  static const dds_topic_descriptor_t* descriptor() { return &rosgraph_msgs_msg_Clock_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Float32> {
  using cdds_type = std_msgs_msg_Float32;
  static const dds_topic_descriptor_t* descriptor() { return &std_msgs_msg_Float32_desc; }
};
template<> struct CycloneDDSTypeMap<msg::String> {
  using cdds_type = std_msgs_msg_StringMsg;
  static const dds_topic_descriptor_t* descriptor() { return &std_msgs_msg_StringMsg_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Imu> {
  using cdds_type = sensor_msgs_msg_Imu;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_Imu_desc; }
};
template<> struct CycloneDDSTypeMap<msg::Image> {
  using cdds_type = sensor_msgs_msg_Image;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_Image_desc; }
};
template<> struct CycloneDDSTypeMap<msg::CameraInfo> {
  using cdds_type = sensor_msgs_msg_CameraInfo;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_CameraInfo_desc; }
};
template<> struct CycloneDDSTypeMap<msg::PointCloud2> {
  using cdds_type = sensor_msgs_msg_PointCloud2;
  static const dds_topic_descriptor_t* descriptor() { return &sensor_msgs_msg_PointCloud2_desc; }
};
template<> struct CycloneDDSTypeMap<msg::TFMessage> {
  using cdds_type = tf2_msgs_msg_TFMessage;
  static const dds_topic_descriptor_t* descriptor() { return &tf2_msgs_msg_TFMessage_desc; }
};
template<> struct CycloneDDSTypeMap<msg::TF2Error> {
  using cdds_type = tf2_msgs_msg_TF2Error;
  static const dds_topic_descriptor_t* descriptor() { return &tf2_msgs_msg_TF2Error_desc; }
};
template<> struct CycloneDDSTypeMap<msg::AckermannDrive> {
  using cdds_type = ackermann_msgs_msg_AckermannDrive;
  static const dds_topic_descriptor_t* descriptor() { return &ackermann_msgs_msg_AckermannDrive_desc; }
};
template<> struct CycloneDDSTypeMap<msg::AckermannDriveStamped> {
  using cdds_type = ackermann_msgs_msg_AckermannDriveStamped;
  static const dds_topic_descriptor_t* descriptor() { return &ackermann_msgs_msg_AckermannDriveStamped_desc; }
};
template<> struct CycloneDDSTypeMap<msg::CarlaCollisionEvent> {
  using cdds_type = carla_msgs_msg_CarlaCollisionEvent;
  static const dds_topic_descriptor_t* descriptor() { return &carla_msgs_msg_CarlaCollisionEvent_desc; }
};
template<> struct CycloneDDSTypeMap<msg::CarlaEgoVehicleControl> {
  using cdds_type = carla_msgs_msg_CarlaEgoVehicleControl;
  static const dds_topic_descriptor_t* descriptor() { return &carla_msgs_msg_CarlaEgoVehicleControl_desc; }
};
template<> struct CycloneDDSTypeMap<msg::CarlaLineInvasion> {
  using cdds_type = carla_msgs_msg_LaneInvasionEvent;
  static const dds_topic_descriptor_t* descriptor() { return &carla_msgs_msg_LaneInvasionEvent_desc; }
};

} // namespace ros2
} // namespace carla
