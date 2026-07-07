// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Must be defined before any includes to suppress real DDS auto-includes
// when the middleware abstraction test groups join this suite.
#define CARLA_ROS2_MIDDLEWARE_TESTING

#include "test.h"

#include <carla/ros2/types/CdrSerialization.h>
#include <carla/ros2/types/CdrTopicInfo.h>

#include <cstdint>
#include <vector>

// ==========================================================================
// Group 1: cdr_topic_info (2 tests)
// ==========================================================================

TEST(cdr_topic_info, type_names_are_non_empty) {
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::Time>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::Header>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::Image>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::PointCloud2>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::TFMessage>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::Imu>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::NavSatFix>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::Odometry>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::CameraInfo>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::CarlaCollisionEvent>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::CarlaEgoVehicleControl>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::CarlaLineInvasion>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::AckermannDriveStamped>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::TransformStamped>::type_name());
  EXPECT_STRNE("", carla::ros2::CdrTopicInfo<carla::ros2::msg::Clock>::type_name());
}

TEST(cdr_topic_info, max_sizes_are_positive) {
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::Time>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::Header>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::Image>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::PointCloud2>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::TFMessage>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::Clock>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::Imu>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::NavSatFix>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::CameraInfo>::max_serialized_size(), 0u);
  EXPECT_GT(carla::ros2::CdrTopicInfo<carla::ros2::msg::AckermannDriveStamped>::max_serialized_size(), 0u);
}

// ==========================================================================
// Group 2: cdr_serialization (22 tests)
// ==========================================================================

TEST(cdr_serialization, time_round_trip) {
  carla::ros2::msg::Time original{};
  original.sec = 42;
  original.nanosec = 123456789u;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::Time recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.sec, 42);
  EXPECT_EQ(recovered.nanosec, 123456789u);
}

// Byte-exact wire-format guard. The symmetric round-trip tests above cannot
// detect an encoding drift (e.g. Fast-CDR defaulting to XCDRv2, which inserts
// DHEADERs) because deserialize_from_cdr would consume whatever encoding
// serialize_to_cdr emitted. This test pins the literal bytes so a regression
// to XCDRv2 or big-endian is caught deterministically. These exact bytes are
// what every backend puts on the wire: FastDDS via write_serialized_payload,
// CycloneDDS via dds_writecdr (raw passthrough of serialize_to_cdr), and Zenoh.
// Layout (classic CDR, encoding version 1, little-endian):
//   00 01 00 00  encapsulation header (PLAIN_CDR little-endian + 2 option bytes)
//   2A 00 00 00  sec    = int32  42        (0x0000002A, LE)
//   15 CD 5B 07  nanosec= uint32 123456789 (0x075BCD15, LE)
TEST(cdr_serialization, time_golden_xcdrv1_bytes) {
  carla::ros2::msg::Time original{};
  original.sec = 42;
  original.nanosec = 123456789u;

  const auto buf = carla::ros2::serialize_to_cdr(original);

  const std::vector<uint8_t> expected{
      0x00u, 0x01u, 0x00u, 0x00u,
      0x2Au, 0x00u, 0x00u, 0x00u,
      0x15u, 0xCDu, 0x5Bu, 0x07u};
  // Assert the size first: a failed or short serialization then yields a
  // targeted size mismatch instead of a noisy full-vector diff.
  ASSERT_EQ(buf.size(), expected.size());
  EXPECT_EQ(buf, expected);
  // Discriminator byte: classic CDR_LE is 0x01; any XCDRv2 representation
  // (PLAIN_CDR2 / DELIMIT_CDR2 / PL_CDR2) would change byte 1.
  EXPECT_EQ(buf[0], 0x00u);
  EXPECT_EQ(buf[1], 0x01u);
}

TEST(cdr_serialization, header_round_trip) {
  carla::ros2::msg::Header original{};
  original.stamp.sec = 10;
  original.stamp.nanosec = 500000000u;
  original.frame_id = "base_link";

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::Header recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.stamp.sec, 10);
  EXPECT_EQ(recovered.stamp.nanosec, 500000000u);
  EXPECT_EQ(recovered.frame_id, "base_link");
}

TEST(cdr_serialization, vector3_round_trip) {
  carla::ros2::msg::Vector3 original{};
  original.x = 1.5;
  original.y = -2.75;
  original.z = 3.0;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::Vector3 recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_DOUBLE_EQ(recovered.x, 1.5);
  EXPECT_DOUBLE_EQ(recovered.y, -2.75);
  EXPECT_DOUBLE_EQ(recovered.z, 3.0);
}

TEST(cdr_serialization, imu_round_trip) {
  carla::ros2::msg::Imu original{};
  original.header.stamp.sec = 5;
  original.header.frame_id = "imu_link";
  original.orientation.x = 0.1;
  original.orientation.y = 0.2;
  original.orientation.z = 0.3;
  original.orientation.w = 0.9;
  original.angular_velocity.x = 0.01;
  original.linear_acceleration.z = 9.81;
  original.orientation_covariance[0] = 1.0;
  original.orientation_covariance[4] = 1.0;
  original.orientation_covariance[8] = 1.0;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::Imu recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.header.stamp.sec, 5);
  EXPECT_EQ(recovered.header.frame_id, "imu_link");
  EXPECT_DOUBLE_EQ(recovered.orientation.x, 0.1);
  EXPECT_DOUBLE_EQ(recovered.orientation.w, 0.9);
  EXPECT_DOUBLE_EQ(recovered.linear_acceleration.z, 9.81);
  EXPECT_DOUBLE_EQ(recovered.orientation_covariance[0], 1.0);
  EXPECT_DOUBLE_EQ(recovered.orientation_covariance[4], 1.0);
}

TEST(cdr_serialization, image_round_trip) {
  carla::ros2::msg::Image original{};
  original.header.frame_id = "camera";
  original.height = 2u;
  original.width = 3u;
  original.encoding = "rgb8";
  original.is_bigendian = 0u;
  original.step = 9u;
  original.data = {1u, 2u, 3u, 4u, 5u, 6u};

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::Image recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.header.frame_id, "camera");
  EXPECT_EQ(recovered.height, 2u);
  EXPECT_EQ(recovered.width, 3u);
  EXPECT_EQ(recovered.encoding, "rgb8");
  ASSERT_EQ(recovered.data.size(), 6u);
  EXPECT_EQ(recovered.data[0], 1u);
  EXPECT_EQ(recovered.data[5], 6u);
}

TEST(cdr_serialization, pointcloud2_round_trip) {
  carla::ros2::msg::PointCloud2 original{};
  original.header.frame_id = "velodyne";
  original.height = 1u;
  original.width = 2u;

  carla::ros2::msg::PointField pf{};
  pf.name = "x";
  pf.offset = 0u;
  pf.datatype = static_cast<uint8_t>(carla::ros2::msg::PointField::FLOAT32);
  pf.count = 1u;
  original.fields.push_back(pf);

  original.is_bigendian = false;
  original.point_step = 4u;
  original.row_step = 8u;
  original.data = {0u, 0u, 128u, 63u,  // 1.0f LE
                   0u, 0u, 0u, 64u};    // 2.0f LE
  original.is_dense = true;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::PointCloud2 recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.header.frame_id, "velodyne");
  EXPECT_EQ(recovered.height, 1u);
  EXPECT_EQ(recovered.width, 2u);
  ASSERT_EQ(recovered.fields.size(), 1u);
  EXPECT_EQ(recovered.fields[0].name, "x");
  EXPECT_EQ(recovered.fields[0].datatype, static_cast<uint8_t>(carla::ros2::msg::PointField::FLOAT32));
  EXPECT_EQ(recovered.is_dense, true);
  ASSERT_EQ(recovered.data.size(), 8u);
}

TEST(cdr_serialization, tfmessage_round_trip) {
  carla::ros2::msg::TFMessage original{};

  carla::ros2::msg::TransformStamped ts{};
  ts.header.stamp.sec = 1;
  ts.header.frame_id = "world";
  ts.child_frame_id = "robot";
  ts.transform.translation.x = 1.0;
  ts.transform.translation.y = 2.0;
  ts.transform.translation.z = 0.5;
  ts.transform.rotation.w = 1.0;
  original.transforms.push_back(ts);

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::TFMessage recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  ASSERT_EQ(recovered.transforms.size(), 1u);
  EXPECT_EQ(recovered.transforms[0].header.stamp.sec, 1);
  EXPECT_EQ(recovered.transforms[0].header.frame_id, "world");
  EXPECT_EQ(recovered.transforms[0].child_frame_id, "robot");
  EXPECT_DOUBLE_EQ(recovered.transforms[0].transform.translation.x, 1.0);
  EXPECT_DOUBLE_EQ(recovered.transforms[0].transform.translation.y, 2.0);
  EXPECT_DOUBLE_EQ(recovered.transforms[0].transform.rotation.w, 1.0);
}

TEST(cdr_serialization, navsat_fix_round_trip) {
  carla::ros2::msg::NavSatFix original{};
  original.header.frame_id = "gps";
  original.status.status = static_cast<uint8_t>(carla::ros2::msg::NavSatStatus::STATUS_FIX);
  original.status.service = static_cast<uint16_t>(carla::ros2::msg::NavSatStatus::SERVICE_GPS);
  original.latitude = 48.8566;
  original.longitude = 2.3522;
  original.altitude = 35.0;
  original.position_covariance[0] = 0.01;
  original.position_covariance_type =
      static_cast<uint8_t>(carla::ros2::msg::NavSatFix::COVARIANCE_TYPE_DIAGONAL_KNOWN);

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::NavSatFix recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.header.frame_id, "gps");
  EXPECT_EQ(
      recovered.status.status,
      static_cast<uint8_t>(carla::ros2::msg::NavSatStatus::STATUS_FIX));
  EXPECT_DOUBLE_EQ(recovered.latitude, 48.8566);
  EXPECT_DOUBLE_EQ(recovered.longitude, 2.3522);
  EXPECT_DOUBLE_EQ(recovered.altitude, 35.0);
  EXPECT_DOUBLE_EQ(recovered.position_covariance[0], 0.01);
  EXPECT_EQ(
      recovered.position_covariance_type,
      static_cast<uint8_t>(carla::ros2::msg::NavSatFix::COVARIANCE_TYPE_DIAGONAL_KNOWN));
}

TEST(cdr_serialization, carla_ego_vehicle_control_round_trip) {
  carla::ros2::msg::CarlaEgoVehicleControl original{};
  original.throttle = 0.75f;
  original.steer = -0.5f;
  original.brake = 0.0f;
  original.hand_brake = false;
  original.reverse = true;
  original.gear = 2;
  original.manual_gear_shift = false;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::CarlaEgoVehicleControl recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_FLOAT_EQ(recovered.throttle, 0.75f);
  EXPECT_FLOAT_EQ(recovered.steer, -0.5f);
  EXPECT_FLOAT_EQ(recovered.brake, 0.0f);
  EXPECT_EQ(recovered.hand_brake, false);
  EXPECT_EQ(recovered.reverse, true);
  EXPECT_EQ(recovered.gear, 2);
  EXPECT_EQ(recovered.manual_gear_shift, false);
}

TEST(cdr_serialization, carla_line_invasion_round_trip) {
  carla::ros2::msg::CarlaLineInvasion original{};
  original.header.frame_id = "vehicle";
  original.crossed_lane_markings = {1, 4, 7};

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::CarlaLineInvasion recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.header.frame_id, "vehicle");
  ASSERT_EQ(recovered.crossed_lane_markings.size(), 3u);
  EXPECT_EQ(recovered.crossed_lane_markings[0], 1);
  EXPECT_EQ(recovered.crossed_lane_markings[1], 4);
  EXPECT_EQ(recovered.crossed_lane_markings[2], 7);
}

TEST(cdr_serialization, clock_round_trip) {
  carla::ros2::msg::Clock original{};
  original.clock.sec = 999;
  original.clock.nanosec = 1u;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::Clock recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_EQ(recovered.clock.sec, 999);
  EXPECT_EQ(recovered.clock.nanosec, 1u);
}

TEST(cdr_serialization, empty_tfmessage_round_trip) {
  carla::ros2::msg::TFMessage original{};

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::TFMessage recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  EXPECT_TRUE(recovered.transforms.empty());
}

TEST(cdr_serialization, pointcloud2_multi_field_round_trip) {
  // Exercises the manual sequence loop in serialize_cdr/deserialize_cdr for
  // PointCloud2::fields with more than one element. The single-field
  // round-trip above does not catch a bug in the loop step.
  carla::ros2::msg::PointCloud2 original{};
  original.header.frame_id = "lidar";
  original.height = 1u;
  original.width = 4u;

  carla::ros2::msg::PointField fx{};
  fx.name = "x";
  fx.offset = 0u;
  fx.datatype = carla::ros2::msg::PointField::FLOAT32;
  fx.count = 1u;

  carla::ros2::msg::PointField fy{};
  fy.name = "y";
  fy.offset = 4u;
  fy.datatype = carla::ros2::msg::PointField::FLOAT32;
  fy.count = 1u;

  carla::ros2::msg::PointField fz{};
  fz.name = "z";
  fz.offset = 8u;
  fz.datatype = carla::ros2::msg::PointField::FLOAT32;
  fz.count = 1u;

  original.fields.push_back(fx);
  original.fields.push_back(fy);
  original.fields.push_back(fz);
  original.is_bigendian = false;
  original.point_step = 12u;
  original.row_step = 48u;
  original.data.resize(48u, 0u);
  original.is_dense = true;

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::PointCloud2 recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  ASSERT_EQ(recovered.fields.size(), 3u);
  EXPECT_EQ(recovered.fields[0].name, "x");
  EXPECT_EQ(recovered.fields[0].offset, 0u);
  EXPECT_EQ(recovered.fields[1].name, "y");
  EXPECT_EQ(recovered.fields[1].offset, 4u);
  EXPECT_EQ(recovered.fields[2].name, "z");
  EXPECT_EQ(recovered.fields[2].offset, 8u);
  EXPECT_EQ(recovered.point_step, 12u);
  EXPECT_EQ(recovered.row_step, 48u);
  ASSERT_EQ(recovered.data.size(), 48u);
}

TEST(cdr_serialization, tfmessage_multi_transform_round_trip) {
  // Exercises the manual sequence loop for TFMessage::transforms with more
  // than one element.
  carla::ros2::msg::TFMessage original{};

  carla::ros2::msg::TransformStamped a{};
  a.header.stamp.sec = 1;
  a.header.frame_id = "world";
  a.child_frame_id = "robot_a";
  a.transform.translation.x = 1.0;
  a.transform.rotation.w = 1.0;

  carla::ros2::msg::TransformStamped b{};
  b.header.stamp.sec = 2;
  b.header.frame_id = "world";
  b.child_frame_id = "robot_b";
  b.transform.translation.y = 2.0;
  b.transform.rotation.w = 1.0;

  original.transforms.push_back(a);
  original.transforms.push_back(b);

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_FALSE(buf.empty());

  carla::ros2::msg::TFMessage recovered{};
  EXPECT_TRUE(carla::ros2::deserialize_from_cdr(buf.data(), buf.size(), recovered));
  ASSERT_EQ(recovered.transforms.size(), 2u);
  EXPECT_EQ(recovered.transforms[0].child_frame_id, "robot_a");
  EXPECT_DOUBLE_EQ(recovered.transforms[0].transform.translation.x, 1.0);
  EXPECT_EQ(recovered.transforms[1].child_frame_id, "robot_b");
  EXPECT_DOUBLE_EQ(recovered.transforms[1].transform.translation.y, 2.0);
}

TEST(cdr_serialization, deserialize_truncated_returns_false) {
  // A truncated buffer must produce a clean false return, not an uncaught
  // Fast-CDR exception leaking out of the bool API.
  carla::ros2::msg::Header original{};
  original.stamp.sec = 7;
  original.frame_id = "needs_more_bytes";

  auto buf = carla::ros2::serialize_to_cdr(original);
  ASSERT_GT(buf.size(), 8u);

  carla::ros2::msg::Header recovered{};
  EXPECT_FALSE(carla::ros2::deserialize_from_cdr(
      buf.data(), buf.size() / 2u, recovered));
}

TEST(cdr_serialization, deserialize_corrupt_encapsulation_returns_false) {
  // A buffer too small to even hold the 4-byte encapsulation header must
  // produce a clean false return.
  const uint8_t bogus[2] = {0xFFu, 0xFFu};
  carla::ros2::msg::Time recovered{};
  EXPECT_FALSE(carla::ros2::deserialize_from_cdr(bogus, sizeof(bogus), recovered));
}

TEST(cdr_serialization, deserialize_pointcloud2_hostile_length_returns_false) {
  // Hand-craft a PointCloud2 buffer that claims its sequence has a hostile
  // length (max uint32). Without the kMaxCdrSequenceElements cap, the
  // call would attempt a multi-GB resize and abort the process.
  std::vector<uint8_t> buf;
  // Encapsulation header: CDR_LE + options.
  buf.push_back(0x00u);
  buf.push_back(0x01u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  // Header.stamp.sec (int32) + nanosec (uint32) = 8 bytes of zeros.
  for (int i = 0; i < 8; ++i) buf.push_back(0x00u);
  // Header.frame_id (string): length 1 (NUL only) + "\0" + 3 padding bytes
  // to keep alignment for the next uint32.
  buf.push_back(0x01u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  // PointCloud2.height (uint32) + width (uint32).
  for (int i = 0; i < 8; ++i) buf.push_back(0x00u);
  // fields_size = 0xFFFFFFFF (hostile).
  buf.push_back(0xFFu);
  buf.push_back(0xFFu);
  buf.push_back(0xFFu);
  buf.push_back(0xFFu);

  carla::ros2::msg::PointCloud2 recovered{};
  EXPECT_FALSE(carla::ros2::deserialize_from_cdr(
      buf.data(), buf.size(), recovered));
}

TEST(cdr_serialization, deserialize_tfmessage_hostile_length_returns_false) {
  // Same idea for TFMessage::transforms — claim a 4-billion-element
  // sequence and verify the cap rejects it instead of OOM-aborting.
  std::vector<uint8_t> buf;
  buf.push_back(0x00u);
  buf.push_back(0x01u);
  buf.push_back(0x00u);
  buf.push_back(0x00u);
  buf.push_back(0xFFu);
  buf.push_back(0xFFu);
  buf.push_back(0xFFu);
  buf.push_back(0xFFu);

  carla::ros2::msg::TFMessage recovered{};
  EXPECT_FALSE(carla::ros2::deserialize_from_cdr(
      buf.data(), buf.size(), recovered));
}

TEST(cdr_serialization, cdr_serialized_size_matches_serialize_to_cdr) {
  // cdr_serialized_size(msg) must return the same byte count as
  // serialize_to_cdr(msg).size() for every message type. This is the contract
  // that GenericCdrPubSubType::getSerializedSizeProvider relies on.
  {
    carla::ros2::msg::Header msg{};
    msg.stamp.sec = 42;
    msg.frame_id = "map";
    EXPECT_EQ(carla::ros2::cdr_serialized_size(msg),
              carla::ros2::serialize_to_cdr(msg).size());
  }
  {
    carla::ros2::msg::Image msg{};
    msg.height = 2u;
    msg.width  = 3u;
    msg.encoding = "rgb8";
    msg.data.assign(6u, 0xAAu);
    EXPECT_EQ(carla::ros2::cdr_serialized_size(msg),
              carla::ros2::serialize_to_cdr(msg).size());
  }
  {
    carla::ros2::msg::PointCloud2 msg{};
    msg.height = 1u;
    msg.width  = 4u;
    msg.data.assign(48u, 0xBBu);
    EXPECT_EQ(carla::ros2::cdr_serialized_size(msg),
              carla::ros2::serialize_to_cdr(msg).size());
  }
  {
    carla::ros2::msg::TFMessage msg{};
    msg.transforms.resize(2u);
    msg.transforms[0].header.frame_id = "world";
    msg.transforms[1].header.frame_id = "base_link";
    EXPECT_EQ(carla::ros2::cdr_serialized_size(msg),
              carla::ros2::serialize_to_cdr(msg).size());
  }
}

TEST(cdr_serialization, cdr_serialized_size_image_exceeds_static_max) {
  // A real 800x600 RGB camera frame is ~1.4 MB. The static
  // CdrTopicInfo<Image>::max_serialized_size() is only 648 bytes.
  // cdr_serialized_size() must return a value greater than the static max,
  // and the round-trip must recover the original data.size().
  carla::ros2::msg::Image msg{};
  msg.height = 600u;
  msg.width  = 800u;
  msg.encoding = "rgb8";
  msg.step = 800u * 3u;
  const size_t data_bytes = 800u * 600u * 3u;  // 1,440,000 bytes
  msg.data.assign(data_bytes, 0x7Fu);

  const uint32_t computed = carla::ros2::cdr_serialized_size(msg);
  EXPECT_GT(computed,
      static_cast<uint32_t>(
          carla::ros2::CdrTopicInfo<carla::ros2::msg::Image>::max_serialized_size()));

  const auto bytes = carla::ros2::serialize_to_cdr(msg);
  ASSERT_FALSE(bytes.empty());
  EXPECT_EQ(computed, static_cast<uint32_t>(bytes.size()));

  carla::ros2::msg::Image recovered{};
  ASSERT_TRUE(carla::ros2::deserialize_from_cdr(
      bytes.data(), bytes.size(), recovered));
  EXPECT_EQ(recovered.data.size(), data_bytes);
}

TEST(cdr_serialization, cdr_serialized_size_pointcloud2_exceeds_static_max) {
  // A typical LiDAR scan is 1-20 MB. The static max_serialized_size() for
  // PointCloud2 is 27597 bytes. This test uses ~1 MB of data to verify the
  // same contract as the Image test above.
  carla::ros2::msg::PointCloud2 msg{};
  msg.height = 1u;
  msg.width  = 22000u;
  msg.row_step = 22000u * 16u;
  const size_t data_bytes = 22000u * 16u;  // ~352,000 bytes (~0.35 MB)
  msg.data.assign(data_bytes, 0x3Cu);
  carla::ros2::msg::PointField pf{};
  pf.name = "x";
  pf.offset = 0u;
  pf.datatype = 7u;  // FLOAT32
  pf.count = 1u;
  msg.fields.push_back(pf);

  const uint32_t computed = carla::ros2::cdr_serialized_size(msg);
  EXPECT_GT(computed,
      static_cast<uint32_t>(
          carla::ros2::CdrTopicInfo<carla::ros2::msg::PointCloud2>::max_serialized_size()));

  const auto bytes = carla::ros2::serialize_to_cdr(msg);
  ASSERT_FALSE(bytes.empty());
  EXPECT_EQ(computed, static_cast<uint32_t>(bytes.size()));

  carla::ros2::msg::PointCloud2 recovered{};
  ASSERT_TRUE(carla::ros2::deserialize_from_cdr(
      bytes.data(), bytes.size(), recovered));
  EXPECT_EQ(recovered.data.size(), data_bytes);
}
