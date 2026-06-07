// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Must be defined before any includes to suppress real DDS auto-includes
// while keeping CARLA_ROS2_MIDDLEWARE_FASTDDS compile-time checks active.
#define CARLA_ROS2_MIDDLEWARE_TESTING
#define CARLA_ROS2_MIDDLEWARE_FASTDDS

#include "test.h"

#include <carla/ros2/middleware/Middleware.h>
#include <carla/ros2/middleware/MiddlewareConfig.h>
#include <carla/ros2/middleware/MiddlewareFactory.h>
#include <carla/ros2/middleware/IPublisherMiddleware.h>
#include <carla/ros2/middleware/ISubscriberMiddleware.h>
#include <carla/ros2/publishers/PublisherImpl.h>
#include <carla/ros2/subscribers/SubscriberImpl.h>
#include <carla/ros2/middleware/fastdds/GenericCdrPubSubType.h>
#include <carla/ros2/types/CdrSerialization.h>
#include <carla/ros2/types/CdrTopicInfo.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using namespace carla::ros2;

// ==========================================================================
// Test infrastructure
// ==========================================================================

struct TestMsg {
  int value{0};
};

struct TestPubTraits {
  using msg_type = TestMsg;
};

struct TestSubTraits {
  using msg_type = TestMsg;
};

// -- Mock publisher middleware ------------------------------------------------

class MockPublisherMiddleware : public IPublisherMiddleware {
 public:
  bool init_return_value{true};
  bool publish_return_value{true};
  bool alive{true};

  bool init_called{false};
  bool publish_called{false};
  std::string last_topic_name;
  void* last_published_data{nullptr};

  bool Init(const std::string& topic_name) override {
    init_called = true;
    last_topic_name = topic_name;
    return init_return_value;
  }

  bool Publish(void* message_data) override {
    publish_called = true;
    last_published_data = message_data;
    return publish_return_value;
  }

  bool IsAlive() const override { return alive; }
  std::string GetTopicName() const override { return last_topic_name; }
};

// -- Mock subscriber middleware -----------------------------------------------

class MockSubscriberMiddleware : public ISubscriberMiddleware {
 public:
  bool init_return_value{true};
  bool alive{true};

  bool init_called{false};
  std::string last_topic_name;
  void* stored_message_ptr{nullptr};
  bool* stored_flag_ptr{nullptr};

  bool Init(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag) override {
    init_called = true;
    last_topic_name = topic_name;
    stored_message_ptr = message_ptr;
    stored_flag_ptr = new_message_flag;
    return init_return_value;
  }

  bool IsAlive() const override { return alive; }
  std::string GetTopicName() const override { return last_topic_name; }
};

// -- Factory fixture (resets static state) ------------------------------------

class MiddlewareFactoryFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    MiddlewareFactory::SetMiddleware(Middleware::FastDDS);
  }
};

// ==========================================================================
// Group 1: middleware_to_string (2 tests)
// ==========================================================================

TEST(middleware_to_string, fastdds_returns_correct_string) {
  EXPECT_STREQ(MiddlewareToString(Middleware::FastDDS), "FastDDS");
}

TEST(middleware_to_string, cyclonedds_returns_correct_string) {
  EXPECT_STREQ(MiddlewareToString(Middleware::CycloneDDS), "CycloneDDS");
}

// ==========================================================================
// Group 2: middleware_from_string (5 tests)
// ==========================================================================

TEST(middleware_from_string, fastdds_lowercase_valid) {
  auto result = MiddlewareFromString("fastdds");
  EXPECT_TRUE(result.valid);
  EXPECT_EQ(result.middleware, Middleware::FastDDS);
}

TEST(middleware_from_string, unknown_string_invalid) {
  auto result = MiddlewareFromString("unknowndds");
  EXPECT_FALSE(result.valid);
}

TEST(middleware_from_string, cyclonedds_lowercase_valid) {
  auto result = MiddlewareFromString("cyclonedds");
  EXPECT_TRUE(result.valid);
  EXPECT_EQ(result.middleware, Middleware::CycloneDDS);
}

TEST(middleware_from_string, empty_string_invalid) {
  auto result = MiddlewareFromString("");
  EXPECT_FALSE(result.valid);
}

TEST(middleware_from_string, uppercase_rejected) {
  auto result = MiddlewareFromString("FastDDS");
  EXPECT_FALSE(result.valid);
}

TEST(middleware_from_string, partial_match_rejected) {
  auto result = MiddlewareFromString("fast");
  EXPECT_FALSE(result.valid);
}

// ==========================================================================
// Group 3: middleware_available (2 tests)
// ==========================================================================

TEST(middleware_available, fastdds_available) {
  EXPECT_TRUE(
      MiddlewareFactory::IsMiddlewareAvailable(Middleware::FastDDS));
}

TEST(middleware_available, available_string_contains_fastdds) {
  std::string available = GetAvailableMiddlewareString();
  EXPECT_NE(available.find("FastDDS"), std::string::npos);
}

TEST(middleware_available, cyclonedds_not_available_without_macro) {
  EXPECT_FALSE(
      MiddlewareFactory::IsMiddlewareAvailable(Middleware::CycloneDDS));
}

// ==========================================================================
// Group 4: middleware_type_name (4 tests)
// ==========================================================================

TEST(middleware_type_name, bare_name) {
  EXPECT_EQ(ToROS2TypeName("Image"), "dds_::Image_");
}

TEST(middleware_type_name, fully_qualified) {
  EXPECT_EQ(
      ToROS2TypeName("sensor_msgs::msg::Image"),
      "sensor_msgs::msg::dds_::Image_");
}

TEST(middleware_type_name, single_namespace) {
  EXPECT_EQ(ToROS2TypeName("msg::Image"), "msg::dds_::Image_");
}

TEST(middleware_type_name, empty_string) {
  EXPECT_EQ(ToROS2TypeName(""), "dds_::_");
}

// ==========================================================================
// Group 5: middleware_config (domain id) (8 tests)
// ==========================================================================

// Restores the global domain id to the unset sentinel after each test so the
// process-wide MiddlewareConfig state does not leak between cases.
class MiddlewareConfigFixture : public ::testing::Test {
 protected:
  void TearDown() override {
    MiddlewareConfig::SetDomainId(kUnsetDomainId);
  }
};

TEST_F(MiddlewareConfigFixture, default_domain_id_is_unset) {
  EXPECT_EQ(MiddlewareConfig::GetDomainId(), kUnsetDomainId);
}

TEST_F(MiddlewareConfigFixture, set_and_get_round_trip) {
  MiddlewareConfig::SetDomainId(42);
  EXPECT_EQ(MiddlewareConfig::GetDomainId(), 42);
}

TEST_F(MiddlewareConfigFixture, set_zero_is_stored) {
  MiddlewareConfig::SetDomainId(0);
  EXPECT_EQ(MiddlewareConfig::GetDomainId(), 0);
}

TEST(middleware_config_valid, zero_is_valid) {
  EXPECT_TRUE(IsValidDomainId(0));
}

TEST(middleware_config_valid, one_is_valid) {
  EXPECT_TRUE(IsValidDomainId(1));
}

TEST(middleware_config_valid, max_is_valid) {
  EXPECT_TRUE(IsValidDomainId(kMaxDomainId));
}

TEST(middleware_config_valid, above_max_is_invalid) {
  EXPECT_FALSE(IsValidDomainId(kMaxDomainId + 1));
}

TEST(middleware_config_valid, unset_sentinel_is_invalid) {
  EXPECT_FALSE(IsValidDomainId(kUnsetDomainId));
}

// ==========================================================================
// Group 5: MiddlewareFactoryFixture (8 tests)
// ==========================================================================

TEST_F(MiddlewareFactoryFixture, set_and_get_middleware) {
  MiddlewareFactory::SetMiddleware(Middleware::FastDDS);
  EXPECT_EQ(MiddlewareFactory::GetMiddleware(), Middleware::FastDDS);
}

TEST_F(MiddlewareFactoryFixture, default_is_fastdds) {
  EXPECT_EQ(MiddlewareFactory::GetMiddleware(), Middleware::FastDDS);
}

TEST_F(MiddlewareFactoryFixture, resolve_available_middleware) {
  auto resolution =
      MiddlewareFactory::ResolveMiddleware(Middleware::FastDDS);
  EXPECT_TRUE(resolution.success);
  EXPECT_EQ(resolution.middleware, Middleware::FastDDS);
}

TEST_F(MiddlewareFactoryFixture, factory_available_string) {
  std::string available = MiddlewareFactory::GetAvailableMiddlewareString();
  EXPECT_NE(available.find("FastDDS"), std::string::npos);
}

TEST_F(MiddlewareFactoryFixture, set_and_get_cyclonedds) {
  MiddlewareFactory::SetMiddleware(Middleware::CycloneDDS);
  EXPECT_EQ(MiddlewareFactory::GetMiddleware(), Middleware::CycloneDDS);
}

TEST_F(MiddlewareFactoryFixture, resolve_unavailable_cyclonedds) {
  auto resolution =
      MiddlewareFactory::ResolveMiddleware(Middleware::CycloneDDS);
  EXPECT_FALSE(resolution.success);
  EXPECT_EQ(resolution.middleware, Middleware::CycloneDDS);
}

TEST_F(MiddlewareFactoryFixture, create_publisher_cyclonedds_unavailable) {
  MiddlewareFactory::SetMiddleware(Middleware::CycloneDDS);
  ::testing::internal::CaptureStderr();
  auto pub = MiddlewareFactory::CreatePublisher<TestPubTraits>();
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(pub, nullptr);
}

TEST_F(MiddlewareFactoryFixture, create_subscriber_cyclonedds_unavailable) {
  MiddlewareFactory::SetMiddleware(Middleware::CycloneDDS);
  ::testing::internal::CaptureStderr();
  auto sub = MiddlewareFactory::CreateSubscriber<TestSubTraits>();
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(sub, nullptr);
}

// ==========================================================================
// Group 6: publisher_impl (7 tests)
// ==========================================================================

TEST(publisher_impl, get_message_returns_pointer) {
  PublisherImpl<TestPubTraits> pub;
  TestMsg* msg = pub.GetMessage();
  ASSERT_NE(msg, nullptr);
  msg->value = 7;
  EXPECT_EQ(pub.GetMessage()->value, 7);
}

TEST(publisher_impl, init_delegates_to_middleware) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IPublisherMiddleware>(mock));
  EXPECT_TRUE(pub.Init("rt/test_topic"));
  EXPECT_TRUE(mock->init_called);
  EXPECT_EQ(mock->last_topic_name, "rt/test_topic");
}

TEST(publisher_impl, publish_delegates_to_middleware) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IPublisherMiddleware>(mock));
  pub.Init("rt/test_topic");
  EXPECT_TRUE(pub.Publish());
  EXPECT_TRUE(mock->publish_called);
  EXPECT_EQ(mock->last_published_data, pub.GetMessage());
}

TEST(publisher_impl, publish_before_init_fails) {
  PublisherImpl<TestPubTraits> pub;
  ::testing::internal::CaptureStderr();
  EXPECT_FALSE(pub.Publish());
  ::testing::internal::GetCapturedStderr();
}

TEST(publisher_impl, is_alive_delegates) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  mock->alive = true;
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IPublisherMiddleware>(mock));
  pub.Init("rt/test_topic");
  EXPECT_TRUE(pub.IsAlive());
  mock->alive = false;
  EXPECT_FALSE(pub.IsAlive());
}

TEST(publisher_impl, topic_name_delegates) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IPublisherMiddleware>(mock));
  pub.Init("rt/camera/image");
  EXPECT_EQ(pub.GetTopicName(), "rt/camera/image");
}

TEST(publisher_impl, data_flows_through_publish) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IPublisherMiddleware>(mock));
  pub.Init("rt/test_topic");

  pub.GetMessage()->value = 42;
  pub.Publish();

  ASSERT_NE(mock->last_published_data, nullptr);
  auto* published = static_cast<TestMsg*>(mock->last_published_data);
  EXPECT_EQ(published->value, 42);
}

// ==========================================================================
// Group 7: subscriber_impl (7 tests)
// ==========================================================================

TEST(subscriber_impl, has_new_message_initially_false) {
  SubscriberImpl<TestSubTraits> sub;
  EXPECT_FALSE(sub.HasNewMessage());
}

TEST(subscriber_impl, init_delegates_to_middleware) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<ISubscriberMiddleware>(mock));
  EXPECT_TRUE(sub.Init("rt/test_topic"));
  EXPECT_TRUE(mock->init_called);
  EXPECT_EQ(mock->last_topic_name, "rt/test_topic");
  EXPECT_NE(mock->stored_message_ptr, nullptr);
  EXPECT_NE(mock->stored_flag_ptr, nullptr);
}

TEST(subscriber_impl, get_message_clears_flag) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<ISubscriberMiddleware>(mock));
  sub.Init("rt/test_topic");

  TestMsg msg;
  msg.value = 77;
  sub.SimulateMessageReceiptForTesting(msg);
  EXPECT_TRUE(sub.HasNewMessage());

  TestMsg retrieved = sub.GetMessage();
  EXPECT_EQ(retrieved.value, 77);
  EXPECT_FALSE(sub.HasNewMessage());
}

TEST(subscriber_impl, is_alive_delegates) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  mock->alive = true;
  sub.SetMiddlewareForTesting(
      std::unique_ptr<ISubscriberMiddleware>(mock));
  sub.Init("rt/test_topic");
  EXPECT_TRUE(sub.IsAlive());
  mock->alive = false;
  EXPECT_FALSE(sub.IsAlive());
}

TEST(subscriber_impl, topic_name_delegates) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<ISubscriberMiddleware>(mock));
  sub.Init("rt/lidar/points");
  EXPECT_EQ(sub.GetTopicName(), "rt/lidar/points");
}

TEST(subscriber_impl, simulate_message_receipt) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<ISubscriberMiddleware>(mock));
  sub.Init("rt/test_topic");

  EXPECT_FALSE(sub.HasNewMessage());
  TestMsg msg;
  msg.value = 123;
  sub.SimulateMessageReceiptForTesting(msg);
  EXPECT_TRUE(sub.HasNewMessage());
  EXPECT_EQ(sub.GetMessage().value, 123);
}

TEST(subscriber_impl, init_failure_propagated) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  mock->init_return_value = false;
  sub.SetMiddlewareForTesting(
      std::unique_ptr<ISubscriberMiddleware>(mock));
  EXPECT_FALSE(sub.Init("rt/test_topic"));
}

// ==========================================================================
// Group 8: cdr_topic_info (2 tests)
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
// Group 9: cdr_serialization (21 tests)
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

// ==========================================================================
// Group 10: generic_cdr_pubsubtype (6 tests)
// Tests for GenericCdrPubSubType<T> — the single FastDDS TopicDataType
// implementation that replaces 30 fastddsgen-generated PubSubType classes.
// ==========================================================================

TEST(generic_cdr_pubsubtype, type_name_matches_cdr_topic_info) {
  // The name set in the GenericCdrPubSubType constructor must equal
  // CdrTopicInfo::type_name() — FastDDS uses it for publisher/subscriber matching.
  EXPECT_STREQ(
      CdrTopicInfo<msg::Clock>::type_name(),
      GenericCdrPubSubType<msg::Clock>().getName());
  EXPECT_STREQ(
      CdrTopicInfo<msg::Image>::type_name(),
      GenericCdrPubSubType<msg::Image>().getName());
  EXPECT_STREQ(
      CdrTopicInfo<msg::Imu>::type_name(),
      GenericCdrPubSubType<msg::Imu>().getName());
  EXPECT_STREQ(
      CdrTopicInfo<msg::TFMessage>::type_name(),
      GenericCdrPubSubType<msg::TFMessage>().getName());
  EXPECT_STREQ(
      CdrTopicInfo<msg::CarlaEgoVehicleControl>::type_name(),
      GenericCdrPubSubType<msg::CarlaEgoVehicleControl>().getName());
}

TEST(generic_cdr_pubsubtype, m_typesize_is_positive) {
  // FastDDS uses m_typeSize to pre-allocate payload buffers.
  // It must be > 0 for every type (min: max_serialized_size + 4 encapsulation bytes).
  EXPECT_GT(GenericCdrPubSubType<msg::Time>().m_typeSize, 0u);
  EXPECT_GT(GenericCdrPubSubType<msg::Clock>().m_typeSize, 0u);
  EXPECT_GT(GenericCdrPubSubType<msg::Header>().m_typeSize, 0u);
  EXPECT_GT(GenericCdrPubSubType<msg::Imu>().m_typeSize, 0u);
  EXPECT_GT(GenericCdrPubSubType<msg::Image>().m_typeSize, 0u);
  EXPECT_GT(GenericCdrPubSubType<msg::PointCloud2>().m_typeSize, 0u);
}

TEST(generic_cdr_pubsubtype, serialize_deserialize_fixed_size_via_payload) {
  // Round-trip a fixed-size type (Clock) through a SerializedPayload_t buffer.
  // This exercises the exact code path FastDDS DataWriter/DataReader use.
  using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;

  GenericCdrPubSubType<msg::Clock> pubsub_type;
  msg::Clock original{};
  original.clock.sec = 100;
  original.clock.nanosec = 500u;

  SerializedPayload_t payload(1024u);
  ASSERT_TRUE(pubsub_type.serialize(static_cast<void*>(&original), &payload));
  EXPECT_GT(payload.length, 0u);

  msg::Clock recovered{};
  ASSERT_TRUE(pubsub_type.deserialize(&payload, static_cast<void*>(&recovered)));
  EXPECT_EQ(recovered.clock.sec, 100);
  EXPECT_EQ(recovered.clock.nanosec, 500u);
}

TEST(generic_cdr_pubsubtype, serialize_deserialize_string_type_via_payload) {
  // Round-trip a type containing a std::string (Header) through SerializedPayload_t.
  using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;

  GenericCdrPubSubType<msg::Header> pubsub_type;
  msg::Header original{};
  original.stamp.sec = 42;
  original.frame_id = "test_frame";

  SerializedPayload_t payload(4096u);
  ASSERT_TRUE(pubsub_type.serialize(static_cast<void*>(&original), &payload));
  EXPECT_GT(payload.length, 0u);

  msg::Header recovered{};
  ASSERT_TRUE(pubsub_type.deserialize(&payload, static_cast<void*>(&recovered)));
  EXPECT_EQ(recovered.stamp.sec, 42);
  EXPECT_EQ(recovered.frame_id, "test_frame");
}

TEST(generic_cdr_pubsubtype, create_and_delete_data) {
  GenericCdrPubSubType<msg::Clock> pubsub_type;

  void* data = pubsub_type.createData();
  ASSERT_NE(data, nullptr);

  // Cast to verify it is a properly-constructed Clock
  msg::Clock* clock = static_cast<msg::Clock*>(data);
  EXPECT_EQ(clock->clock.sec, 0);
  EXPECT_EQ(clock->clock.nanosec, 0u);

  // Must not crash
  pubsub_type.deleteData(data);
}

TEST(generic_cdr_pubsubtype, getkey_returns_false) {
  // CARLA has no keyed topics — getKey must always return false.
  GenericCdrPubSubType<msg::Clock> pubsub_type;
  EXPECT_FALSE(pubsub_type.m_isGetKeyDefined);
  EXPECT_FALSE(pubsub_type.getKey(nullptr, nullptr, false));
}

// ==========================================================================
// Group 11: generic_cdr_pubsubtype_large_payload (3 tests)
// Tests that getSerializedSizeProvider() returns the actual instance size and
// that serialize/deserialize succeed for payloads exceeding max_serialized_size().
// These tests catch the runtime bug where Image and PointCloud2 publish failed
// with RETCODE_ERROR because the static max size (~648 bytes for Image) was far
// smaller than a real camera frame (~1-8 MB).
// ==========================================================================

TEST(generic_cdr_pubsubtype_large_payload, image_large_payload_serialize_succeeds) {
  using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;

  // 640x480 BGRA image: 1,228,800 bytes of pixel data.
  // This is vastly larger than CdrTopicInfo<msg::Image>::max_serialized_size() = 648.
  constexpr uint32_t width  = 640u;
  constexpr uint32_t height = 480u;
  constexpr uint32_t channels = 4u;
  const uint32_t pixel_bytes = width * height * channels;

  msg::Image original{};
  original.header.stamp.sec = 42;
  original.header.frame_id = "camera";
  original.height = height;
  original.width  = width;
  original.encoding = "bgra8";
  original.is_bigendian = 0u;
  original.step = width * channels;
  original.data.assign(pixel_bytes, 0xABu);

  GenericCdrPubSubType<msg::Image> pubsub_type;

  // getSerializedSizeProvider() must report the actual instance size, not the
  // static max. The returned size must accommodate all pixel data.
  auto size_fn = pubsub_type.getSerializedSizeProvider(
      static_cast<void*>(&original));
  uint32_t reported_size = size_fn();
  EXPECT_GT(reported_size, pixel_bytes)
      << "getSerializedSizeProvider must exceed the raw pixel count";

  SerializedPayload_t payload(reported_size);
  ASSERT_TRUE(pubsub_type.serialize(static_cast<void*>(&original), &payload));
  EXPECT_GT(payload.length, pixel_bytes);

  msg::Image recovered{};
  ASSERT_TRUE(pubsub_type.deserialize(&payload, static_cast<void*>(&recovered)));
  EXPECT_EQ(recovered.header.frame_id, "camera");
  EXPECT_EQ(recovered.height, height);
  EXPECT_EQ(recovered.width, width);
  EXPECT_EQ(recovered.encoding, "bgra8");
  ASSERT_EQ(recovered.data.size(), pixel_bytes);
  EXPECT_EQ(recovered.data[0], 0xABu);
  EXPECT_EQ(recovered.data[pixel_bytes - 1u], 0xABu);
}

TEST(generic_cdr_pubsubtype_large_payload, pointcloud2_large_payload_serialize_succeeds) {
  using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;

  // 10,000 points x 16 bytes/point = 160,000 bytes of point data.
  // This exceeds CdrTopicInfo<msg::PointCloud2>::max_serialized_size() = 27,597.
  constexpr uint32_t num_points = 10000u;
  constexpr uint32_t point_step = 16u;
  const uint32_t data_bytes = num_points * point_step;

  msg::PointCloud2 original{};
  original.header.frame_id = "velodyne";
  original.height = 1u;
  original.width  = num_points;
  original.point_step = point_step;
  original.row_step   = num_points * point_step;
  original.is_dense   = true;
  original.data.assign(data_bytes, 0x55u);

  msg::PointField field{};
  field.name     = "x";
  field.offset   = 0u;
  field.datatype = static_cast<uint8_t>(msg::PointField::FLOAT32);
  field.count    = 1u;
  original.fields.push_back(field);

  GenericCdrPubSubType<msg::PointCloud2> pubsub_type;

  auto size_fn = pubsub_type.getSerializedSizeProvider(
      static_cast<void*>(&original));
  uint32_t reported_size = size_fn();
  EXPECT_GT(reported_size, data_bytes)
      << "getSerializedSizeProvider must exceed the raw point data size";

  SerializedPayload_t payload(reported_size);
  ASSERT_TRUE(pubsub_type.serialize(static_cast<void*>(&original), &payload));
  EXPECT_GT(payload.length, data_bytes);

  msg::PointCloud2 recovered{};
  ASSERT_TRUE(pubsub_type.deserialize(&payload, static_cast<void*>(&recovered)));
  EXPECT_EQ(recovered.header.frame_id, "velodyne");
  EXPECT_EQ(recovered.height, 1u);
  EXPECT_EQ(recovered.width, num_points);
  EXPECT_EQ(recovered.is_dense, true);
  ASSERT_EQ(recovered.data.size(), data_bytes);
  EXPECT_EQ(recovered.data[0], 0x55u);
}

TEST(generic_cdr_pubsubtype_large_payload, size_provider_returns_actual_size_not_max) {
  // getSerializedSizeProvider() must return different sizes for messages with
  // different amounts of variable-length data, proving it is instance-dependent.
  msg::Image small_img{};
  small_img.data.assign(100u, 0u);

  msg::Image large_img{};
  large_img.data.assign(100000u, 0u);

  GenericCdrPubSubType<msg::Image> pubsub_type;

  auto small_fn = pubsub_type.getSerializedSizeProvider(
      static_cast<void*>(&small_img));
  auto large_fn = pubsub_type.getSerializedSizeProvider(
      static_cast<void*>(&large_img));

  uint32_t small_size = small_fn();
  uint32_t large_size = large_fn();

  EXPECT_LT(small_size, large_size)
      << "Size must grow with the data vector length";
  EXPECT_GT(large_size, 100000u)
      << "Size must at least cover the raw data bytes";
}
