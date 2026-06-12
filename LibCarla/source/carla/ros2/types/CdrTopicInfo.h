// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstddef>

#include "carla/ros2/types/msg/Accel.h"
#include "carla/ros2/types/msg/AckermannDrive.h"
#include "carla/ros2/types/msg/AckermannDriveStamped.h"
#include "carla/ros2/types/msg/CameraInfo.h"
#include "carla/ros2/types/msg/CarlaBoundingBox.h"
#include "carla/ros2/types/msg/CarlaCollisionEvent.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleControl.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleInfo.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleInfoWheel.h"
#include "carla/ros2/types/msg/CarlaEgoVehicleStatus.h"
#include "carla/ros2/types/msg/CarlaLineInvasion.h"
#include "carla/ros2/types/msg/CarlaTrafficLightInfo.h"
#include "carla/ros2/types/msg/CarlaTrafficLightInfoList.h"
#include "carla/ros2/types/msg/CarlaTrafficLightStatus.h"
#include "carla/ros2/types/msg/CarlaTrafficLightStatusList.h"
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

/// Per-type metadata needed by the middleware layer.
///
/// type_name()           — ROS2-compatible type name string used when
///                         registering the type with a DomainParticipant.
///                         Follows the "pkg::msg::dds_::TypeName_" pattern.
///
/// type_hash()           — REP-2011 RIHS01 type hash string, placed in the
///                         DDS endpoint USER_DATA QoS policy (PID_USER_DATA,
///                         0x002c per OMG DDSI-RTPS v2.5 §9.6.2.2.2).
///                         Format: "RIHS01_<64 lowercase hex>" (71 chars).
///                         Consumed by UserDataFormat.h to build the
///                         REP-2016 KV payload "typehash=RIHS01_<hex>;".
///                         The hash is pinned per message definition and is
///                         stable across ROS 2 distributions. A specialization
///                         may return nullptr when no hash is available for the
///                         type; UserDataFormat.h then emits an empty user_data
///                         payload and callers skip setting the QoS field.
///                         To compute the hash for a new type, use
///                         Util/ros2/compute_type_hash.sh (Docker required).
///                         Full workflow: Docs/ros2/adding_message_types.md.
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

template<> struct CdrTopicInfo<msg::Accel> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Accel_";
  }
  static const char* type_hash() {
    return "RIHS01_dc448243ded9b1fcbcca24aba0c22f013dae06c354ba2d849571c0a2a3f57ca0";
  }
  // Two Vector3 (6 × float64).
  static size_t max_serialized_size() { return 48u; }
};

template<> struct CdrTopicInfo<msg::AckermannDrive> {
  static const char* type_name() {
    return "ackermann_msgs::msg::dds_::AckermannDrive_";
  }
  static const char* type_hash() {
    return "RIHS01_acf287a224a947dd1b0b87d6d76cdb73f497b0237b8fc73be2173b2ebbb82c99";
  }
  static size_t max_serialized_size() { return 20u; }
};

template<> struct CdrTopicInfo<msg::AckermannDriveStamped> {
  static const char* type_name() {
    return "ackermann_msgs::msg::dds_::AckermannDriveStamped_";
  }
  static const char* type_hash() {
    return "RIHS01_48ca7612a08d3bb72744fd98b71b7cf2ea24c6ad50fa4e1aa0bbad963c90d8cf";
  }
  static size_t max_serialized_size() { return 288u; }
};

template<> struct CdrTopicInfo<msg::CameraInfo> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::CameraInfo_";
  }
  static const char* type_hash() {
    return "RIHS01_b3dfd68ff46c9d56c80fd3bd4ed22c7a4ddce8c8348f2f59c299e73118e7e275";
  }
  static size_t max_serialized_size() { return 3793u; }
};

template<> struct CdrTopicInfo<msg::CarlaBoundingBox> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaBoundingBox_";
  }
  static const char* type_hash() {
    return "RIHS01_64150a58baddf4ecdfd2adcc41b74bc2a7caaa1677545e971b47623ca1c31b63";
  }
  // Two Vector3 (6 × float64).
  static size_t max_serialized_size() { return 48u; }
};

template<> struct CdrTopicInfo<msg::CarlaCollisionEvent> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaCollisionEvent_";
  }
  static const char* type_hash() {
    return "RIHS01_d77acb472c5effb98998bfb2e176ae3ffac0a84ce33a79f90e999544e3fcfeff";
  }
  static size_t max_serialized_size() { return 296u; }
};

template<> struct CdrTopicInfo<msg::CarlaEgoVehicleControl> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaEgoVehicleControl_";
  }
  static const char* type_hash() {
    return "RIHS01_4f251fa2a554e8ed996f77eb1d5b65515af1369eceb04d5122cb5761f7801be3";
  }
  static size_t max_serialized_size() { return 289u; }
};

template<> struct CdrTopicInfo<msg::CarlaEgoVehicleInfo> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaEgoVehicleInfo_";
  }
  static const char* type_hash() {
    return "RIHS01_b8dc3866014924ce7fe57e0cff05a5448ec18f27156656b3b1b69f4da558e956";
  }
  // Sized for two 256-byte strings and up to 8 wheels.
  static size_t max_serialized_size() { return 976u; }
};

template<> struct CdrTopicInfo<msg::CarlaEgoVehicleInfoWheel> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaEgoVehicleInfoWheel_";
  }
  static const char* type_hash() {
    return "RIHS01_63caae9f2637752b93a616b10e00d21be15ca32cac533efd3596349a5cf4d4fe";
  }
  // Six float32 plus one Vector3 (3 × float64).
  static size_t max_serialized_size() { return 48u; }
};

template<> struct CdrTopicInfo<msg::CarlaEgoVehicleStatus> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaEgoVehicleStatus_";
  }
  static const char* type_hash() {
    return "RIHS01_3565cc74470f5c37eb316a36426effd811ac1baf835df3fc2bb7a88574bb3e07";
  }
  static size_t max_serialized_size() { return 641u; }
};

template<> struct CdrTopicInfo<msg::CarlaLineInvasion> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::LaneInvasionEvent_";
  }
  static const char* type_hash() {
    return "RIHS01_1d81c780738761101e1b4cb165af96ed32b8aa5cd523713c9b28d1ee95af719b";
  }
  static size_t max_serialized_size() { return 672u; }
};

template<> struct CdrTopicInfo<msg::CarlaTrafficLightInfo> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaTrafficLightInfo_";
  }
  static const char* type_hash() {
    return "RIHS01_c6bc358a736c0843889a6f65d8070f9891619a250fcc58ad51ffb9f87dd138b4";
  }
  // uint32 + 4 padding + Pose (7 × float64) + CarlaBoundingBox (6 × float64).
  static size_t max_serialized_size() { return 112u; }
};

template<> struct CdrTopicInfo<msg::CarlaTrafficLightInfoList> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaTrafficLightInfoList_";
  }
  static const char* type_hash() {
    return "RIHS01_9a13ecb7abe8b5b53c9dc5b7794981c90bd04b0556f8ab66cf26863ac600fde6";
  }
  // Sized for a 4-byte length plus 64 aligned 112-byte entries.
  static size_t max_serialized_size() { return 7176u; }
};

template<> struct CdrTopicInfo<msg::CarlaTrafficLightStatus> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaTrafficLightStatus_";
  }
  static const char* type_hash() {
    return "RIHS01_e4a58f6956c9d395f8aa8df33b6aa2765cb83c8522ec3e676196dc49485bf0b8";
  }
  // One uint32 plus one uint8.
  static size_t max_serialized_size() { return 5u; }
};

template<> struct CdrTopicInfo<msg::CarlaTrafficLightStatusList> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::CarlaTrafficLightStatusList_";
  }
  static const char* type_hash() {
    return "RIHS01_07f3b9ec436d9925e8b55f229d420336a997e824911afdb299f2dd0f32a26268";
  }
  // Sized for a 4-byte length plus 64 aligned 8-byte entries.
  static size_t max_serialized_size() { return 516u; }
};

template<> struct CdrTopicInfo<msg::Clock> {
  static const char* type_name() {
    return "rosgraph_msgs::msg::dds_::Clock_";
  }
  static const char* type_hash() {
    return "RIHS01_692f7a66e93a3c83e71765d033b60349ba68023a8c689a79e48078bcb5c58564";
  }
  // Clock holds one Time (8 bytes = 2 × int32).
  static size_t max_serialized_size() { return 8u; }
};

template<> struct CdrTopicInfo<msg::Float32> {
  static const char* type_name() {
    return "std_msgs::msg::dds_::Float32_";
  }
  static const char* type_hash() {
    return "RIHS01_7170d3d8f841f7be3172ce5f4f59f3a4d7f63b0447e8b33327601ad64d83d6e2";
  }
  static size_t max_serialized_size() { return 4u; }
};

template<> struct CdrTopicInfo<msg::Header> {
  static const char* type_name() {
    return "std_msgs::msg::dds_::Header_";
  }
  static const char* type_hash() {
    return "RIHS01_f49fb3ae2cf070f793645ff749683ac6b06203e41c891e17701b1cb597ce6a01";
  }
  static size_t max_serialized_size() { return 268u; }
};

template<> struct CdrTopicInfo<msg::Image> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::Image_";
  }
  static const char* type_hash() {
    return "RIHS01_d31d41a9a4c4bc8eae9be757b0beed306564f7526c88ea6a4588fb9582527d47";
  }
  static size_t max_serialized_size() { return 648u; }
};

template<> struct CdrTopicInfo<msg::Imu> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::Imu_";
  }
  static const char* type_hash() {
    return "RIHS01_7d9a00ff131080897a5ec7e26e315954b8eae3353c3f995c55faf71574000b5b";
  }
  static size_t max_serialized_size() { return 568u; }
};

template<> struct CdrTopicInfo<msg::NavSatFix> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::NavSatFix_";
  }
  static const char* type_hash() {
    return "RIHS01_62223ab3fe210a15976021da7afddc9e200dc9ec75231c1b6a557fc598a65404";
  }
  static size_t max_serialized_size() { return 369u; }
};

template<> struct CdrTopicInfo<msg::NavSatStatus> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::NavSatStatus_";
  }
  static const char* type_hash() {
    return "RIHS01_d1ed3befa628e09571bd273b888ba1c1fd187c9a5e0006b385d7e5e9095a3204";
  }
  static size_t max_serialized_size() { return 4u; }
};

template<> struct CdrTopicInfo<msg::Odometry> {
  static const char* type_name() {
    return "nav_msgs::msg::dds_::Odometry_";
  }
  static const char* type_hash() {
    return "RIHS01_3cc97dc7fb7502f8714462c526d369e35b603cfc34d946e3f2eda2766dfec6e0";
  }
  static size_t max_serialized_size() { return 1208u; }
};

template<> struct CdrTopicInfo<msg::Point> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Point_";
  }
  static const char* type_hash() {
    return "RIHS01_6963084842a9b04494d6b2941d11444708d892da2f4b09843b9c43f42a7f6881";
  }
  static size_t max_serialized_size() { return 24u; }
};

template<> struct CdrTopicInfo<msg::Point32> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Point32_";
  }
  static const char* type_hash() {
    return "RIHS01_2fc4db7cae16a4582c79a56b66173a8d48d52c7dc520ddc55a0d4bcf2a4bfdbc";
  }
  static size_t max_serialized_size() { return 12u; }
};

template<> struct CdrTopicInfo<msg::PointCloud2> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::PointCloud2_";
  }
  static const char* type_hash() {
    return "RIHS01_9198cabf7da3796ae6fe19c4cb3bdd3525492988c70522628af5daa124bae2b5";
  }
  static size_t max_serialized_size() { return 27597u; }
};

template<> struct CdrTopicInfo<msg::PointField> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::PointField_";
  }
  static const char* type_hash() {
    return "RIHS01_5c6a4750728c2bcfbbf7037225b20b02d4429634732146b742dee1726637ef01";
  }
  static size_t max_serialized_size() { return 272u; }
};

template<> struct CdrTopicInfo<msg::Pose> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Pose_";
  }
  static const char* type_hash() {
    return "RIHS01_d501954e9476cea2996984e812054b68026ae0bfae789d9a10b23daf35cc90fa";
  }
  static size_t max_serialized_size() { return 56u; }
};

template<> struct CdrTopicInfo<msg::PoseWithCovariance> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::PoseWithCovariance_";
  }
  static const char* type_hash() {
    return "RIHS01_9a7c0fd234b7f45c6098745ecccd773ca1085670e64107135397aee31c02e1bb";
  }
  static size_t max_serialized_size() { return 344u; }
};

template<> struct CdrTopicInfo<msg::Quaternion> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Quaternion_";
  }
  static const char* type_hash() {
    return "RIHS01_8a765f66778c8ff7c8ab94afcc590a2ed5325a1d9a076ffff38fbce36f458684";
  }
  static size_t max_serialized_size() { return 32u; }
};

template<> struct CdrTopicInfo<msg::RegionOfInterest> {
  static const char* type_name() {
    return "sensor_msgs::msg::dds_::RegionOfInterest_";
  }
  static const char* type_hash() {
    return "RIHS01_ad16bcba5f9131dcdba6fbded19f726f5440e3c513b4fb586dd3027eeed8abb1";
  }
  static size_t max_serialized_size() { return 17u; }
};

template<> struct CdrTopicInfo<msg::String> {
  static const char* type_name() {
    return "std_msgs::msg::dds_::String_";
  }
  static const char* type_hash() {
    return "RIHS01_df668c740482bbd48fb39d76a70dfd4bd59db1288021743503259e948f6b1a18";
  }
  static size_t max_serialized_size() { return 260u; }
};

template<> struct CdrTopicInfo<msg::TF2Error> {
  static const char* type_name() {
    return "tf2_msgs::msg::dds_::TF2Error_";
  }
  static const char* type_hash() {
    return "RIHS01_db2485e7d6a0ec75bf087e058ee45df682acc3a621ca7780503d10aac141809a";
  }
  static size_t max_serialized_size() { return 264u; }
};

template<> struct CdrTopicInfo<msg::TFMessage> {
  static const char* type_name() {
    return "tf2_msgs::msg::dds_::TFMessage_";
  }
  static const char* type_hash() {
    return "RIHS01_e369d0f05a23ae52508854b66f6aa0437f3449d652e8cbf22d5abe85d020f087";
  }
  static size_t max_serialized_size() { return 58408u; }
};

template<> struct CdrTopicInfo<msg::Time> {
  static const char* type_name() {
    return "builtin_interfaces::msg::dds_::Time_";
  }
  static const char* type_hash() {
    return "RIHS01_b106235e25a4c5ed35098aa0a61a3ee9c9b18d197f398b0e4206cea9acf9c197";
  }
  static size_t max_serialized_size() { return 8u; }
};

template<> struct CdrTopicInfo<msg::Transform> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Transform_";
  }
  static const char* type_hash() {
    return "RIHS01_beb83fbe698636351461f6f35d1abb20010c43d55374d81bd041f1ba2581fddc";
  }
  static size_t max_serialized_size() { return 56u; }
};

template<> struct CdrTopicInfo<msg::TransformStamped> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::TransformStamped_";
  }
  static const char* type_hash() {
    return "RIHS01_0a241f87d04668d94099cbb5ba11691d5ad32c2f29682e4eb5653424bd275206";
  }
  static size_t max_serialized_size() { return 584u; }
};

template<> struct CdrTopicInfo<msg::Twist> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Twist_";
  }
  static const char* type_hash() {
    return "RIHS01_9c45bf16fe0983d80e3cfe750d6835843d265a9a6c46bd2e609fcddde6fb8d2a";
  }
  static size_t max_serialized_size() { return 48u; }
};

template<> struct CdrTopicInfo<msg::TwistWithCovariance> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::TwistWithCovariance_";
  }
  static const char* type_hash() {
    return "RIHS01_49f574f033f095d8b6cd1beaca5ca7925e296e84af1716d16c89d38b059c8c18";
  }
  static size_t max_serialized_size() { return 336u; }
};

template<> struct CdrTopicInfo<msg::Vector3> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::Vector3_";
  }
  static const char* type_hash() {
    return "RIHS01_cc12fe83e4c02719f1ce8070bfd14aecd40f75a96696a67a2a1f37f7dbb0765d";
  }
  static size_t max_serialized_size() { return 24u; }
};

} // namespace ros2
} // namespace carla
