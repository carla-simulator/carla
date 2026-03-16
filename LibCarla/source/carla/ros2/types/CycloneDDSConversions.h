// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// POD message types
#include "carla/ros2/types/msg/Time.h"
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Vector3.h"
#include "carla/ros2/types/msg/Quaternion.h"
#include "carla/ros2/types/msg/Point.h"
#include "carla/ros2/types/msg/Point32.h"
#include "carla/ros2/types/msg/Pose.h"
#include "carla/ros2/types/msg/PoseWithCovariance.h"
#include "carla/ros2/types/msg/Twist.h"
#include "carla/ros2/types/msg/TwistWithCovariance.h"
#include "carla/ros2/types/msg/Transform.h"
#include "carla/ros2/types/msg/TransformStamped.h"
#include "carla/ros2/types/msg/Odometry.h"
#include "carla/ros2/types/msg/RegionOfInterest.h"
#include "carla/ros2/types/msg/PointField.h"
#include "carla/ros2/types/msg/NavSatStatus.h"
#include "carla/ros2/types/msg/NavSatFix.h"
#include "carla/ros2/types/msg/Clock.h"
#include "carla/ros2/types/msg/Float32.h"
#include "carla/ros2/types/msg/String.h"
#include "carla/ros2/types/msg/Imu.h"
#include "carla/ros2/types/msg/Image.h"
#include "carla/ros2/types/msg/CameraInfo.h"
#include "carla/ros2/types/msg/PointCloud2.h"
#include "carla/ros2/types/msg/TFMessage.h"
#include "carla/ros2/types/msg/TF2Error.h"
#include "carla/ros2/types/msg/AckermannDrive.h"
#include "carla/ros2/types/msg/AckermannDriveStamped.h"
#include "carla/ros2/types/msg/CarlaCollisionEvent.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/msg/CarlaLineInvasion.h"

// CycloneDDS-generated C types (full paths to avoid ambiguity with FastDDS types)
#include "carla/ros2/types/cyclonedds/builtin_interfaces/Time.h"
#include "carla/ros2/types/cyclonedds/std_msgs/Header.h"
#include "carla/ros2/types/cyclonedds/std_msgs/Float32.h"
#include "carla/ros2/types/cyclonedds/std_msgs/String.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Vector3.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Quaternion.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Point.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Point32.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Pose.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/PoseWithCovariance.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Twist.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/TwistWithCovariance.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/Transform.h"
#include "carla/ros2/types/cyclonedds/geometry_msgs/TransformStamped.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/RegionOfInterest.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/PointField.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/NavSatStatus.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/NavSatFix.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/Imu.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/Image.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/CameraInfo.h"
#include "carla/ros2/types/cyclonedds/sensor_msgs/PointCloud2.h"
#include "carla/ros2/types/cyclonedds/rosgraph_msgs/Clock.h"
#include "carla/ros2/types/cyclonedds/nav_msgs/Odometry.h"
#include "carla/ros2/types/cyclonedds/tf2_msgs/TFMessage.h"
#include "carla/ros2/types/cyclonedds/tf2_msgs/TF2Error.h"
#include "carla/ros2/types/cyclonedds/ackermann_msgs/AckermannDrive.h"
#include "carla/ros2/types/cyclonedds/ackermann_msgs/AckermannDriveStamped.h"
#include "carla/ros2/types/cyclonedds/carla_msgs/CarlaCollisionEvent.h"
#include "carla/ros2/types/cyclonedds/carla_msgs/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/cyclonedds/carla_msgs/CarlaLineInvasion.h"

#include <dds/dds.h>

#include <cstring>  // memcpy, memset
#include <string>

namespace carla {
namespace ros2 {

// ============================================================
// POD <-> CycloneDDS C type conversions
// Order matters: primitives first, then composites that call them.
// ============================================================

// --- Primitives ---

inline void to_cyclonedds(const msg::Time& src, builtin_interfaces_msg_Time& dst) {
  dst.sec = src.sec;
  dst.nanosec = src.nanosec;
}
inline void from_cyclonedds(const builtin_interfaces_msg_Time& src, msg::Time& dst) {
  dst.sec = src.sec;
  dst.nanosec = src.nanosec;
}

inline void to_cyclonedds(const msg::Header& src, std_msgs_msg_Header& dst) {
  to_cyclonedds(src.stamp, dst.stamp);
  dst.frame_id = dds_string_dup(src.frame_id.c_str());
}
inline void from_cyclonedds(const std_msgs_msg_Header& src, msg::Header& dst) {
  from_cyclonedds(src.stamp, dst.stamp);
  dst.frame_id = src.frame_id ? src.frame_id : "";
}

inline void to_cyclonedds(const msg::Vector3& src, geometry_msgs_msg_Vector3& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
}
inline void from_cyclonedds(const geometry_msgs_msg_Vector3& src, msg::Vector3& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
}

inline void to_cyclonedds(const msg::Quaternion& src, geometry_msgs_msg_Quaternion& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
  dst.w = src.w;
}
inline void from_cyclonedds(const geometry_msgs_msg_Quaternion& src, msg::Quaternion& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
  dst.w = src.w;
}

inline void to_cyclonedds(const msg::Point& src, geometry_msgs_msg_Point& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
}
inline void from_cyclonedds(const geometry_msgs_msg_Point& src, msg::Point& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
}

inline void to_cyclonedds(const msg::Point32& src, geometry_msgs_msg_Point32& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
}
inline void from_cyclonedds(const geometry_msgs_msg_Point32& src, msg::Point32& dst) {
  dst.x = src.x;
  dst.y = src.y;
  dst.z = src.z;
}

// --- Geometry composites ---

inline void to_cyclonedds(const msg::Pose& src, geometry_msgs_msg_Pose& dst) {
  to_cyclonedds(src.position, dst.position);
  to_cyclonedds(src.orientation, dst.orientation);
}
inline void from_cyclonedds(const geometry_msgs_msg_Pose& src, msg::Pose& dst) {
  from_cyclonedds(src.position, dst.position);
  from_cyclonedds(src.orientation, dst.orientation);
}

// Note: CycloneDDS field is pose_value (renamed to avoid IDL collision)
inline void to_cyclonedds(const msg::PoseWithCovariance& src, geometry_msgs_msg_PoseWithCovariance& dst) {
  to_cyclonedds(src.pose, dst.pose_value);
  memcpy(dst.covariance, src.covariance.data(), 36 * sizeof(double));
}
inline void from_cyclonedds(const geometry_msgs_msg_PoseWithCovariance& src, msg::PoseWithCovariance& dst) {
  from_cyclonedds(src.pose_value, dst.pose);
  memcpy(dst.covariance.data(), src.covariance, 36 * sizeof(double));
}

inline void to_cyclonedds(const msg::Twist& src, geometry_msgs_msg_Twist& dst) {
  to_cyclonedds(src.linear, dst.linear);
  to_cyclonedds(src.angular, dst.angular);
}
inline void from_cyclonedds(const geometry_msgs_msg_Twist& src, msg::Twist& dst) {
  from_cyclonedds(src.linear, dst.linear);
  from_cyclonedds(src.angular, dst.angular);
}

// Note: CycloneDDS field is twist_value (renamed to avoid IDL collision)
inline void to_cyclonedds(const msg::TwistWithCovariance& src, geometry_msgs_msg_TwistWithCovariance& dst) {
  to_cyclonedds(src.twist, dst.twist_value);
  memcpy(dst.covariance, src.covariance.data(), 36 * sizeof(double));
}
inline void from_cyclonedds(const geometry_msgs_msg_TwistWithCovariance& src, msg::TwistWithCovariance& dst) {
  from_cyclonedds(src.twist_value, dst.twist);
  memcpy(dst.covariance.data(), src.covariance, 36 * sizeof(double));
}

inline void to_cyclonedds(const msg::Transform& src, geometry_msgs_msg_Transform& dst) {
  to_cyclonedds(src.translation, dst.translation);
  to_cyclonedds(src.rotation, dst.rotation);
}
inline void from_cyclonedds(const geometry_msgs_msg_Transform& src, msg::Transform& dst) {
  from_cyclonedds(src.translation, dst.translation);
  from_cyclonedds(src.rotation, dst.rotation);
}

// Note: CycloneDDS field is transform_value (renamed to avoid IDL collision)
inline void to_cyclonedds(const msg::TransformStamped& src, geometry_msgs_msg_TransformStamped& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.child_frame_id = dds_string_dup(src.child_frame_id.c_str());
  to_cyclonedds(src.transform, dst.transform_value);
}
inline void from_cyclonedds(const geometry_msgs_msg_TransformStamped& src, msg::TransformStamped& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.child_frame_id = src.child_frame_id ? src.child_frame_id : "";
  from_cyclonedds(src.transform_value, dst.transform);
}

// --- Sensor primitives ---

inline void to_cyclonedds(const msg::RegionOfInterest& src, sensor_msgs_msg_RegionOfInterest& dst) {
  dst.x_offset = src.x_offset;
  dst.y_offset = src.y_offset;
  dst.height = src.height;
  dst.width = src.width;
  dst.do_rectify = src.do_rectify;
}
inline void from_cyclonedds(const sensor_msgs_msg_RegionOfInterest& src, msg::RegionOfInterest& dst) {
  dst.x_offset = src.x_offset;
  dst.y_offset = src.y_offset;
  dst.height = src.height;
  dst.width = src.width;
  dst.do_rectify = src.do_rectify;
}

inline void to_cyclonedds(const msg::PointField& src, sensor_msgs_msg_PointField& dst) {
  dst.name = dds_string_dup(src.name.c_str());
  dst.offset = src.offset;
  dst.datatype = src.datatype;
  dst.count = src.count;
}
inline void from_cyclonedds(const sensor_msgs_msg_PointField& src, msg::PointField& dst) {
  dst.name = src.name ? src.name : "";
  dst.offset = src.offset;
  dst.datatype = src.datatype;
  dst.count = src.count;
}

inline void to_cyclonedds(const msg::NavSatStatus& src, sensor_msgs_msg_NavSatStatus& dst) {
  dst.status = src.status;
  dst.service = src.service;
}
inline void from_cyclonedds(const sensor_msgs_msg_NavSatStatus& src, msg::NavSatStatus& dst) {
  dst.status = src.status;
  dst.service = src.service;
}

// --- Simple messages ---

// Note: CycloneDDS field is clock_time (renamed to avoid IDL collision)
inline void to_cyclonedds(const msg::Clock& src, rosgraph_msgs_msg_Clock& dst) {
  to_cyclonedds(src.clock, dst.clock_time);
}
inline void from_cyclonedds(const rosgraph_msgs_msg_Clock& src, msg::Clock& dst) {
  from_cyclonedds(src.clock_time, dst.clock);
}

inline void to_cyclonedds(const msg::Float32& src, std_msgs_msg_Float32& dst) {
  dst.data = src.data;
}
inline void from_cyclonedds(const std_msgs_msg_Float32& src, msg::Float32& dst) {
  dst.data = src.data;
}

inline void to_cyclonedds(const msg::String& src, std_msgs_msg_StringMsg& dst) {
  dst.data = dds_string_dup(src.data.c_str());
}
inline void from_cyclonedds(const std_msgs_msg_StringMsg& src, msg::String& dst) {
  dst.data = src.data ? src.data : "";
}

// --- Complex sensor messages ---

inline void to_cyclonedds(const msg::Imu& src, sensor_msgs_msg_Imu& dst) {
  to_cyclonedds(src.header, dst.header);
  to_cyclonedds(src.orientation, dst.orientation);
  memcpy(dst.orientation_covariance, src.orientation_covariance.data(), 9 * sizeof(double));
  to_cyclonedds(src.angular_velocity, dst.angular_velocity);
  memcpy(dst.angular_velocity_covariance, src.angular_velocity_covariance.data(), 9 * sizeof(double));
  to_cyclonedds(src.linear_acceleration, dst.linear_acceleration);
  memcpy(dst.linear_acceleration_covariance, src.linear_acceleration_covariance.data(), 9 * sizeof(double));
}
inline void from_cyclonedds(const sensor_msgs_msg_Imu& src, msg::Imu& dst) {
  from_cyclonedds(src.header, dst.header);
  from_cyclonedds(src.orientation, dst.orientation);
  memcpy(dst.orientation_covariance.data(), src.orientation_covariance, 9 * sizeof(double));
  from_cyclonedds(src.angular_velocity, dst.angular_velocity);
  memcpy(dst.angular_velocity_covariance.data(), src.angular_velocity_covariance, 9 * sizeof(double));
  from_cyclonedds(src.linear_acceleration, dst.linear_acceleration);
  memcpy(dst.linear_acceleration_covariance.data(), src.linear_acceleration_covariance, 9 * sizeof(double));
}

inline void to_cyclonedds(const msg::NavSatFix& src, sensor_msgs_msg_NavSatFix& dst) {
  to_cyclonedds(src.header, dst.header);
  to_cyclonedds(src.status, dst.status);
  dst.latitude = src.latitude;
  dst.longitude = src.longitude;
  dst.altitude = src.altitude;
  memcpy(dst.position_covariance, src.position_covariance.data(), 9 * sizeof(double));
  dst.position_covariance_type = src.position_covariance_type;
}
inline void from_cyclonedds(const sensor_msgs_msg_NavSatFix& src, msg::NavSatFix& dst) {
  from_cyclonedds(src.header, dst.header);
  from_cyclonedds(src.status, dst.status);
  dst.latitude = src.latitude;
  dst.longitude = src.longitude;
  dst.altitude = src.altitude;
  memcpy(dst.position_covariance.data(), src.position_covariance, 9 * sizeof(double));
  dst.position_covariance_type = src.position_covariance_type;
}

// Image: borrow data buffer for publish (zero-copy; valid because dds_write serializes synchronously)
inline void to_cyclonedds(const msg::Image& src, sensor_msgs_msg_Image& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.height = src.height;
  dst.width = src.width;
  dst.encoding = dds_string_dup(src.encoding.c_str());
  dst.is_bigendian = src.is_bigendian;
  dst.step = src.step;
  dst.data._length = static_cast<uint32_t>(src.data.size());
  dst.data._maximum = dst.data._length;
  dst.data._buffer = const_cast<uint8_t*>(src.data.data());
  dst.data._release = false;  // don't free — owned by POD
}
inline void from_cyclonedds(const sensor_msgs_msg_Image& src, msg::Image& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.height = src.height;
  dst.width = src.width;
  dst.encoding = src.encoding ? src.encoding : "";
  dst.is_bigendian = src.is_bigendian;
  dst.step = src.step;
  dst.data.assign(src.data._buffer, src.data._buffer + src.data._length);
}

inline void to_cyclonedds(const msg::CameraInfo& src, sensor_msgs_msg_CameraInfo& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.height = src.height;
  dst.width = src.width;
  dst.distortion_model = dds_string_dup(src.distortion_model.c_str());
  // d: variable-length sequence of doubles
  dst.d._length = static_cast<uint32_t>(src.d.size());
  dst.d._maximum = dst.d._length;
  if (dst.d._length > 0) {
    dst.d._buffer = static_cast<double*>(dds_alloc(dst.d._length * sizeof(double)));
    memcpy(dst.d._buffer, src.d.data(), dst.d._length * sizeof(double));
    dst.d._release = true;
  } else {
    dst.d._buffer = nullptr;
    dst.d._release = false;
  }
  memcpy(dst.k, src.k.data(), 9 * sizeof(double));
  memcpy(dst.r, src.r.data(), 9 * sizeof(double));
  memcpy(dst.p, src.p.data(), 12 * sizeof(double));
  dst.binning_x = src.binning_x;
  dst.binning_y = src.binning_y;
  to_cyclonedds(src.roi, dst.roi);
}
inline void from_cyclonedds(const sensor_msgs_msg_CameraInfo& src, msg::CameraInfo& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.height = src.height;
  dst.width = src.width;
  dst.distortion_model = src.distortion_model ? src.distortion_model : "";
  dst.d.assign(src.d._buffer, src.d._buffer + src.d._length);
  memcpy(dst.k.data(), src.k, 9 * sizeof(double));
  memcpy(dst.r.data(), src.r, 9 * sizeof(double));
  memcpy(dst.p.data(), src.p, 12 * sizeof(double));
  dst.binning_x = src.binning_x;
  dst.binning_y = src.binning_y;
  from_cyclonedds(src.roi, dst.roi);
}

// PointCloud2: struct sequence (fields) + byte sequence (data)
inline void to_cyclonedds(const msg::PointCloud2& src, sensor_msgs_msg_PointCloud2& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.height = src.height;
  dst.width = src.width;
  // fields: sequence of PointField structs
  dst.fields._length = static_cast<uint32_t>(src.fields.size());
  dst.fields._maximum = dst.fields._length;
  if (dst.fields._length > 0) {
    dst.fields._buffer = static_cast<sensor_msgs_msg_PointField*>(
        dds_alloc(dst.fields._length * sizeof(sensor_msgs_msg_PointField)));
    memset(dst.fields._buffer, 0, dst.fields._length * sizeof(sensor_msgs_msg_PointField));
    dst.fields._release = true;
    for (uint32_t i = 0; i < dst.fields._length; ++i)
      to_cyclonedds(src.fields[i], dst.fields._buffer[i]);
  } else {
    dst.fields._buffer = nullptr;
    dst.fields._release = false;
  }
  dst.is_bigendian = src.is_bigendian;
  dst.point_step = src.point_step;
  dst.row_step = src.row_step;
  // data: borrow buffer (zero-copy for publish)
  dst.data._length = static_cast<uint32_t>(src.data.size());
  dst.data._maximum = dst.data._length;
  dst.data._buffer = const_cast<uint8_t*>(src.data.data());
  dst.data._release = false;
  dst.is_dense = src.is_dense;
}
inline void from_cyclonedds(const sensor_msgs_msg_PointCloud2& src, msg::PointCloud2& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.height = src.height;
  dst.width = src.width;
  dst.fields.resize(src.fields._length);
  for (uint32_t i = 0; i < src.fields._length; ++i)
    from_cyclonedds(src.fields._buffer[i], dst.fields[i]);
  dst.is_bigendian = src.is_bigendian;
  dst.point_step = src.point_step;
  dst.row_step = src.row_step;
  dst.data.assign(src.data._buffer, src.data._buffer + src.data._length);
  dst.is_dense = src.is_dense;
}

// --- tf2 messages ---

inline void to_cyclonedds(const msg::TFMessage& src, tf2_msgs_msg_TFMessage& dst) {
  dst.transforms._length = static_cast<uint32_t>(src.transforms.size());
  dst.transforms._maximum = dst.transforms._length;
  if (dst.transforms._length > 0) {
    dst.transforms._buffer = static_cast<geometry_msgs_msg_TransformStamped*>(
        dds_alloc(dst.transforms._length * sizeof(geometry_msgs_msg_TransformStamped)));
    memset(dst.transforms._buffer, 0, dst.transforms._length * sizeof(geometry_msgs_msg_TransformStamped));
    dst.transforms._release = true;
    for (uint32_t i = 0; i < dst.transforms._length; ++i)
      to_cyclonedds(src.transforms[i], dst.transforms._buffer[i]);
  } else {
    dst.transforms._buffer = nullptr;
    dst.transforms._release = false;
  }
}
inline void from_cyclonedds(const tf2_msgs_msg_TFMessage& src, msg::TFMessage& dst) {
  dst.transforms.resize(src.transforms._length);
  for (uint32_t i = 0; i < src.transforms._length; ++i)
    from_cyclonedds(src.transforms._buffer[i], dst.transforms[i]);
}

inline void to_cyclonedds(const msg::TF2Error& src, tf2_msgs_msg_TF2Error& dst) {
  dst.error = src.error;
  dst.error_string = dds_string_dup(src.error_string.c_str());
}
inline void from_cyclonedds(const tf2_msgs_msg_TF2Error& src, msg::TF2Error& dst) {
  dst.error = src.error;
  dst.error_string = src.error_string ? src.error_string : "";
}

// --- Navigation ---

inline void to_cyclonedds(const msg::Odometry& src, nav_msgs_msg_Odometry& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.child_frame_id = dds_string_dup(src.child_frame_id.c_str());
  to_cyclonedds(src.pose, dst.pose);
  to_cyclonedds(src.twist, dst.twist);
}
inline void from_cyclonedds(const nav_msgs_msg_Odometry& src, msg::Odometry& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.child_frame_id = src.child_frame_id ? src.child_frame_id : "";
  from_cyclonedds(src.pose, dst.pose);
  from_cyclonedds(src.twist, dst.twist);
}

// --- Ackermann ---

inline void to_cyclonedds(const msg::AckermannDrive& src, ackermann_msgs_msg_AckermannDrive& dst) {
  dst.steering_angle = src.steering_angle;
  dst.steering_angle_velocity = src.steering_angle_velocity;
  dst.speed = src.speed;
  dst.acceleration = src.acceleration;
  dst.jerk = src.jerk;
}
inline void from_cyclonedds(const ackermann_msgs_msg_AckermannDrive& src, msg::AckermannDrive& dst) {
  dst.steering_angle = src.steering_angle;
  dst.steering_angle_velocity = src.steering_angle_velocity;
  dst.speed = src.speed;
  dst.acceleration = src.acceleration;
  dst.jerk = src.jerk;
}

inline void to_cyclonedds(const msg::AckermannDriveStamped& src, ackermann_msgs_msg_AckermannDriveStamped& dst) {
  to_cyclonedds(src.header, dst.header);
  to_cyclonedds(src.drive, dst.drive);
}
inline void from_cyclonedds(const ackermann_msgs_msg_AckermannDriveStamped& src, msg::AckermannDriveStamped& dst) {
  from_cyclonedds(src.header, dst.header);
  from_cyclonedds(src.drive, dst.drive);
}

// --- CARLA custom messages ---

inline void to_cyclonedds(const msg::CarlaCollisionEvent& src, carla_msgs_msg_CarlaCollisionEvent& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.other_actor_id = src.other_actor_id;
  to_cyclonedds(src.normal_impulse, dst.normal_impulse);
}
inline void from_cyclonedds(const carla_msgs_msg_CarlaCollisionEvent& src, msg::CarlaCollisionEvent& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.other_actor_id = src.other_actor_id;
  from_cyclonedds(src.normal_impulse, dst.normal_impulse);
}

inline void to_cyclonedds(const msg::CarlaEgoVehicleControl& src, carla_msgs_msg_CarlaEgoVehicleControl& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.throttle = src.throttle;
  dst.steer = src.steer;
  dst.brake = src.brake;
  dst.hand_brake = src.hand_brake;
  dst.reverse = src.reverse;
  dst.gear = src.gear;
  dst.manual_gear_shift = src.manual_gear_shift;
}
inline void from_cyclonedds(const carla_msgs_msg_CarlaEgoVehicleControl& src, msg::CarlaEgoVehicleControl& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.throttle = src.throttle;
  dst.steer = src.steer;
  dst.brake = src.brake;
  dst.hand_brake = src.hand_brake;
  dst.reverse = src.reverse;
  dst.gear = src.gear;
  dst.manual_gear_shift = src.manual_gear_shift;
}

// Note: POD is CarlaLineInvasion, CycloneDDS type is LaneInvasionEvent
inline void to_cyclonedds(const msg::CarlaLineInvasion& src, carla_msgs_msg_LaneInvasionEvent& dst) {
  to_cyclonedds(src.header, dst.header);
  dst.crossed_lane_markings._length = static_cast<uint32_t>(src.crossed_lane_markings.size());
  dst.crossed_lane_markings._maximum = dst.crossed_lane_markings._length;
  if (dst.crossed_lane_markings._length > 0) {
    dst.crossed_lane_markings._buffer = static_cast<int32_t*>(
        dds_alloc(dst.crossed_lane_markings._length * sizeof(int32_t)));
    memcpy(dst.crossed_lane_markings._buffer, src.crossed_lane_markings.data(),
           dst.crossed_lane_markings._length * sizeof(int32_t));
    dst.crossed_lane_markings._release = true;
  } else {
    dst.crossed_lane_markings._buffer = nullptr;
    dst.crossed_lane_markings._release = false;
  }
}
inline void from_cyclonedds(const carla_msgs_msg_LaneInvasionEvent& src, msg::CarlaLineInvasion& dst) {
  from_cyclonedds(src.header, dst.header);
  dst.crossed_lane_markings.assign(src.crossed_lane_markings._buffer,
                                    src.crossed_lane_markings._buffer + src.crossed_lane_markings._length);
}

} // namespace ros2
} // namespace carla
