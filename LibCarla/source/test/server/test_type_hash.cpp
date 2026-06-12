// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Tests for REP-2011 type hash values embedded in CdrTopicInfo<T>::type_hash().
// Verifies format RIHS01_[0-9a-f]{64} for all 40 specializations, uniqueness
// across the set, and equality with the golden hashes the canonical message
// packages compute, so a field-layout drift from the canonical definition
// fails here instead of silently breaking decode on a real subscriber.

#define CARLA_ROS2_MIDDLEWARE_TESTING
#include "test.h"

#include <carla/ros2/types/CdrTopicInfo.h>
#include <carla/ros2/types/UserDataFormat.h>

#include <cstring>
#include <regex>
#include <set>
#include <string>
#include <vector>

using namespace carla::ros2;

// ---------------------------------------------------------------------------
// Format tests
// ---------------------------------------------------------------------------

static void check_hash(const char* hash, const std::string& type_label) {
  ASSERT_NE(hash, nullptr) << "type_hash() is nullptr for " << type_label;
  const std::string s(hash);
  ASSERT_EQ(s.size(), 71u) << "type_hash length != 71 for " << type_label
                              << " (got " << s.size() << "): " << s;
  const std::regex pattern("^RIHS01_[0-9a-f]{64}$");
  EXPECT_TRUE(std::regex_match(s, pattern))
      << "type_hash format mismatch for " << type_label << ": " << s;
}

#define CHECK_HASH(MsgType) \
  check_hash(CdrTopicInfo<msg::MsgType>::type_hash(), #MsgType)

TEST(TypeHash, FormatAllTypes) {
  CHECK_HASH(Accel);
  CHECK_HASH(AckermannDrive);
  CHECK_HASH(AckermannDriveStamped);
  CHECK_HASH(CameraInfo);
  CHECK_HASH(CarlaBoundingBox);
  CHECK_HASH(CarlaCollisionEvent);
  CHECK_HASH(CarlaEgoVehicleControl);
  CHECK_HASH(CarlaEgoVehicleInfo);
  CHECK_HASH(CarlaEgoVehicleInfoWheel);
  CHECK_HASH(CarlaEgoVehicleStatus);
  CHECK_HASH(CarlaLineInvasion);
  CHECK_HASH(CarlaTrafficLightInfo);
  CHECK_HASH(CarlaTrafficLightInfoList);
  CHECK_HASH(CarlaTrafficLightStatus);
  CHECK_HASH(CarlaTrafficLightStatusList);
  CHECK_HASH(Clock);
  CHECK_HASH(Float32);
  CHECK_HASH(Header);
  CHECK_HASH(Image);
  CHECK_HASH(Imu);
  CHECK_HASH(NavSatFix);
  CHECK_HASH(NavSatStatus);
  CHECK_HASH(Odometry);
  CHECK_HASH(Point);
  CHECK_HASH(Point32);
  CHECK_HASH(PointCloud2);
  CHECK_HASH(PointField);
  CHECK_HASH(Pose);
  CHECK_HASH(PoseWithCovariance);
  CHECK_HASH(Quaternion);
  CHECK_HASH(RegionOfInterest);
  CHECK_HASH(String);
  CHECK_HASH(TF2Error);
  CHECK_HASH(TFMessage);
  CHECK_HASH(Time);
  CHECK_HASH(Transform);
  CHECK_HASH(TransformStamped);
  CHECK_HASH(Twist);
  CHECK_HASH(TwistWithCovariance);
  CHECK_HASH(Vector3);
}

TEST(TypeHash, UniqueAcrossAllTypes) {
  std::vector<std::string> hashes = {
    CdrTopicInfo<msg::Accel>::type_hash(),
    CdrTopicInfo<msg::AckermannDrive>::type_hash(),
    CdrTopicInfo<msg::AckermannDriveStamped>::type_hash(),
    CdrTopicInfo<msg::CameraInfo>::type_hash(),
    CdrTopicInfo<msg::CarlaBoundingBox>::type_hash(),
    CdrTopicInfo<msg::CarlaCollisionEvent>::type_hash(),
    CdrTopicInfo<msg::CarlaEgoVehicleControl>::type_hash(),
    CdrTopicInfo<msg::CarlaEgoVehicleInfo>::type_hash(),
    CdrTopicInfo<msg::CarlaEgoVehicleInfoWheel>::type_hash(),
    CdrTopicInfo<msg::CarlaEgoVehicleStatus>::type_hash(),
    CdrTopicInfo<msg::CarlaLineInvasion>::type_hash(),
    CdrTopicInfo<msg::CarlaTrafficLightInfo>::type_hash(),
    CdrTopicInfo<msg::CarlaTrafficLightInfoList>::type_hash(),
    CdrTopicInfo<msg::CarlaTrafficLightStatus>::type_hash(),
    CdrTopicInfo<msg::CarlaTrafficLightStatusList>::type_hash(),
    CdrTopicInfo<msg::Clock>::type_hash(),
    CdrTopicInfo<msg::Float32>::type_hash(),
    CdrTopicInfo<msg::Header>::type_hash(),
    CdrTopicInfo<msg::Image>::type_hash(),
    CdrTopicInfo<msg::Imu>::type_hash(),
    CdrTopicInfo<msg::NavSatFix>::type_hash(),
    CdrTopicInfo<msg::NavSatStatus>::type_hash(),
    CdrTopicInfo<msg::Odometry>::type_hash(),
    CdrTopicInfo<msg::Point>::type_hash(),
    CdrTopicInfo<msg::Point32>::type_hash(),
    CdrTopicInfo<msg::PointCloud2>::type_hash(),
    CdrTopicInfo<msg::PointField>::type_hash(),
    CdrTopicInfo<msg::Pose>::type_hash(),
    CdrTopicInfo<msg::PoseWithCovariance>::type_hash(),
    CdrTopicInfo<msg::Quaternion>::type_hash(),
    CdrTopicInfo<msg::RegionOfInterest>::type_hash(),
    CdrTopicInfo<msg::String>::type_hash(),
    CdrTopicInfo<msg::TF2Error>::type_hash(),
    CdrTopicInfo<msg::TFMessage>::type_hash(),
    CdrTopicInfo<msg::Time>::type_hash(),
    CdrTopicInfo<msg::Transform>::type_hash(),
    CdrTopicInfo<msg::TransformStamped>::type_hash(),
    CdrTopicInfo<msg::Twist>::type_hash(),
    CdrTopicInfo<msg::TwistWithCovariance>::type_hash(),
    CdrTopicInfo<msg::Vector3>::type_hash(),
  };
  std::set<std::string> unique_set(hashes.begin(), hashes.end());
  EXPECT_EQ(unique_set.size(), hashes.size())
      << "Duplicate type hash detected across CdrTopicInfo specializations";
}

// ---------------------------------------------------------------------------
// Golden hash tests
// ---------------------------------------------------------------------------

// Golden RIHS01 hashes computed by the canonical message packages themselves,
// not copied from CdrTopicInfo.h. Provenance (extracted 2026-07-06):
//   - Standard types: REP-2011 JSON metadata installed by each package in
//     osrf/ros:jazzy-desktop (/opt/ros/jazzy/share/<pkg>/msg/<Type>.json);
//     ackermann_msgs from the ros-jazzy-ackermann-msgs package.
//   - carla_msgs types: rosidl output of building ros-carla-msgs master in
//     the same image (the Util/ros2/compute_type_hash.sh flow), including a
//     LaneInvasionEvent.msg alias of CarlaLaneInvasionEvent.msg because CARLA
//     registers that type name on the wire.
// The RIHS01 hash is a function of the message definition only, so it is
// identical on every distro that implements REP-2011 (Iron and newer).
// Humble predates REP-2011: it ships no hashes and never validates them,
// so the Jazzy values below are the golden set.
#define CHECK_GOLDEN_HASH(MsgType, golden)                       \
  EXPECT_STREQ(CdrTopicInfo<msg::MsgType>::type_hash(), golden)  \
      << "type hash drift from the canonical " #MsgType          \
         " definition; regenerate with Util/ros2/compute_type_hash.sh"

TEST(TypeHash, GoldenValuesMatchCanonicalPackages) {
  CHECK_GOLDEN_HASH(Accel, "RIHS01_dc448243ded9b1fcbcca24aba0c22f013dae06c354ba2d849571c0a2a3f57ca0");
  CHECK_GOLDEN_HASH(AckermannDrive, "RIHS01_acf287a224a947dd1b0b87d6d76cdb73f497b0237b8fc73be2173b2ebbb82c99");
  CHECK_GOLDEN_HASH(AckermannDriveStamped, "RIHS01_48ca7612a08d3bb72744fd98b71b7cf2ea24c6ad50fa4e1aa0bbad963c90d8cf");
  CHECK_GOLDEN_HASH(CameraInfo, "RIHS01_b3dfd68ff46c9d56c80fd3bd4ed22c7a4ddce8c8348f2f59c299e73118e7e275");
  CHECK_GOLDEN_HASH(CarlaCollisionEvent, "RIHS01_d77acb472c5effb98998bfb2e176ae3ffac0a84ce33a79f90e999544e3fcfeff");
  CHECK_GOLDEN_HASH(CarlaEgoVehicleControl, "RIHS01_4f251fa2a554e8ed996f77eb1d5b65515af1369eceb04d5122cb5761f7801be3");
  CHECK_GOLDEN_HASH(CarlaEgoVehicleInfo, "RIHS01_b8dc3866014924ce7fe57e0cff05a5448ec18f27156656b3b1b69f4da558e956");
  CHECK_GOLDEN_HASH(
      CarlaEgoVehicleInfoWheel, "RIHS01_63caae9f2637752b93a616b10e00d21be15ca32cac533efd3596349a5cf4d4fe");
  CHECK_GOLDEN_HASH(CarlaEgoVehicleStatus, "RIHS01_3565cc74470f5c37eb316a36426effd811ac1baf835df3fc2bb7a88574bb3e07");
  CHECK_GOLDEN_HASH(CarlaLineInvasion, "RIHS01_1d81c780738761101e1b4cb165af96ed32b8aa5cd523713c9b28d1ee95af719b");
  CHECK_GOLDEN_HASH(Clock, "RIHS01_692f7a66e93a3c83e71765d033b60349ba68023a8c689a79e48078bcb5c58564");
  CHECK_GOLDEN_HASH(Float32, "RIHS01_7170d3d8f841f7be3172ce5f4f59f3a4d7f63b0447e8b33327601ad64d83d6e2");
  CHECK_GOLDEN_HASH(Header, "RIHS01_f49fb3ae2cf070f793645ff749683ac6b06203e41c891e17701b1cb597ce6a01");
  CHECK_GOLDEN_HASH(Image, "RIHS01_d31d41a9a4c4bc8eae9be757b0beed306564f7526c88ea6a4588fb9582527d47");
  CHECK_GOLDEN_HASH(Imu, "RIHS01_7d9a00ff131080897a5ec7e26e315954b8eae3353c3f995c55faf71574000b5b");
  CHECK_GOLDEN_HASH(NavSatFix, "RIHS01_62223ab3fe210a15976021da7afddc9e200dc9ec75231c1b6a557fc598a65404");
  CHECK_GOLDEN_HASH(NavSatStatus, "RIHS01_d1ed3befa628e09571bd273b888ba1c1fd187c9a5e0006b385d7e5e9095a3204");
  CHECK_GOLDEN_HASH(Odometry, "RIHS01_3cc97dc7fb7502f8714462c526d369e35b603cfc34d946e3f2eda2766dfec6e0");
  CHECK_GOLDEN_HASH(Point, "RIHS01_6963084842a9b04494d6b2941d11444708d892da2f4b09843b9c43f42a7f6881");
  CHECK_GOLDEN_HASH(Point32, "RIHS01_2fc4db7cae16a4582c79a56b66173a8d48d52c7dc520ddc55a0d4bcf2a4bfdbc");
  CHECK_GOLDEN_HASH(PointCloud2, "RIHS01_9198cabf7da3796ae6fe19c4cb3bdd3525492988c70522628af5daa124bae2b5");
  CHECK_GOLDEN_HASH(PointField, "RIHS01_5c6a4750728c2bcfbbf7037225b20b02d4429634732146b742dee1726637ef01");
  CHECK_GOLDEN_HASH(Pose, "RIHS01_d501954e9476cea2996984e812054b68026ae0bfae789d9a10b23daf35cc90fa");
  CHECK_GOLDEN_HASH(PoseWithCovariance, "RIHS01_9a7c0fd234b7f45c6098745ecccd773ca1085670e64107135397aee31c02e1bb");
  CHECK_GOLDEN_HASH(Quaternion, "RIHS01_8a765f66778c8ff7c8ab94afcc590a2ed5325a1d9a076ffff38fbce36f458684");
  CHECK_GOLDEN_HASH(RegionOfInterest, "RIHS01_ad16bcba5f9131dcdba6fbded19f726f5440e3c513b4fb586dd3027eeed8abb1");
  CHECK_GOLDEN_HASH(String, "RIHS01_df668c740482bbd48fb39d76a70dfd4bd59db1288021743503259e948f6b1a18");
  CHECK_GOLDEN_HASH(TF2Error, "RIHS01_db2485e7d6a0ec75bf087e058ee45df682acc3a621ca7780503d10aac141809a");
  CHECK_GOLDEN_HASH(TFMessage, "RIHS01_e369d0f05a23ae52508854b66f6aa0437f3449d652e8cbf22d5abe85d020f087");
  CHECK_GOLDEN_HASH(Time, "RIHS01_b106235e25a4c5ed35098aa0a61a3ee9c9b18d197f398b0e4206cea9acf9c197");
  CHECK_GOLDEN_HASH(Transform, "RIHS01_beb83fbe698636351461f6f35d1abb20010c43d55374d81bd041f1ba2581fddc");
  CHECK_GOLDEN_HASH(TransformStamped, "RIHS01_0a241f87d04668d94099cbb5ba11691d5ad32c2f29682e4eb5653424bd275206");
  CHECK_GOLDEN_HASH(Twist, "RIHS01_9c45bf16fe0983d80e3cfe750d6835843d265a9a6c46bd2e609fcddde6fb8d2a");
  CHECK_GOLDEN_HASH(TwistWithCovariance, "RIHS01_49f574f033f095d8b6cd1beaca5ca7925e296e84af1716d16c89d38b059c8c18");
  CHECK_GOLDEN_HASH(Vector3, "RIHS01_cc12fe83e4c02719f1ce8070bfd14aecd40f75a96696a67a2a1f37f7dbb0765d");
}

// ---------------------------------------------------------------------------
// UserDataFormat tests
// ---------------------------------------------------------------------------

TEST(UserDataFormat, BuildPayloadFormat) {
  const char* hash = "RIHS01_acf287a224a947dd1b0b87d6d76cdb73f497b0237b8fc73be2173b2ebbb82c99";
  auto ud = build_user_data(hash);
  const std::string payload(ud.begin(), ud.end());
  // REP-2016 KV format: "typehash=<hash>;"
  const std::string expected = std::string("typehash=") + hash + ";";
  EXPECT_EQ(payload, expected);
}

TEST(UserDataFormat, NullHashReturnsEmpty) {
  auto ud = build_user_data(nullptr);
  EXPECT_TRUE(ud.empty());
}

TEST(UserDataFormat, BuildForTemplateImu) {
  auto ud = build_user_data_for<msg::Imu>();
  EXPECT_FALSE(ud.empty());
  const std::string payload(ud.begin(), ud.end());
  EXPECT_EQ(payload.substr(0, 9), "typehash=");
  EXPECT_EQ(payload.back(), ';');
  // Confirm the embedded hash matches type_hash()
  const std::string expected_hash = CdrTopicInfo<msg::Imu>::type_hash();
  EXPECT_NE(payload.find(expected_hash), std::string::npos);
}

TEST(UserDataFormat, PayloadHasNoNullTerminator) {
  auto ud = build_user_data_for<msg::Clock>();
  // The last byte should be ';', not '\0'
  EXPECT_FALSE(ud.empty());
  EXPECT_EQ(static_cast<char>(ud.back()), ';');
}
