// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Tests for REP-2011 type hash values embedded in CdrTopicInfo<T>::type_hash().
// Verifies format RIHS01_[0-9a-f]{64} for all 31 specializations and
// uniqueness across the set.

#define CARLA_ROS2_DDS_TESTING
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
  CHECK_HASH(AckermannDrive);
  CHECK_HASH(AckermannDriveStamped);
  CHECK_HASH(CameraInfo);
  CHECK_HASH(CarlaCollisionEvent);
  CHECK_HASH(CarlaEgoVehicleControl);
  CHECK_HASH(CarlaLineInvasion);
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
    CdrTopicInfo<msg::AckermannDrive>::type_hash(),
    CdrTopicInfo<msg::AckermannDriveStamped>::type_hash(),
    CdrTopicInfo<msg::CameraInfo>::type_hash(),
    CdrTopicInfo<msg::CarlaCollisionEvent>::type_hash(),
    CdrTopicInfo<msg::CarlaEgoVehicleControl>::type_hash(),
    CdrTopicInfo<msg::CarlaLineInvasion>::type_hash(),
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
