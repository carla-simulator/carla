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

// FastDDS-generated types
#include "carla/ros2/types/fastdds/builtin_interfaces/Time.h"
#include "carla/ros2/types/fastdds/std_msgs/Header.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Vector3.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Quaternion.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Point.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Point32.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Pose.h"
#include "carla/ros2/types/fastdds/geometry_msgs/PoseWithCovariance.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Twist.h"
#include "carla/ros2/types/fastdds/geometry_msgs/TwistWithCovariance.h"
#include "carla/ros2/types/fastdds/geometry_msgs/Transform.h"
#include "carla/ros2/types/fastdds/geometry_msgs/TransformStamped.h"
#include "carla/ros2/types/fastdds/nav_msgs/Odometry.h"
#include "carla/ros2/types/fastdds/sensor_msgs/RegionOfInterest.h"
#include "carla/ros2/types/fastdds/sensor_msgs/PointField.h"
#include "carla/ros2/types/fastdds/sensor_msgs/NavSatStatus.h"
#include "carla/ros2/types/fastdds/sensor_msgs/NavSatFix.h"
#include "carla/ros2/types/fastdds/rosgraph_msgs/Clock.h"
#include "carla/ros2/types/fastdds/std_msgs/Float32.h"
#include "carla/ros2/types/fastdds/std_msgs/String.h"
#include "carla/ros2/types/fastdds/sensor_msgs/Imu.h"
#include "carla/ros2/types/fastdds/sensor_msgs/Image.h"
#include "carla/ros2/types/fastdds/sensor_msgs/CameraInfo.h"
#include "carla/ros2/types/fastdds/sensor_msgs/PointCloud2.h"
#include "carla/ros2/types/fastdds/tf2_msgs/TFMessage.h"
#include "carla/ros2/types/fastdds/tf2_msgs/TF2Error.h"
#include "carla/ros2/types/fastdds/ackermann_msgs/AckermannDrive.h"
#include "carla/ros2/types/fastdds/ackermann_msgs/AckermannDriveStamped.h"
#include "carla/ros2/types/fastdds/carla_msgs/CarlaCollisionEvent.h"
#include "carla/ros2/types/fastdds/carla_msgs/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/fastdds/carla_msgs/CarlaLineInvasion.h"

#include <utility> // std::move

namespace carla {
namespace ros2 {

// ============================================================
// Forward declarations — order matters (composites call primitives)
// ============================================================

// --- Primitives ---

inline void to_fastdds(const msg::Time& src, builtin_interfaces::msg::Time& dst) {
  dst.sec(src.sec);
  dst.nanosec(src.nanosec);
}
inline void from_fastdds(const builtin_interfaces::msg::Time& src, msg::Time& dst) {
  dst.sec = src.sec();
  dst.nanosec = src.nanosec();
}

inline void to_fastdds(const msg::Header& src, std_msgs::msg::Header& dst) {
  to_fastdds(src.stamp, dst.stamp());
  dst.frame_id(src.frame_id);
}
inline void from_fastdds(const std_msgs::msg::Header& src, msg::Header& dst) {
  from_fastdds(src.stamp(), dst.stamp);
  dst.frame_id = src.frame_id();
}

inline void to_fastdds(const msg::Vector3& src, geometry_msgs::msg::Vector3& dst) {
  dst.x(src.x);
  dst.y(src.y);
  dst.z(src.z);
}
inline void from_fastdds(const geometry_msgs::msg::Vector3& src, msg::Vector3& dst) {
  dst.x = src.x();
  dst.y = src.y();
  dst.z = src.z();
}

inline void to_fastdds(const msg::Quaternion& src, geometry_msgs::msg::Quaternion& dst) {
  dst.x(src.x);
  dst.y(src.y);
  dst.z(src.z);
  dst.w(src.w);
}
inline void from_fastdds(const geometry_msgs::msg::Quaternion& src, msg::Quaternion& dst) {
  dst.x = src.x();
  dst.y = src.y();
  dst.z = src.z();
  dst.w = src.w();
}

inline void to_fastdds(const msg::Point& src, geometry_msgs::msg::Point& dst) {
  dst.x(src.x);
  dst.y(src.y);
  dst.z(src.z);
}
inline void from_fastdds(const geometry_msgs::msg::Point& src, msg::Point& dst) {
  dst.x = src.x();
  dst.y = src.y();
  dst.z = src.z();
}

inline void to_fastdds(const msg::Point32& src, geometry_msgs::msg::Point32& dst) {
  dst.x(src.x);
  dst.y(src.y);
  dst.z(src.z);
}
inline void from_fastdds(const geometry_msgs::msg::Point32& src, msg::Point32& dst) {
  dst.x = src.x();
  dst.y = src.y();
  dst.z = src.z();
}

// --- Geometry composites ---

inline void to_fastdds(const msg::Pose& src, geometry_msgs::msg::Pose& dst) {
  to_fastdds(src.position, dst.position());
  to_fastdds(src.orientation, dst.orientation());
}
inline void from_fastdds(const geometry_msgs::msg::Pose& src, msg::Pose& dst) {
  from_fastdds(src.position(), dst.position);
  from_fastdds(src.orientation(), dst.orientation);
}

// Note: FastDDS field is pose_value (renamed to avoid case-insensitive collision)
inline void to_fastdds(const msg::PoseWithCovariance& src, geometry_msgs::msg::PoseWithCovariance& dst) {
  to_fastdds(src.pose, dst.pose_value());
  dst.covariance(src.covariance);
}
inline void from_fastdds(const geometry_msgs::msg::PoseWithCovariance& src, msg::PoseWithCovariance& dst) {
  from_fastdds(src.pose_value(), dst.pose);
  dst.covariance = src.covariance();
}

inline void to_fastdds(const msg::Twist& src, geometry_msgs::msg::Twist& dst) {
  to_fastdds(src.linear, dst.linear());
  to_fastdds(src.angular, dst.angular());
}
inline void from_fastdds(const geometry_msgs::msg::Twist& src, msg::Twist& dst) {
  from_fastdds(src.linear(), dst.linear);
  from_fastdds(src.angular(), dst.angular);
}

// Note: FastDDS field is twist_value (renamed to avoid case-insensitive collision)
inline void to_fastdds(const msg::TwistWithCovariance& src, geometry_msgs::msg::TwistWithCovariance& dst) {
  to_fastdds(src.twist, dst.twist_value());
  dst.covariance(src.covariance);
}
inline void from_fastdds(const geometry_msgs::msg::TwistWithCovariance& src, msg::TwistWithCovariance& dst) {
  from_fastdds(src.twist_value(), dst.twist);
  dst.covariance = src.covariance();
}

inline void to_fastdds(const msg::Transform& src, geometry_msgs::msg::Transform& dst) {
  to_fastdds(src.translation, dst.translation());
  to_fastdds(src.rotation, dst.rotation());
}
inline void from_fastdds(const geometry_msgs::msg::Transform& src, msg::Transform& dst) {
  from_fastdds(src.translation(), dst.translation);
  from_fastdds(src.rotation(), dst.rotation);
}

// Note: FastDDS field is transform_value (renamed to avoid case-insensitive collision)
inline void to_fastdds(const msg::TransformStamped& src, geometry_msgs::msg::TransformStamped& dst) {
  to_fastdds(src.header, dst.header());
  dst.child_frame_id(src.child_frame_id);
  to_fastdds(src.transform, dst.transform_value());
}
inline void from_fastdds(const geometry_msgs::msg::TransformStamped& src, msg::TransformStamped& dst) {
  from_fastdds(src.header(), dst.header);
  dst.child_frame_id = src.child_frame_id();
  from_fastdds(src.transform_value(), dst.transform);
}

// --- Sensor primitives ---

inline void to_fastdds(const msg::RegionOfInterest& src, sensor_msgs::msg::RegionOfInterest& dst) {
  dst.x_offset(src.x_offset);
  dst.y_offset(src.y_offset);
  dst.height(src.height);
  dst.width(src.width);
  dst.do_rectify(src.do_rectify);
}
inline void from_fastdds(const sensor_msgs::msg::RegionOfInterest& src, msg::RegionOfInterest& dst) {
  dst.x_offset = src.x_offset();
  dst.y_offset = src.y_offset();
  dst.height = src.height();
  dst.width = src.width();
  dst.do_rectify = src.do_rectify();
}

inline void to_fastdds(const msg::PointField& src, sensor_msgs::msg::PointField& dst) {
  dst.name(src.name);
  dst.offset(src.offset);
  dst.datatype(src.datatype);
  dst.count(src.count);
}
inline void from_fastdds(const sensor_msgs::msg::PointField& src, msg::PointField& dst) {
  dst.name = src.name();
  dst.offset = src.offset();
  dst.datatype = src.datatype();
  dst.count = src.count();
}

inline void to_fastdds(const msg::NavSatStatus& src, sensor_msgs::msg::NavSatStatus& dst) {
  dst.status(src.status);
  dst.service(src.service);
}
inline void from_fastdds(const sensor_msgs::msg::NavSatStatus& src, msg::NavSatStatus& dst) {
  dst.status = src.status();
  dst.service = src.service();
}

// --- Simple messages ---

// Note: FastDDS field is clock_time (renamed to avoid case-insensitive collision)
inline void to_fastdds(const msg::Clock& src, rosgraph_msgs::msg::Clock& dst) {
  to_fastdds(src.clock, dst.clock_time());
}
inline void from_fastdds(const rosgraph_msgs::msg::Clock& src, msg::Clock& dst) {
  from_fastdds(src.clock_time(), dst.clock);
}

inline void to_fastdds(const msg::Float32& src, std_msgs::msg::Float32& dst) {
  dst.data(src.data);
}
inline void from_fastdds(const std_msgs::msg::Float32& src, msg::Float32& dst) {
  dst.data = src.data();
}

inline void to_fastdds(const msg::String& src, std_msgs::msg::StringMsg& dst) {
  dst.data(src.data);
}
inline void from_fastdds(const std_msgs::msg::StringMsg& src, msg::String& dst) {
  dst.data = src.data();
}

// --- Complex sensor messages ---

inline void to_fastdds(const msg::Imu& src, sensor_msgs::msg::Imu& dst) {
  to_fastdds(src.header, dst.header());
  to_fastdds(src.orientation, dst.orientation());
  dst.orientation_covariance(src.orientation_covariance);
  to_fastdds(src.angular_velocity, dst.angular_velocity());
  dst.angular_velocity_covariance(src.angular_velocity_covariance);
  to_fastdds(src.linear_acceleration, dst.linear_acceleration());
  dst.linear_acceleration_covariance(src.linear_acceleration_covariance);
}
inline void from_fastdds(const sensor_msgs::msg::Imu& src, msg::Imu& dst) {
  from_fastdds(src.header(), dst.header);
  from_fastdds(src.orientation(), dst.orientation);
  dst.orientation_covariance = src.orientation_covariance();
  from_fastdds(src.angular_velocity(), dst.angular_velocity);
  dst.angular_velocity_covariance = src.angular_velocity_covariance();
  from_fastdds(src.linear_acceleration(), dst.linear_acceleration);
  dst.linear_acceleration_covariance = src.linear_acceleration_covariance();
}

inline void to_fastdds(const msg::NavSatFix& src, sensor_msgs::msg::NavSatFix& dst) {
  to_fastdds(src.header, dst.header());
  to_fastdds(src.status, dst.status());
  dst.latitude(src.latitude);
  dst.longitude(src.longitude);
  dst.altitude(src.altitude);
  dst.position_covariance(src.position_covariance);
  dst.position_covariance_type(src.position_covariance_type);
}
inline void from_fastdds(const sensor_msgs::msg::NavSatFix& src, msg::NavSatFix& dst) {
  from_fastdds(src.header(), dst.header);
  from_fastdds(src.status(), dst.status);
  dst.latitude = src.latitude();
  dst.longitude = src.longitude();
  dst.altitude = src.altitude();
  dst.position_covariance = src.position_covariance();
  dst.position_covariance_type = src.position_covariance_type();
}

// Image: const overload (copy)
inline void to_fastdds(const msg::Image& src, sensor_msgs::msg::Image& dst) {
  to_fastdds(src.header, dst.header());
  dst.height(src.height);
  dst.width(src.width);
  dst.encoding(src.encoding);
  dst.is_bigendian(src.is_bigendian);
  dst.step(src.step);
  dst.data(src.data);
}
// Image: non-const overload (move data vector)
inline void to_fastdds(msg::Image& src, sensor_msgs::msg::Image& dst) {
  to_fastdds(src.header, dst.header());
  dst.height(src.height);
  dst.width(src.width);
  dst.encoding(std::move(src.encoding));
  dst.is_bigendian(src.is_bigendian);
  dst.step(src.step);
  dst.data(std::move(src.data));
}
inline void from_fastdds(const sensor_msgs::msg::Image& src, msg::Image& dst) {
  from_fastdds(src.header(), dst.header);
  dst.height = src.height();
  dst.width = src.width();
  dst.encoding = src.encoding();
  dst.is_bigendian = src.is_bigendian();
  dst.step = src.step();
  dst.data = src.data();
}

inline void to_fastdds(const msg::CameraInfo& src, sensor_msgs::msg::CameraInfo& dst) {
  to_fastdds(src.header, dst.header());
  dst.height(src.height);
  dst.width(src.width);
  dst.distortion_model(src.distortion_model);
  dst.d(src.d);
  dst.k(src.k);
  dst.r(src.r);
  dst.p(src.p);
  dst.binning_x(src.binning_x);
  dst.binning_y(src.binning_y);
  to_fastdds(src.roi, dst.roi());
}
inline void from_fastdds(const sensor_msgs::msg::CameraInfo& src, msg::CameraInfo& dst) {
  from_fastdds(src.header(), dst.header);
  dst.height = src.height();
  dst.width = src.width();
  dst.distortion_model = src.distortion_model();
  dst.d = src.d();
  dst.k = src.k();
  dst.r = src.r();
  dst.p = src.p();
  dst.binning_x = src.binning_x();
  dst.binning_y = src.binning_y();
  from_fastdds(src.roi(), dst.roi);
}

// PointCloud2: const overload (copy)
inline void to_fastdds(const msg::PointCloud2& src, sensor_msgs::msg::PointCloud2& dst) {
  to_fastdds(src.header, dst.header());
  dst.height(src.height);
  dst.width(src.width);
  std::vector<sensor_msgs::msg::PointField> fields(src.fields.size());
  for (size_t i = 0; i < src.fields.size(); ++i) {
    to_fastdds(src.fields[i], fields[i]);
  }
  dst.fields(std::move(fields));
  dst.is_bigendian(src.is_bigendian);
  dst.point_step(src.point_step);
  dst.row_step(src.row_step);
  dst.data(src.data);
  dst.is_dense(src.is_dense);
}
// PointCloud2: non-const overload (move data vector)
inline void to_fastdds(msg::PointCloud2& src, sensor_msgs::msg::PointCloud2& dst) {
  to_fastdds(src.header, dst.header());
  dst.height(src.height);
  dst.width(src.width);
  std::vector<sensor_msgs::msg::PointField> fields(src.fields.size());
  for (size_t i = 0; i < src.fields.size(); ++i) {
    to_fastdds(src.fields[i], fields[i]);
  }
  dst.fields(std::move(fields));
  dst.is_bigendian(src.is_bigendian);
  dst.point_step(src.point_step);
  dst.row_step(src.row_step);
  dst.data(std::move(src.data));
  dst.is_dense(src.is_dense);
}
inline void from_fastdds(const sensor_msgs::msg::PointCloud2& src, msg::PointCloud2& dst) {
  from_fastdds(src.header(), dst.header);
  dst.height = src.height();
  dst.width = src.width();
  dst.fields.resize(src.fields().size());
  for (size_t i = 0; i < src.fields().size(); ++i) {
    from_fastdds(src.fields()[i], dst.fields[i]);
  }
  dst.is_bigendian = src.is_bigendian();
  dst.point_step = src.point_step();
  dst.row_step = src.row_step();
  dst.data = src.data();
  dst.is_dense = src.is_dense();
}

// --- tf2 messages ---

inline void to_fastdds(const msg::TFMessage& src, tf2_msgs::msg::TFMessage& dst) {
  std::vector<geometry_msgs::msg::TransformStamped> transforms(src.transforms.size());
  for (size_t i = 0; i < src.transforms.size(); ++i) {
    to_fastdds(src.transforms[i], transforms[i]);
  }
  dst.transforms(std::move(transforms));
}
inline void from_fastdds(const tf2_msgs::msg::TFMessage& src, msg::TFMessage& dst) {
  dst.transforms.resize(src.transforms().size());
  for (size_t i = 0; i < src.transforms().size(); ++i) {
    from_fastdds(src.transforms()[i], dst.transforms[i]);
  }
}

inline void to_fastdds(const msg::TF2Error& src, tf2_msgs::msg::TF2Error& dst) {
  dst.error(src.error);
  dst.error_string(src.error_string);
}
inline void from_fastdds(const tf2_msgs::msg::TF2Error& src, msg::TF2Error& dst) {
  dst.error = src.error();
  dst.error_string = src.error_string();
}

// --- Navigation ---

inline void to_fastdds(const msg::Odometry& src, nav_msgs::msg::Odometry& dst) {
  to_fastdds(src.header, dst.header());
  dst.child_frame_id(src.child_frame_id);
  to_fastdds(src.pose, dst.pose());
  to_fastdds(src.twist, dst.twist());
}
inline void from_fastdds(const nav_msgs::msg::Odometry& src, msg::Odometry& dst) {
  from_fastdds(src.header(), dst.header);
  dst.child_frame_id = src.child_frame_id();
  from_fastdds(src.pose(), dst.pose);
  from_fastdds(src.twist(), dst.twist);
}

// --- Ackermann ---

inline void to_fastdds(const msg::AckermannDrive& src, ackermann_msgs::msg::AckermannDrive& dst) {
  dst.steering_angle(src.steering_angle);
  dst.steering_angle_velocity(src.steering_angle_velocity);
  dst.speed(src.speed);
  dst.acceleration(src.acceleration);
  dst.jerk(src.jerk);
}
inline void from_fastdds(const ackermann_msgs::msg::AckermannDrive& src, msg::AckermannDrive& dst) {
  dst.steering_angle = src.steering_angle();
  dst.steering_angle_velocity = src.steering_angle_velocity();
  dst.speed = src.speed();
  dst.acceleration = src.acceleration();
  dst.jerk = src.jerk();
}

inline void to_fastdds(const msg::AckermannDriveStamped& src, ackermann_msgs::msg::AckermannDriveStamped& dst) {
  to_fastdds(src.header, dst.header());
  to_fastdds(src.drive, dst.drive());
}
inline void from_fastdds(const ackermann_msgs::msg::AckermannDriveStamped& src, msg::AckermannDriveStamped& dst) {
  from_fastdds(src.header(), dst.header);
  from_fastdds(src.drive(), dst.drive);
}

// --- CARLA custom messages ---

inline void to_fastdds(const msg::CarlaCollisionEvent& src, carla_msgs::msg::CarlaCollisionEvent& dst) {
  to_fastdds(src.header, dst.header());
  dst.other_actor_id(src.other_actor_id);
  to_fastdds(src.normal_impulse, dst.normal_impulse());
}
inline void from_fastdds(const carla_msgs::msg::CarlaCollisionEvent& src, msg::CarlaCollisionEvent& dst) {
  from_fastdds(src.header(), dst.header);
  dst.other_actor_id = src.other_actor_id();
  from_fastdds(src.normal_impulse(), dst.normal_impulse);
}

inline void to_fastdds(const msg::CarlaEgoVehicleControl& src, carla_msgs::msg::CarlaEgoVehicleControl& dst) {
  to_fastdds(src.header, dst.header());
  dst.throttle(src.throttle);
  dst.steer(src.steer);
  dst.brake(src.brake);
  dst.hand_brake(src.hand_brake);
  dst.reverse(src.reverse);
  dst.gear(src.gear);
  dst.manual_gear_shift(src.manual_gear_shift);
}
inline void from_fastdds(const carla_msgs::msg::CarlaEgoVehicleControl& src, msg::CarlaEgoVehicleControl& dst) {
  from_fastdds(src.header(), dst.header);
  dst.throttle = src.throttle();
  dst.steer = src.steer();
  dst.brake = src.brake();
  dst.hand_brake = src.hand_brake();
  dst.reverse = src.reverse();
  dst.gear = src.gear();
  dst.manual_gear_shift = src.manual_gear_shift();
}

// Note: POD is CarlaLineInvasion, FastDDS class is LaneInvasionEvent
inline void to_fastdds(const msg::CarlaLineInvasion& src, carla_msgs::msg::LaneInvasionEvent& dst) {
  to_fastdds(src.header, dst.header());
  dst.crossed_lane_markings(src.crossed_lane_markings);
}
inline void from_fastdds(const carla_msgs::msg::LaneInvasionEvent& src, msg::CarlaLineInvasion& dst) {
  from_fastdds(src.header(), dst.header);
  dst.crossed_lane_markings = src.crossed_lane_markings();
}

} // namespace ros2
} // namespace carla
