// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Must be defined before any includes to suppress real DDS auto-includes
// while keeping CARLA_ROS2_DDS_FASTDDS compile-time checks active.
#define CARLA_ROS2_DDS_TESTING
#define CARLA_ROS2_DDS_FASTDDS

#include "test.h"

#include <carla/ros2/dds/DDSMiddleware.h>
#include <carla/ros2/dds/DDSMiddlewareFactory.h>
#include <carla/ros2/dds/IDDSPublisherMiddleware.h>
#include <carla/ros2/dds/IDDSSubscriberMiddleware.h>
#include <carla/ros2/publishers/PublisherImpl.h>
#include <carla/ros2/subscribers/SubscriberImpl.h>

#include <memory>
#include <string>

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

class MockPublisherMiddleware : public IDDSPublisherMiddleware {
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

class MockSubscriberMiddleware : public IDDSSubscriberMiddleware {
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

class DDSMiddlewareFactoryFixture : public ::testing::Test {
 protected:
  void SetUp() override {
    DDSMiddlewareFactory::SetMiddleware(DDSMiddleware::FastDDS);
  }
};

// ==========================================================================
// Group 1: dds_middleware_enum (2 tests)
// ==========================================================================

TEST(dds_middleware_enum, values_exist) {
  DDSMiddleware mw_fast = DDSMiddleware::FastDDS;
  EXPECT_EQ(static_cast<int>(mw_fast), 0);
  DDSMiddleware mw_cyclone = DDSMiddleware::CycloneDDS;
  EXPECT_EQ(static_cast<int>(mw_cyclone), 1);
}

TEST(dds_middleware_enum, switch_covers_all) {
  DDSMiddleware values[] = {DDSMiddleware::FastDDS, DDSMiddleware::CycloneDDS};
  for (auto mw : values) {
    bool covered = false;
    switch (mw) {
      case DDSMiddleware::FastDDS:
        covered = true;
        break;
      case DDSMiddleware::CycloneDDS:
        covered = true;
        break;
    }
    EXPECT_TRUE(covered);
  }
}

// ==========================================================================
// Group 2: dds_middleware_to_string (3 tests)
// ==========================================================================

TEST(dds_middleware_to_string, fastdds_returns_correct_string) {
  EXPECT_STREQ(DDSMiddlewareToString(DDSMiddleware::FastDDS), "FastDDS");
}

TEST(dds_middleware_to_string, result_is_not_null) {
  const char* result = DDSMiddlewareToString(DDSMiddleware::FastDDS);
  ASSERT_NE(result, nullptr);
}

TEST(dds_middleware_to_string, result_is_not_empty) {
  const char* result = DDSMiddlewareToString(DDSMiddleware::FastDDS);
  EXPECT_STRNE(result, "");
}

TEST(dds_middleware_to_string, cyclonedds_returns_correct_string) {
  EXPECT_STREQ(DDSMiddlewareToString(DDSMiddleware::CycloneDDS), "CycloneDDS");
}

// ==========================================================================
// Group 3: dds_middleware_from_string (5 tests)
// ==========================================================================

TEST(dds_middleware_from_string, fastdds_lowercase_valid) {
  auto result = DDSMiddlewareFromString("fastdds");
  EXPECT_TRUE(result.valid);
  EXPECT_EQ(result.middleware, DDSMiddleware::FastDDS);
}

TEST(dds_middleware_from_string, unknown_string_invalid) {
  auto result = DDSMiddlewareFromString("unknowndds");
  EXPECT_FALSE(result.valid);
}

TEST(dds_middleware_from_string, cyclonedds_lowercase_valid) {
  auto result = DDSMiddlewareFromString("cyclonedds");
  EXPECT_TRUE(result.valid);
  EXPECT_EQ(result.middleware, DDSMiddleware::CycloneDDS);
}

TEST(dds_middleware_from_string, empty_string_invalid) {
  auto result = DDSMiddlewareFromString("");
  EXPECT_FALSE(result.valid);
}

TEST(dds_middleware_from_string, uppercase_rejected) {
  auto result = DDSMiddlewareFromString("FastDDS");
  EXPECT_FALSE(result.valid);
}

TEST(dds_middleware_from_string, partial_match_rejected) {
  auto result = DDSMiddlewareFromString("fast");
  EXPECT_FALSE(result.valid);
}

// ==========================================================================
// Group 4: dds_middleware_available (2 tests)
// ==========================================================================

TEST(dds_middleware_available, fastdds_available) {
  EXPECT_TRUE(
      DDSMiddlewareFactory::IsMiddlewareAvailable(DDSMiddleware::FastDDS));
}

TEST(dds_middleware_available, available_string_contains_fastdds) {
  std::string available = GetAvailableMiddlewareString();
  EXPECT_NE(available.find("FastDDS"), std::string::npos);
}

TEST(dds_middleware_available, cyclonedds_not_available_without_macro) {
  EXPECT_FALSE(
      DDSMiddlewareFactory::IsMiddlewareAvailable(DDSMiddleware::CycloneDDS));
}

// ==========================================================================
// Group 5: dds_middleware_type_name (4 tests)
// ==========================================================================

TEST(dds_middleware_type_name, bare_name) {
  EXPECT_EQ(ToROS2DDSTypeName("Image"), "dds_::Image_");
}

TEST(dds_middleware_type_name, fully_qualified) {
  EXPECT_EQ(
      ToROS2DDSTypeName("sensor_msgs::msg::Image"),
      "sensor_msgs::msg::dds_::Image_");
}

TEST(dds_middleware_type_name, single_namespace) {
  EXPECT_EQ(ToROS2DDSTypeName("msg::Image"), "msg::dds_::Image_");
}

TEST(dds_middleware_type_name, empty_string) {
  EXPECT_EQ(ToROS2DDSTypeName(""), "dds_::_");
}

// ==========================================================================
// Group 6: DDSMiddlewareFactoryFixture (5 tests)
// ==========================================================================

TEST_F(DDSMiddlewareFactoryFixture, set_and_get_middleware) {
  DDSMiddlewareFactory::SetMiddleware(DDSMiddleware::FastDDS);
  EXPECT_EQ(DDSMiddlewareFactory::GetMiddleware(), DDSMiddleware::FastDDS);
}

TEST_F(DDSMiddlewareFactoryFixture, default_is_fastdds) {
  EXPECT_EQ(DDSMiddlewareFactory::GetMiddleware(), DDSMiddleware::FastDDS);
}

TEST_F(DDSMiddlewareFactoryFixture, is_middleware_available_fastdds) {
  EXPECT_TRUE(
      DDSMiddlewareFactory::IsMiddlewareAvailable(DDSMiddleware::FastDDS));
}

TEST_F(DDSMiddlewareFactoryFixture, resolve_available_middleware) {
  auto resolution =
      DDSMiddlewareFactory::ResolveMiddleware(DDSMiddleware::FastDDS);
  EXPECT_TRUE(resolution.success);
  EXPECT_EQ(resolution.middleware, DDSMiddleware::FastDDS);
}

TEST_F(DDSMiddlewareFactoryFixture, factory_available_string) {
  std::string available = DDSMiddlewareFactory::GetAvailableMiddlewareString();
  EXPECT_NE(available.find("FastDDS"), std::string::npos);
}

TEST_F(DDSMiddlewareFactoryFixture, set_and_get_cyclonedds) {
  DDSMiddlewareFactory::SetMiddleware(DDSMiddleware::CycloneDDS);
  EXPECT_EQ(DDSMiddlewareFactory::GetMiddleware(), DDSMiddleware::CycloneDDS);
}

TEST_F(DDSMiddlewareFactoryFixture, resolve_unavailable_cyclonedds) {
  auto resolution =
      DDSMiddlewareFactory::ResolveMiddleware(DDSMiddleware::CycloneDDS);
  EXPECT_FALSE(resolution.success);
  EXPECT_EQ(resolution.middleware, DDSMiddleware::CycloneDDS);
}

TEST_F(DDSMiddlewareFactoryFixture, create_publisher_cyclonedds_unavailable) {
  DDSMiddlewareFactory::SetMiddleware(DDSMiddleware::CycloneDDS);
  ::testing::internal::CaptureStderr();
  auto pub = DDSMiddlewareFactory::CreatePublisher<TestPubTraits>();
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(pub, nullptr);
}

TEST_F(DDSMiddlewareFactoryFixture, create_subscriber_cyclonedds_unavailable) {
  DDSMiddlewareFactory::SetMiddleware(DDSMiddleware::CycloneDDS);
  ::testing::internal::CaptureStderr();
  auto sub = DDSMiddlewareFactory::CreateSubscriber<TestSubTraits>();
  ::testing::internal::GetCapturedStderr();
  EXPECT_EQ(sub, nullptr);
}

// ==========================================================================
// Group 7: dds_publisher_interface (5 tests)
// ==========================================================================

TEST(dds_publisher_interface, mock_init_success) {
  MockPublisherMiddleware mock;
  mock.init_return_value = true;
  EXPECT_TRUE(mock.Init("rt/test_topic"));
  EXPECT_TRUE(mock.init_called);
  EXPECT_EQ(mock.last_topic_name, "rt/test_topic");
}

TEST(dds_publisher_interface, mock_init_failure) {
  MockPublisherMiddleware mock;
  mock.init_return_value = false;
  EXPECT_FALSE(mock.Init("rt/test_topic"));
  EXPECT_TRUE(mock.init_called);
}

TEST(dds_publisher_interface, mock_publish_records_data) {
  MockPublisherMiddleware mock;
  TestMsg msg;
  msg.value = 42;
  EXPECT_TRUE(mock.Publish(&msg));
  EXPECT_TRUE(mock.publish_called);
  EXPECT_EQ(mock.last_published_data, &msg);
}

TEST(dds_publisher_interface, mock_topic_name) {
  MockPublisherMiddleware mock;
  mock.Init("rt/camera/image");
  EXPECT_EQ(mock.GetTopicName(), "rt/camera/image");
}

TEST(dds_publisher_interface, mock_is_alive) {
  MockPublisherMiddleware mock;
  mock.alive = true;
  EXPECT_TRUE(mock.IsAlive());
  mock.alive = false;
  EXPECT_FALSE(mock.IsAlive());
}

// ==========================================================================
// Group 8: dds_subscriber_interface (5 tests)
// ==========================================================================

TEST(dds_subscriber_interface, mock_init_stores_pointers) {
  MockSubscriberMiddleware mock;
  TestMsg msg;
  bool flag = false;
  EXPECT_TRUE(mock.Init("rt/test_topic", &msg, &flag));
  EXPECT_TRUE(mock.init_called);
  EXPECT_EQ(mock.stored_message_ptr, &msg);
  EXPECT_EQ(mock.stored_flag_ptr, &flag);
}

TEST(dds_subscriber_interface, mock_init_failure) {
  MockSubscriberMiddleware mock;
  mock.init_return_value = false;
  TestMsg msg;
  bool flag = false;
  EXPECT_FALSE(mock.Init("rt/test_topic", &msg, &flag));
}

TEST(dds_subscriber_interface, mock_topic_name) {
  MockSubscriberMiddleware mock;
  TestMsg msg;
  bool flag = false;
  mock.Init("rt/lidar/points", &msg, &flag);
  EXPECT_EQ(mock.GetTopicName(), "rt/lidar/points");
}

TEST(dds_subscriber_interface, mock_is_alive) {
  MockSubscriberMiddleware mock;
  mock.alive = true;
  EXPECT_TRUE(mock.IsAlive());
  mock.alive = false;
  EXPECT_FALSE(mock.IsAlive());
}

TEST(dds_subscriber_interface, mock_simulates_message_receipt) {
  MockSubscriberMiddleware mock;
  TestMsg msg;
  bool flag = false;
  mock.Init("rt/test", &msg, &flag);
  ASSERT_NE(mock.stored_message_ptr, nullptr);
  ASSERT_NE(mock.stored_flag_ptr, nullptr);
  auto* typed_ptr = static_cast<TestMsg*>(mock.stored_message_ptr);
  typed_ptr->value = 99;
  *mock.stored_flag_ptr = true;
  EXPECT_EQ(msg.value, 99);
  EXPECT_TRUE(flag);
}

// ==========================================================================
// Group 9: publisher_impl (7 tests)
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
      std::unique_ptr<IDDSPublisherMiddleware>(mock));
  EXPECT_TRUE(pub.Init("rt/test_topic"));
  EXPECT_TRUE(mock->init_called);
  EXPECT_EQ(mock->last_topic_name, "rt/test_topic");
}

TEST(publisher_impl, publish_delegates_to_middleware) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IDDSPublisherMiddleware>(mock));
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
      std::unique_ptr<IDDSPublisherMiddleware>(mock));
  pub.Init("rt/test_topic");
  EXPECT_TRUE(pub.IsAlive());
  mock->alive = false;
  EXPECT_FALSE(pub.IsAlive());
}

TEST(publisher_impl, topic_name_delegates) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IDDSPublisherMiddleware>(mock));
  pub.Init("rt/camera/image");
  EXPECT_EQ(pub.GetTopicName(), "rt/camera/image");
}

TEST(publisher_impl, data_flows_through_publish) {
  PublisherImpl<TestPubTraits> pub;
  auto* mock = new MockPublisherMiddleware();
  pub.SetMiddlewareForTesting(
      std::unique_ptr<IDDSPublisherMiddleware>(mock));
  pub.Init("rt/test_topic");

  pub.GetMessage()->value = 42;
  pub.Publish();

  ASSERT_NE(mock->last_published_data, nullptr);
  auto* published = static_cast<TestMsg*>(mock->last_published_data);
  EXPECT_EQ(published->value, 42);
}

// ==========================================================================
// Group 10: subscriber_impl (7 tests)
// ==========================================================================

TEST(subscriber_impl, has_new_message_initially_false) {
  SubscriberImpl<TestSubTraits> sub;
  EXPECT_FALSE(sub.HasNewMessage());
}

TEST(subscriber_impl, init_delegates_to_middleware) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<IDDSSubscriberMiddleware>(mock));
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
      std::unique_ptr<IDDSSubscriberMiddleware>(mock));
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
      std::unique_ptr<IDDSSubscriberMiddleware>(mock));
  sub.Init("rt/test_topic");
  EXPECT_TRUE(sub.IsAlive());
  mock->alive = false;
  EXPECT_FALSE(sub.IsAlive());
}

TEST(subscriber_impl, topic_name_delegates) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<IDDSSubscriberMiddleware>(mock));
  sub.Init("rt/lidar/points");
  EXPECT_EQ(sub.GetTopicName(), "rt/lidar/points");
}

TEST(subscriber_impl, simulate_message_receipt) {
  SubscriberImpl<TestSubTraits> sub;
  auto* mock = new MockSubscriberMiddleware();
  sub.SetMiddlewareForTesting(
      std::unique_ptr<IDDSSubscriberMiddleware>(mock));
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
      std::unique_ptr<IDDSSubscriberMiddleware>(mock));
  EXPECT_FALSE(sub.Init("rt/test_topic"));
}
