// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/exceptions/BadParamException.h>
#include <fastcdr/exceptions/Exception.h>

#include <cstdint>
#include <vector>

#include "carla/ros2/types/msg/Accel.h"
#include "carla/ros2/types/msg/AckermannDrive.h"
#include "carla/ros2/types/msg/AckermannDriveStamped.h"
#include "carla/ros2/types/msg/CameraInfo.h"
#include "carla/ros2/types/msg/CarlaCollisionEvent.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleInfo.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleInfoWheel.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleStatus.h"
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

// ==========================================================================
// Internal CDR helpers — one overload pair per message type.
// Ordered from least-dependent to most-dependent so each helper's body
// can call the helpers for its nested types without forward declarations.
//
// Wire format: OMG DDSI-RTPS v2.5 Section 10 + DDS-XTypes 1.3 clause 7.4.1.1
// (Classic CDR, encoding version 1, little-endian). Sequences are encoded
// as a uint32_t length followed by elements; strings as a uint32_t length
// (including the terminating NUL) followed by bytes; bool as a single octet.
// ==========================================================================

/// Sanity cap for the length field of a CDR sequence read from the wire.
/// Protects against malformed/hostile payloads claiming a multi-GB sequence,
/// which would otherwise OOM-abort the process inside std::vector::resize().
/// 1,048,576 elements is far above any realistic ROS2 message (PointCloud2
/// rarely has more than a dozen fields; TFMessage rarely has more than a few
/// hundred transforms) while bounding the worst-case allocation to ~80 MiB.
static constexpr uint32_t kMaxCdrSequenceElements = 1u << 20;

/// Write the uint32_t length field of a CDR sequence, rejecting sizes above the
/// sane cap so the cast to uint32_t cannot wrap and produce a malformed stream.
/// Mirrors the guard applied when reading sequence lengths back from the wire.
inline void serialize_cdr_sequence_length(
    eprosima::fastcdr::Cdr& cdr, size_t size, const char* what) {
  if (size > kMaxCdrSequenceElements) {
    throw eprosima::fastcdr::exception::BadParamException(what);
  }
  cdr << static_cast<uint32_t>(size);
}

// --------------------------------------------------------------------------
// Leaf types (no nested msg:: fields)
// --------------------------------------------------------------------------

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Time& m) {
  cdr << m.sec;
  cdr << m.nanosec;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Time& m) {
  cdr >> m.sec;
  cdr >> m.nanosec;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Vector3& m) {
  cdr << m.x;
  cdr << m.y;
  cdr << m.z;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Vector3& m) {
  cdr >> m.x;
  cdr >> m.y;
  cdr >> m.z;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Quaternion& m) {
  cdr << m.x;
  cdr << m.y;
  cdr << m.z;
  cdr << m.w;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Quaternion& m) {
  cdr >> m.x;
  cdr >> m.y;
  cdr >> m.z;
  cdr >> m.w;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Accel& m) {
  serialize_cdr(cdr, m.linear);
  serialize_cdr(cdr, m.angular);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Accel& m) {
  deserialize_cdr(cdr, m.linear);
  deserialize_cdr(cdr, m.angular);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Point& m) {
  cdr << m.x;
  cdr << m.y;
  cdr << m.z;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Point& m) {
  cdr >> m.x;
  cdr >> m.y;
  cdr >> m.z;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Point32& m) {
  cdr << m.x;
  cdr << m.y;
  cdr << m.z;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Point32& m) {
  cdr >> m.x;
  cdr >> m.y;
  cdr >> m.z;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::NavSatStatus& m) {
  cdr << m.status;
  cdr << m.service;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::NavSatStatus& m) {
  cdr >> m.status;
  cdr >> m.service;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::RegionOfInterest& m) {
  cdr << m.x_offset;
  cdr << m.y_offset;
  cdr << m.height;
  cdr << m.width;
  cdr << m.do_rectify;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::RegionOfInterest& m) {
  cdr >> m.x_offset;
  cdr >> m.y_offset;
  cdr >> m.height;
  cdr >> m.width;
  cdr >> m.do_rectify;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Float32& m) {
  cdr << m.data;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Float32& m) {
  cdr >> m.data;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::AckermannDrive& m) {
  cdr << m.steering_angle;
  cdr << m.steering_angle_velocity;
  cdr << m.speed;
  cdr << m.acceleration;
  cdr << m.jerk;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::AckermannDrive& m) {
  cdr >> m.steering_angle;
  cdr >> m.steering_angle_velocity;
  cdr >> m.speed;
  cdr >> m.acceleration;
  cdr >> m.jerk;
}

// --

/// PointField: name is a string; offset, datatype, count are primitives.
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::PointField& m) {
  cdr << m.name;
  cdr << m.offset;
  cdr << m.datatype;
  cdr << m.count;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::PointField& m) {
  cdr >> m.name;
  cdr >> m.offset;
  cdr >> m.datatype;
  cdr >> m.count;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::TF2Error& m) {
  cdr << m.error;
  cdr << m.error_string;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::TF2Error& m) {
  cdr >> m.error;
  cdr >> m.error_string;
}

// --------------------------------------------------------------------------
// Types with nested msg:: fields
// --------------------------------------------------------------------------

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Header& m) {
  serialize_cdr(cdr, m.stamp);
  cdr << m.frame_id;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Header& m) {
  deserialize_cdr(cdr, m.stamp);
  cdr >> m.frame_id;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Twist& m) {
  serialize_cdr(cdr, m.linear);
  serialize_cdr(cdr, m.angular);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Twist& m) {
  deserialize_cdr(cdr, m.linear);
  deserialize_cdr(cdr, m.angular);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Transform& m) {
  serialize_cdr(cdr, m.translation);
  serialize_cdr(cdr, m.rotation);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Transform& m) {
  deserialize_cdr(cdr, m.translation);
  deserialize_cdr(cdr, m.rotation);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Pose& m) {
  serialize_cdr(cdr, m.position);
  serialize_cdr(cdr, m.orientation);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Pose& m) {
  deserialize_cdr(cdr, m.position);
  deserialize_cdr(cdr, m.orientation);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Clock& m) {
  serialize_cdr(cdr, m.clock);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Clock& m) {
  deserialize_cdr(cdr, m.clock);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::String& m) {
  cdr << m.data;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::String& m) {
  cdr >> m.data;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::TransformStamped& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.child_frame_id;
  serialize_cdr(cdr, m.transform);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::TransformStamped& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.child_frame_id;
  deserialize_cdr(cdr, m.transform);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::TwistWithCovariance& m) {
  serialize_cdr(cdr, m.twist);
  cdr << m.covariance;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::TwistWithCovariance& m) {
  deserialize_cdr(cdr, m.twist);
  cdr >> m.covariance;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::PoseWithCovariance& m) {
  serialize_cdr(cdr, m.pose);
  cdr << m.covariance;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::PoseWithCovariance& m) {
  deserialize_cdr(cdr, m.pose);
  cdr >> m.covariance;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::AckermannDriveStamped& m) {
  serialize_cdr(cdr, m.header);
  serialize_cdr(cdr, m.drive);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::AckermannDriveStamped& m) {
  deserialize_cdr(cdr, m.header);
  deserialize_cdr(cdr, m.drive);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CarlaCollisionEvent& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.other_actor_id;
  serialize_cdr(cdr, m.normal_impulse);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CarlaCollisionEvent& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.other_actor_id;
  deserialize_cdr(cdr, m.normal_impulse);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::NavSatFix& m) {
  serialize_cdr(cdr, m.header);
  serialize_cdr(cdr, m.status);
  cdr << m.latitude;
  cdr << m.longitude;
  cdr << m.altitude;
  cdr << m.position_covariance;
  cdr << m.position_covariance_type;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::NavSatFix& m) {
  deserialize_cdr(cdr, m.header);
  deserialize_cdr(cdr, m.status);
  cdr >> m.latitude;
  cdr >> m.longitude;
  cdr >> m.altitude;
  cdr >> m.position_covariance;
  cdr >> m.position_covariance_type;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Imu& m) {
  serialize_cdr(cdr, m.header);
  serialize_cdr(cdr, m.orientation);
  cdr << m.orientation_covariance;
  serialize_cdr(cdr, m.angular_velocity);
  cdr << m.angular_velocity_covariance;
  serialize_cdr(cdr, m.linear_acceleration);
  cdr << m.linear_acceleration_covariance;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Imu& m) {
  deserialize_cdr(cdr, m.header);
  deserialize_cdr(cdr, m.orientation);
  cdr >> m.orientation_covariance;
  deserialize_cdr(cdr, m.angular_velocity);
  cdr >> m.angular_velocity_covariance;
  deserialize_cdr(cdr, m.linear_acceleration);
  cdr >> m.linear_acceleration_covariance;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CarlaEgoVehicleControl& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.throttle;
  cdr << m.steer;
  cdr << m.brake;
  cdr << m.hand_brake;
  cdr << m.reverse;
  cdr << m.gear;
  cdr << m.manual_gear_shift;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CarlaEgoVehicleControl& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.throttle;
  cdr >> m.steer;
  cdr >> m.brake;
  cdr >> m.hand_brake;
  cdr >> m.reverse;
  cdr >> m.gear;
  cdr >> m.manual_gear_shift;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CarlaEgoVehicleStatus& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.velocity;
  serialize_cdr(cdr, m.acceleration);
  serialize_cdr(cdr, m.orientation);
  serialize_cdr(cdr, m.control);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CarlaEgoVehicleStatus& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.velocity;
  deserialize_cdr(cdr, m.acceleration);
  deserialize_cdr(cdr, m.orientation);
  deserialize_cdr(cdr, m.control);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CarlaEgoVehicleInfoWheel& m) {
  cdr << m.tire_friction;
  cdr << m.damping_rate;
  cdr << m.max_steer_angle;
  cdr << m.radius;
  cdr << m.max_brake_torque;
  cdr << m.max_handbrake_torque;
  serialize_cdr(cdr, m.position);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CarlaEgoVehicleInfoWheel& m) {
  cdr >> m.tire_friction;
  cdr >> m.damping_rate;
  cdr >> m.max_steer_angle;
  cdr >> m.radius;
  cdr >> m.max_brake_torque;
  cdr >> m.max_handbrake_torque;
  deserialize_cdr(cdr, m.position);
}

// --

/// CarlaEgoVehicleInfo::wheels is a sequence of structs.
/// Write length + elements manually.
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CarlaEgoVehicleInfo& m) {
  cdr << m.id;
  cdr << m.type;
  cdr << m.rolename;
  // CDR sequence length is uint32_t per DDS-XTypes 1.3 clause 7.4.1.1.
  serialize_cdr_sequence_length(
      cdr, m.wheels.size(),
      "CarlaEgoVehicleInfo::wheels length exceeds sane CDR sequence cap");
  for (const auto& w : m.wheels) {
    serialize_cdr(cdr, w);
  }
  cdr << m.max_rpm;
  cdr << m.moi;
  cdr << m.damping_rate_full_throttle;
  cdr << m.damping_rate_zero_throttle_clutch_engaged;
  cdr << m.damping_rate_zero_throttle_clutch_disengaged;
  cdr << m.use_gear_autobox;
  cdr << m.gear_switch_time;
  cdr << m.clutch_strength;
  cdr << m.mass;
  cdr << m.drag_coefficient;
  serialize_cdr(cdr, m.center_of_mass);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CarlaEgoVehicleInfo& m) {
  cdr >> m.id;
  cdr >> m.type;
  cdr >> m.rolename;
  uint32_t wheels_size{0u};
  cdr >> wheels_size;
  if (wheels_size > kMaxCdrSequenceElements) {
    throw eprosima::fastcdr::exception::BadParamException(
        "CarlaEgoVehicleInfo::wheels length exceeds sane CDR sequence cap");
  }
  m.wheels.resize(static_cast<size_t>(wheels_size));
  for (auto& w : m.wheels) {
    deserialize_cdr(cdr, w);
  }
  cdr >> m.max_rpm;
  cdr >> m.moi;
  cdr >> m.damping_rate_full_throttle;
  cdr >> m.damping_rate_zero_throttle_clutch_engaged;
  cdr >> m.damping_rate_zero_throttle_clutch_disengaged;
  cdr >> m.use_gear_autobox;
  cdr >> m.gear_switch_time;
  cdr >> m.clutch_strength;
  cdr >> m.mass;
  cdr >> m.drag_coefficient;
  deserialize_cdr(cdr, m.center_of_mass);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CarlaLineInvasion& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.crossed_lane_markings;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CarlaLineInvasion& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.crossed_lane_markings;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Odometry& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.child_frame_id;
  serialize_cdr(cdr, m.pose);
  serialize_cdr(cdr, m.twist);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Odometry& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.child_frame_id;
  deserialize_cdr(cdr, m.pose);
  deserialize_cdr(cdr, m.twist);
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::Image& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.height;
  cdr << m.width;
  cdr << m.encoding;
  cdr << m.is_bigendian;
  cdr << m.step;
  cdr << m.data;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::Image& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.height;
  cdr >> m.width;
  cdr >> m.encoding;
  cdr >> m.is_bigendian;
  cdr >> m.step;
  cdr >> m.data;
}

// --

inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::CameraInfo& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.height;
  cdr << m.width;
  cdr << m.distortion_model;
  cdr << m.d;
  cdr << m.k;
  cdr << m.r;
  cdr << m.p;
  cdr << m.binning_x;
  cdr << m.binning_y;
  serialize_cdr(cdr, m.roi);
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::CameraInfo& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.height;
  cdr >> m.width;
  cdr >> m.distortion_model;
  cdr >> m.d;
  cdr >> m.k;
  cdr >> m.r;
  cdr >> m.p;
  cdr >> m.binning_x;
  cdr >> m.binning_y;
  deserialize_cdr(cdr, m.roi);
}

// --

/// PointCloud2::fields is a sequence of structs; FastCDR's generic vector
/// operator calls serialize() on each element, which our POD types don't
/// provide. Write length + elements manually.
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::PointCloud2& m) {
  serialize_cdr(cdr, m.header);
  cdr << m.height;
  cdr << m.width;
  // CDR sequence length is uint32_t per DDS-XTypes 1.3 clause 7.4.1.1.
  serialize_cdr_sequence_length(
      cdr, m.fields.size(),
      "PointCloud2::fields length exceeds sane CDR sequence cap");
  for (const auto& f : m.fields) {
    serialize_cdr(cdr, f);
  }
  cdr << m.is_bigendian;
  cdr << m.point_step;
  cdr << m.row_step;
  cdr << m.data;
  cdr << m.is_dense;
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::PointCloud2& m) {
  deserialize_cdr(cdr, m.header);
  cdr >> m.height;
  cdr >> m.width;
  uint32_t fields_size{0u};
  cdr >> fields_size;
  if (fields_size > kMaxCdrSequenceElements) {
    throw eprosima::fastcdr::exception::BadParamException(
        "PointCloud2::fields length exceeds sane CDR sequence cap");
  }
  m.fields.resize(static_cast<size_t>(fields_size));
  for (auto& f : m.fields) {
    deserialize_cdr(cdr, f);
  }
  cdr >> m.is_bigendian;
  cdr >> m.point_step;
  cdr >> m.row_step;
  cdr >> m.data;
  cdr >> m.is_dense;
}

// --

/// TFMessage::transforms is a sequence of TransformStamped structs.
/// Write length + elements manually.
inline void serialize_cdr(
    eprosima::fastcdr::Cdr& cdr, const msg::TFMessage& m) {
  // CDR sequence length is uint32_t per DDS-XTypes 1.3 clause 7.4.1.1.
  serialize_cdr_sequence_length(
      cdr, m.transforms.size(),
      "TFMessage::transforms length exceeds sane CDR sequence cap");
  for (const auto& t : m.transforms) {
    serialize_cdr(cdr, t);
  }
}

inline void deserialize_cdr(
    eprosima::fastcdr::Cdr& cdr, msg::TFMessage& m) {
  uint32_t transforms_size{0u};
  cdr >> transforms_size;
  if (transforms_size > kMaxCdrSequenceElements) {
    throw eprosima::fastcdr::exception::BadParamException(
        "TFMessage::transforms length exceeds sane CDR sequence cap");
  }
  m.transforms.resize(static_cast<size_t>(transforms_size));
  for (auto& t : m.transforms) {
    deserialize_cdr(cdr, t);
  }
}

// ==========================================================================
// Public API
// ==========================================================================

/// Serialize a msg::X to a CDR byte buffer including the DDS encapsulation
/// header (Classic CDR, encoding version 1, little-endian). The returned
/// buffer is wire-compatible with all ROS2 distros and can be passed to
/// FastDDS write_serialized_payload() or CycloneDDS dds_writecdr().
/// Returns an empty vector if Fast-CDR raises an exception (e.g. out of
/// memory while growing the internal FastBuffer).
template<typename T>
std::vector<uint8_t> serialize_to_cdr(const T& msg) {
  eprosima::fastcdr::FastBuffer fb;
  // Force LITTLE_ENDIANNESS so the encapsulation header is CDR_LE
  // ({0x00, 0x01}) per DDSI-RTPS v2.5 Table 10.3, regardless of host
  // endianness. ROS2 ecosystems test against CDR_LE.
  eprosima::fastcdr::Cdr cdr{
      fb,
      eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
      eprosima::fastcdr::Cdr::DDS_CDR};
  try {
    cdr.serialize_encapsulation();
    serialize_cdr(cdr, msg);
  } catch (const eprosima::fastcdr::exception::Exception&) {
    return std::vector<uint8_t>{};
  }
  const char* buf{fb.getBuffer()};
  const size_t len{cdr.getSerializedDataLength()};
  return std::vector<uint8_t>{
      reinterpret_cast<const uint8_t*>(buf),
      reinterpret_cast<const uint8_t*>(buf) + len};
}

/// Return the exact CDR-serialized size in bytes for a message instance,
/// including the 4-byte DDS encapsulation header. Used by GenericCdrPubSubType
/// to tell FastDDS the actual payload size before write(), so the payload
/// buffer is sized correctly for variable-length fields (e.g. Image::data,
/// PointCloud2::data). The result is provably equal to serialize_to_cdr(msg).size().
template<typename T>
uint32_t cdr_serialized_size(const T& msg) {
  eprosima::fastcdr::FastBuffer fb;
  eprosima::fastcdr::Cdr cdr{
      fb,
      eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
      eprosima::fastcdr::Cdr::DDS_CDR};
  cdr.serialize_encapsulation();
  serialize_cdr(cdr, msg);
  return static_cast<uint32_t>(cdr.getSerializedDataLength());
}

/// Deserialize a msg::X from a CDR byte buffer that was produced by
/// serialize_to_cdr() or by any ROS2-compatible DDS middleware.
/// Returns true on success, false on any Fast-CDR error (truncated buffer,
/// malformed encapsulation header, sequence length exceeding the sanity
/// cap, etc.). The buffer must include the 4-byte DDS encapsulation header.
template<typename T>
bool deserialize_from_cdr(
    const uint8_t* data, size_t size, T& msg) {
  eprosima::fastcdr::FastBuffer fb{
      // FastBuffer requires a non-const pointer; the buffer is only read.
      reinterpret_cast<char*>(const_cast<uint8_t*>(data)),
      size};
  eprosima::fastcdr::Cdr cdr{
      fb,
      eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
      eprosima::fastcdr::Cdr::DDS_CDR};
  try {
    cdr.read_encapsulation();
    deserialize_cdr(cdr, msg);
  } catch (const eprosima::fastcdr::exception::Exception&) {
    return false;
  }
  return true;
}

} // namespace ros2
} // namespace carla
