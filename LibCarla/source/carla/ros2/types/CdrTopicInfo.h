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
#include "carla/ros2/types/msg/Control.h"
#include "carla/ros2/types/msg/ControlModeReport.h"
#include "carla/ros2/types/msg/Engage.h"
#include "carla/ros2/types/msg/Float32.h"
#include "carla/ros2/types/msg/GearCommand.h"
#include "carla/ros2/types/msg/GearReport.h"
#include "carla/ros2/types/msg/HazardLightsCommand.h"
#include "carla/ros2/types/msg/HazardLightsReport.h"
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/Image.h"
#include "carla/ros2/types/msg/Imu.h"
#include "carla/ros2/types/msg/Lateral.h"
#include "carla/ros2/types/msg/Longitudinal.h"
#include "carla/ros2/types/msg/NavSatFix.h"
#include "carla/ros2/types/msg/NavSatStatus.h"
#include "carla/ros2/types/msg/Odometry.h"
#include "carla/ros2/types/msg/Point.h"
#include "carla/ros2/types/msg/Point32.h"
#include "carla/ros2/types/msg/PointCloud2.h"
#include "carla/ros2/types/msg/PointField.h"
#include "carla/ros2/types/msg/Pose.h"
#include "carla/ros2/types/msg/PoseStamped.h"
#include "carla/ros2/types/msg/PoseWithCovariance.h"
#include "carla/ros2/types/msg/PoseWithCovarianceStamped.h"
#include "carla/ros2/types/msg/Quaternion.h"
#include "carla/ros2/types/msg/RegionOfInterest.h"
#include "carla/ros2/types/msg/SteeringReport.h"
#include "carla/ros2/types/msg/String.h"
#include "carla/ros2/types/msg/TF2Error.h"
#include "carla/ros2/types/msg/TFMessage.h"
#include "carla/ros2/types/msg/Time.h"
#include "carla/ros2/types/msg/Transform.h"
#include "carla/ros2/types/msg/TransformStamped.h"
#include "carla/ros2/types/msg/TurnIndicatorsCommand.h"
#include "carla/ros2/types/msg/TurnIndicatorsReport.h"
#include "carla/ros2/types/msg/Twist.h"
#include "carla/ros2/types/msg/TwistWithCovariance.h"
#include "carla/ros2/types/msg/Vector3.h"
#include "carla/ros2/types/msg/VehicleEmergencyStamped.h"
#include "carla/ros2/types/msg/VelocityReport.h"

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

template<> struct CdrTopicInfo<msg::CarlaLineInvasion> {
  static const char* type_name() {
    return "carla_msgs::msg::dds_::LaneInvasionEvent_";
  }
  static const char* type_hash() {
    return "RIHS01_1d81c780738761101e1b4cb165af96ed32b8aa5cd523713c9b28d1ee95af719b";
  }
  static size_t max_serialized_size() { return 672u; }
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

template<> struct CdrTopicInfo<msg::Control> {
  static const char* type_name() {
    return "autoware_control_msgs::msg::dds_::Control_";
  }
  static const char* type_hash() {
    return "RIHS01_7818be59aa790ebb777db06e55a2c15e3756de4cc35c80b1e8271afc5bab2e9d";
  }
  static size_t max_serialized_size() { return 74u; }
};

template<> struct CdrTopicInfo<msg::ControlModeReport> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::ControlModeReport_";
  }
  static const char* type_hash() {
    return "RIHS01_968feaa6441be3c3b161f2eb65972a4b15394d0a7ddc4664318551280d1ff222";
  }
  static size_t max_serialized_size() { return 9u; }
};

template<> struct CdrTopicInfo<msg::Engage> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::Engage_";
  }
  static const char* type_hash() {
    return "RIHS01_a865a0a669052a5bb45be203266058660ae8b3be8d080c4567f78d8174f4849e";
  }
  static size_t max_serialized_size() { return 9u; }
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

template<> struct CdrTopicInfo<msg::GearCommand> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::GearCommand_";
  }
  static const char* type_hash() {
    return "RIHS01_4aa982c6505fafa9bdca2d243d0a71e46ff9a1e7013767f13a533d0a5d30a1a6";
  }
  static size_t max_serialized_size() { return 9u; }
};

template<> struct CdrTopicInfo<msg::GearReport> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::GearReport_";
  }
  static const char* type_hash() {
    return "RIHS01_4d14bc3f186c1a6af6a732bb5ebd540cdd742a56770012f4c3cb9e762de8f391";
  }
  static size_t max_serialized_size() { return 9u; }
};

template<> struct CdrTopicInfo<msg::HazardLightsCommand> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::HazardLightsCommand_";
  }
  static const char* type_hash() {
    return "RIHS01_ad2386a78a210625f1fb35f8043ef4e78da58cf01f33c340fa9468e5e8cdd0bc";
  }
  static size_t max_serialized_size() { return 9u; }
};

template<> struct CdrTopicInfo<msg::HazardLightsReport> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::HazardLightsReport_";
  }
  static const char* type_hash() {
    return "RIHS01_01ce3b4293a5c2799fd7483b2d62a790e26fe8f2b5d60e48149163475685f28a";
  }
  static size_t max_serialized_size() { return 9u; }
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

template<> struct CdrTopicInfo<msg::Lateral> {
  static const char* type_name() {
    return "autoware_control_msgs::msg::dds_::Lateral_";
  }
  static const char* type_hash() {
    return "RIHS01_94d39634041a92db4e460dae93def564fbeb89142e9c64f9c2ed540a182935a1";
  }
  static size_t max_serialized_size() { return 25u; }
};

template<> struct CdrTopicInfo<msg::Longitudinal> {
  static const char* type_name() {
    return "autoware_control_msgs::msg::dds_::Longitudinal_";
  }
  static const char* type_hash() {
    return "RIHS01_4ac4a6b5085ee6bb12067b763568abbf68ec3f2baf64b49e1da360aa6413edc1";
  }
  static size_t max_serialized_size() { return 30u; }
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

template<> struct CdrTopicInfo<msg::PoseStamped> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::PoseStamped_";
  }
  static const char* type_hash() {
    return "RIHS01_10f3786d7d40fd2b54367835614bff85d4ad3b5dab62bf8bca0cc232d73b4cd8";
  }
  // Header(268) + Pose(56).
  static size_t max_serialized_size() { return 324u; }
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

template<> struct CdrTopicInfo<msg::PoseWithCovarianceStamped> {
  static const char* type_name() {
    return "geometry_msgs::msg::dds_::PoseWithCovarianceStamped_";
  }
  static const char* type_hash() {
    return "RIHS01_26432f9803e43727d3c8f668d1fdb3c630f548af631e2f4e31382371bfea3b6e";
  }
  // Header(268) + PoseWithCovariance(344).
  static size_t max_serialized_size() { return 612u; }
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

template<> struct CdrTopicInfo<msg::SteeringReport> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::SteeringReport_";
  }
  static const char* type_hash() {
    return "RIHS01_aa3acc9ca95ebc4daf9dec0ecf87911ad9c196392857c3026bfead589db65a94";
  }
  static size_t max_serialized_size() { return 12u; }
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

template<> struct CdrTopicInfo<msg::TurnIndicatorsCommand> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::TurnIndicatorsCommand_";
  }
  static const char* type_hash() {
    return "RIHS01_657d7f761a9b02c543e4bf3e0000405906b6169dc0bbfd6637c64fe00d98cec7";
  }
  static size_t max_serialized_size() { return 9u; }
};

template<> struct CdrTopicInfo<msg::TurnIndicatorsReport> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::TurnIndicatorsReport_";
  }
  static const char* type_hash() {
    return "RIHS01_c05a54cd244f1c9d683613b11c87a5b3ef816eed7a5f207368301221731a0964";
  }
  static size_t max_serialized_size() { return 9u; }
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

template<> struct CdrTopicInfo<msg::VehicleEmergencyStamped> {
  static const char* type_name() {
    return "tier4_vehicle_msgs::msg::dds_::VehicleEmergencyStamped_";
  }
  static const char* type_hash() {
    return "RIHS01_017801df5493a599ad735d55f3e93ea132c46f8f630bce94a5f3eb2dc4a4bb3c";
  }
  static size_t max_serialized_size() { return 9u; }
};

template<> struct CdrTopicInfo<msg::VelocityReport> {
  static const char* type_name() {
    return "autoware_vehicle_msgs::msg::dds_::VelocityReport_";
  }
  static const char* type_hash() {
    return "RIHS01_9052adda949c32f4a98500abc1fb5bd23f2560e321eebdfbb25318d6108d4ce4";
  }
  // Header(268) + 3 x float32.
  static size_t max_serialized_size() { return 280u; }
};

} // namespace ros2
} // namespace carla
