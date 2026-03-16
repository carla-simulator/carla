// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// Suppress the auto-FastDDS fallback in DDSBackendFactory.h so this test file
// does not need FastDDS or CycloneDDS headers / libraries.
#define CARLA_ROS2_DDS_TESTING

#include "test.h"

#include "carla/ros2/dds/DDSBackend.h"
#include "carla/ros2/dds/IDDSPublisherBackend.h"
#include "carla/ros2/dds/IDDSSubscriberBackend.h"
#include "carla/ros2/dds/DDSBackendFactory.h"
#include "carla/ros2/publishers/PublisherImpl.h"
#include "carla/ros2/subscribers/SubscriberImpl.h"

#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Minimal message and traits used for template instantiation in all tests.
// No DDS-specific types required.
// ---------------------------------------------------------------------------

struct TestMsg {
  int value { 0 };
};

struct TestPubTraits {
  using msg_type = TestMsg;
};

struct TestSubTraits {
  using msg_type = TestMsg;
};

// ---------------------------------------------------------------------------
// Mock publisher backend — records all calls, returns configurable values.
// ---------------------------------------------------------------------------

class MockPublisherBackend : public carla::ros2::IDDSPublisherBackend {
public:
  // Configurable return values
  bool init_return_value    { true };
  bool publish_return_value { true };
  bool alive                { false };

  // Instrumentation
  bool        init_called    { false };
  bool        publish_called { false };
  std::string last_topic_name;
  void*       last_published_data { nullptr };

  bool Init(const std::string& topic_name) override {
    init_called     = true;
    last_topic_name = topic_name;
    return init_return_value;
  }

  bool Publish(void* message_data) override {
    publish_called      = true;
    last_published_data = message_data;
    return publish_return_value;
  }

  bool IsAlive() const override {
    return alive;
  }

  std::string GetTopicName() const override {
    return last_topic_name;
  }
};

// ---------------------------------------------------------------------------
// Mock subscriber backend — records all calls, stores message/flag pointers.
// ---------------------------------------------------------------------------

class MockSubscriberBackend : public carla::ros2::IDDSSubscriberBackend {
public:
  // Configurable return values
  bool init_return_value { true };
  bool alive             { false };

  // Instrumentation
  bool        init_called        { false };
  std::string last_topic_name;
  void*       stored_message_ptr { nullptr };
  bool*       stored_flag_ptr    { nullptr };

  bool Init(const std::string& topic_name, void* message_ptr, bool* new_message_flag) override {
    init_called        = true;
    last_topic_name    = topic_name;
    stored_message_ptr = message_ptr;
    stored_flag_ptr    = new_message_flag;
    return init_return_value;
  }

  bool IsAlive() const override {
    return alive;
  }

  std::string GetTopicName() const override {
    return last_topic_name;
  }
};

// ===========================================================================
// Group 1: DDSBackend enum
// ===========================================================================

TEST(dds_backend_enum, values_are_distinct) {
  EXPECT_NE(carla::ros2::DDSBackend::FastDDS, carla::ros2::DDSBackend::CycloneDDS);
}

TEST(dds_backend_enum, switch_covers_both_variants) {
  // Verify compile-time exhaustiveness: both variants handled without default.
  auto describe = [](carla::ros2::DDSBackend b) -> const char* {
    switch (b) {
      case carla::ros2::DDSBackend::FastDDS:    return "fastdds";
      case carla::ros2::DDSBackend::CycloneDDS: return "cyclonedds";
    }
    return "unknown";
  };
  EXPECT_STREQ(describe(carla::ros2::DDSBackend::FastDDS),    "fastdds");
  EXPECT_STREQ(describe(carla::ros2::DDSBackend::CycloneDDS), "cyclonedds");
}

// ===========================================================================
// Group 2: DDSBackendFactory — SetBackend / GetBackend
//
// Each test calls SetBackend() at the start to be independent of run order.
// The function-local static in GetActiveBackend() persists across all tests in
// the binary, so explicit reset is required.
// ===========================================================================

class DDSBackendFactoryFixture : public ::testing::Test {
protected:
  void SetUp() override {
    // Reset to the documented default before every test.
    carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::FastDDS);
  }
};

TEST_F(DDSBackendFactoryFixture, default_backend_is_fastdds) {
  // After SetUp resets to FastDDS, GetBackend() must return FastDDS.
  EXPECT_EQ(carla::ros2::DDSBackendFactory::GetBackend(), carla::ros2::DDSBackend::FastDDS);
}

TEST_F(DDSBackendFactoryFixture, set_fastdds_and_get) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::FastDDS);
  EXPECT_EQ(carla::ros2::DDSBackendFactory::GetBackend(), carla::ros2::DDSBackend::FastDDS);
}

TEST_F(DDSBackendFactoryFixture, set_cyclonedds_and_get) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_EQ(carla::ros2::DDSBackendFactory::GetBackend(), carla::ros2::DDSBackend::CycloneDDS);
}

TEST_F(DDSBackendFactoryFixture, switch_between_backends) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_EQ(carla::ros2::DDSBackendFactory::GetBackend(), carla::ros2::DDSBackend::CycloneDDS);

  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::FastDDS);
  EXPECT_EQ(carla::ros2::DDSBackendFactory::GetBackend(), carla::ros2::DDSBackend::FastDDS);

  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_EQ(carla::ros2::DDSBackendFactory::GetBackend(), carla::ros2::DDSBackend::CycloneDDS);
}

// In TESTING mode neither DDS backend is compiled in, so CreatePublisher/
// CreateSubscriber must return nullptr regardless of the selected backend.

TEST_F(DDSBackendFactoryFixture, create_publisher_returns_null_when_no_backend_compiled_fastdds) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::FastDDS);
  auto backend = carla::ros2::DDSBackendFactory::CreatePublisher<TestPubTraits>();
  EXPECT_EQ(backend, nullptr);
}

TEST_F(DDSBackendFactoryFixture, create_publisher_returns_null_when_no_backend_compiled_cyclonedds) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  auto backend = carla::ros2::DDSBackendFactory::CreatePublisher<TestPubTraits>();
  EXPECT_EQ(backend, nullptr);
}

TEST_F(DDSBackendFactoryFixture, create_subscriber_returns_null_when_no_backend_compiled_fastdds) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::FastDDS);
  auto backend = carla::ros2::DDSBackendFactory::CreateSubscriber<TestSubTraits>();
  EXPECT_EQ(backend, nullptr);
}

TEST_F(DDSBackendFactoryFixture, create_subscriber_returns_null_when_no_backend_compiled_cyclonedds) {
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  auto backend = carla::ros2::DDSBackendFactory::CreateSubscriber<TestSubTraits>();
  EXPECT_EQ(backend, nullptr);
}

// ===========================================================================
// Group 3: IDDSPublisherBackend interface — tested via MockPublisherBackend
// ===========================================================================

TEST(dds_publisher_interface, virtual_destructor_via_base_pointer) {
  // Verify polymorphic destruction compiles and executes without crash.
  std::unique_ptr<carla::ros2::IDDSPublisherBackend> backend =
      std::make_unique<MockPublisherBackend>();
  SUCCEED();  // reaching here without crash is the test
}

TEST(dds_publisher_interface, init_records_topic_name) {
  MockPublisherBackend mock;
  EXPECT_FALSE(mock.init_called);
  bool ok = mock.Init("rt/test_topic");
  EXPECT_TRUE(ok);
  EXPECT_TRUE(mock.init_called);
  EXPECT_EQ(mock.last_topic_name, "rt/test_topic");
}

TEST(dds_publisher_interface, init_failure_propagates) {
  MockPublisherBackend mock;
  mock.init_return_value = false;
  EXPECT_FALSE(mock.Init("rt/topic"));
}

TEST(dds_publisher_interface, publish_receives_correct_pointer) {
  MockPublisherBackend mock;
  int dummy = 42;
  mock.Publish(&dummy);
  EXPECT_TRUE(mock.publish_called);
  EXPECT_EQ(mock.last_published_data, static_cast<void*>(&dummy));
}

TEST(dds_publisher_interface, get_topic_name_matches_init_arg) {
  MockPublisherBackend mock;
  mock.Init("rt/camera/image_raw");
  EXPECT_EQ(mock.GetTopicName(), "rt/camera/image_raw");
}

TEST(dds_publisher_interface, is_alive_initially_false) {
  MockPublisherBackend mock;
  EXPECT_FALSE(mock.IsAlive());
}

TEST(dds_publisher_interface, is_alive_reflects_alive_flag) {
  MockPublisherBackend mock;
  mock.alive = true;
  EXPECT_TRUE(mock.IsAlive());
}

// ===========================================================================
// Group 4: IDDSSubscriberBackend interface — tested via MockSubscriberBackend
// ===========================================================================

TEST(dds_subscriber_interface, virtual_destructor_via_base_pointer) {
  std::unique_ptr<carla::ros2::IDDSSubscriberBackend> backend =
      std::make_unique<MockSubscriberBackend>();
  SUCCEED();
}

TEST(dds_subscriber_interface, init_stores_message_and_flag_pointers) {
  MockSubscriberBackend mock;
  TestMsg msg;
  bool    flag = false;
  bool ok = mock.Init("rt/lidar", &msg, &flag);
  EXPECT_TRUE(ok);
  EXPECT_TRUE(mock.init_called);
  EXPECT_EQ(mock.last_topic_name,    "rt/lidar");
  EXPECT_EQ(mock.stored_message_ptr, static_cast<void*>(&msg));
  EXPECT_EQ(mock.stored_flag_ptr,    &flag);
}

TEST(dds_subscriber_interface, init_failure_propagates) {
  MockSubscriberBackend mock;
  mock.init_return_value = false;
  TestMsg msg;
  bool flag = false;
  EXPECT_FALSE(mock.Init("rt/topic", &msg, &flag));
}

TEST(dds_subscriber_interface, get_topic_name_matches_init_arg) {
  MockSubscriberBackend mock;
  TestMsg msg;
  bool flag = false;
  mock.Init("rt/gnss", &msg, &flag);
  EXPECT_EQ(mock.GetTopicName(), "rt/gnss");
}

TEST(dds_subscriber_interface, is_alive_initially_false) {
  MockSubscriberBackend mock;
  EXPECT_FALSE(mock.IsAlive());
}

TEST(dds_subscriber_interface, simulated_message_receipt_via_stored_pointers) {
  MockSubscriberBackend mock;
  TestMsg msg;
  bool flag = false;
  mock.Init("rt/topic", &msg, &flag);

  // Simulate what the DDS callback does: write message, set flag.
  static_cast<TestMsg*>(mock.stored_message_ptr)->value = 99;
  *mock.stored_flag_ptr = true;

  EXPECT_EQ(msg.value, 99);
  EXPECT_TRUE(flag);
}

// ===========================================================================
// Group 5: PublisherImpl<T> delegation via mock injection
// ===========================================================================

TEST(publisher_impl, get_message_returns_valid_internal_pointer) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  EXPECT_NE(pub.GetMessage(), nullptr);
}

TEST(publisher_impl, is_alive_without_backend_returns_false) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  EXPECT_FALSE(pub.IsAlive());
}

TEST(publisher_impl, get_topic_name_without_backend_returns_empty) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  EXPECT_EQ(pub.GetTopicName(), "");
}

TEST(publisher_impl, publish_without_backend_returns_false) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  // Calling Publish() without a backend is an error condition that now logs to
  // stderr. Capture it to prevent noise in test output.
  ::testing::internal::CaptureStderr();
  bool result = pub.Publish();
  ::testing::internal::GetCapturedStderr();
  EXPECT_FALSE(result);
}

TEST(publisher_impl, init_delegates_to_backend_with_correct_topic) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  auto* raw_mock = new MockPublisherBackend();
  pub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSPublisherBackend>(raw_mock));

  bool result = pub.Init("rt/sensor/camera");
  EXPECT_TRUE(result);
  EXPECT_TRUE(raw_mock->init_called);
  EXPECT_EQ(raw_mock->last_topic_name, "rt/sensor/camera");
}

TEST(publisher_impl, init_failure_from_backend_propagates) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  auto* raw_mock = new MockPublisherBackend();
  raw_mock->init_return_value = false;
  pub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSPublisherBackend>(raw_mock));

  EXPECT_FALSE(pub.Init("rt/topic"));
}

TEST(publisher_impl, publish_passes_internal_message_pointer_to_backend) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  auto* raw_mock = new MockPublisherBackend();
  pub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSPublisherBackend>(raw_mock));
  pub.Init("rt/topic");

  pub.GetMessage()->value = 7;
  bool ok = pub.Publish();

  EXPECT_TRUE(ok);
  EXPECT_TRUE(raw_mock->publish_called);
  // The pointer passed to Publish() must be the same as GetMessage() returns.
  EXPECT_EQ(raw_mock->last_published_data, static_cast<void*>(pub.GetMessage()));
  // And it must carry the message value we set.
  EXPECT_EQ(static_cast<TestMsg*>(raw_mock->last_published_data)->value, 7);
}

TEST(publisher_impl, is_alive_reflects_backend_state) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  auto* raw_mock = new MockPublisherBackend();
  raw_mock->alive = true;
  pub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSPublisherBackend>(raw_mock));

  EXPECT_TRUE(pub.IsAlive());
}

TEST(publisher_impl, get_topic_name_reflects_backend_after_init) {
  carla::ros2::PublisherImpl<TestPubTraits> pub;
  auto* raw_mock = new MockPublisherBackend();
  pub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSPublisherBackend>(raw_mock));
  pub.Init("rt/lidar/point_cloud");

  EXPECT_EQ(pub.GetTopicName(), "rt/lidar/point_cloud");
}

// ===========================================================================
// Group 6: SubscriberImpl<S> delegation via mock injection
// ===========================================================================

TEST(subscriber_impl, has_new_message_initially_false) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  EXPECT_FALSE(sub.HasNewMessage());
}

TEST(subscriber_impl, is_alive_without_backend_returns_false) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  EXPECT_FALSE(sub.IsAlive());
}

TEST(subscriber_impl, get_topic_name_without_backend_returns_empty) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  EXPECT_EQ(sub.GetTopicName(), "");
}

TEST(subscriber_impl, init_delegates_to_backend_with_correct_pointers) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  auto* raw_mock = new MockSubscriberBackend();
  sub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSSubscriberBackend>(raw_mock));

  bool result = sub.Init("rt/control/cmd");
  EXPECT_TRUE(result);
  EXPECT_TRUE(raw_mock->init_called);
  EXPECT_EQ(raw_mock->last_topic_name, "rt/control/cmd");

  // Verify the backend received non-null pointers (addresses of internal storage).
  EXPECT_NE(raw_mock->stored_message_ptr, nullptr);
  EXPECT_NE(raw_mock->stored_flag_ptr,    nullptr);

  // Verify the pointers point into sub's storage by writing through them.
  static_cast<TestMsg*>(raw_mock->stored_message_ptr)->value = 55;
  *raw_mock->stored_flag_ptr = true;
  EXPECT_TRUE(sub.HasNewMessage());
  EXPECT_EQ(sub.GetMessage().value, 55);
}

TEST(subscriber_impl, init_failure_from_backend_propagates) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  auto* raw_mock = new MockSubscriberBackend();
  raw_mock->init_return_value = false;
  sub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSSubscriberBackend>(raw_mock));

  EXPECT_FALSE(sub.Init("rt/topic"));
}

TEST(subscriber_impl, get_message_returns_value_and_clears_new_message_flag) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  auto* raw_mock = new MockSubscriberBackend();
  sub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSSubscriberBackend>(raw_mock));

  TestMsg expected;
  expected.value = 42;
  sub.SimulateMessageReceiptForTesting(expected);

  EXPECT_TRUE(sub.HasNewMessage());
  TestMsg got = sub.GetMessage();
  EXPECT_EQ(got.value, 42);
  EXPECT_FALSE(sub.HasNewMessage());  // GetMessage() must clear the flag
}

TEST(subscriber_impl, is_alive_reflects_backend_state) {
  carla::ros2::SubscriberImpl<TestSubTraits> sub;
  auto* raw_mock = new MockSubscriberBackend();
  raw_mock->alive = true;
  sub.SetBackendForTesting(std::unique_ptr<carla::ros2::IDDSSubscriberBackend>(raw_mock));

  EXPECT_TRUE(sub.IsAlive());
}

// ===========================================================================
// Group 7: IsBackendAvailable() — compile-time availability checks
//
// In TESTING mode, neither CARLA_ROS2_DDS_FASTDDS nor CARLA_ROS2_DDS_CYCLONEDDS
// is defined, so both backends report as unavailable.
// ===========================================================================

TEST_F(DDSBackendFactoryFixture, is_backend_available_fastdds_returns_false_in_testing_mode) {
  EXPECT_FALSE(carla::ros2::DDSBackendFactory::IsBackendAvailable(carla::ros2::DDSBackend::FastDDS));
}

TEST_F(DDSBackendFactoryFixture, is_backend_available_cyclonedds_returns_false_in_testing_mode) {
  EXPECT_FALSE(carla::ros2::DDSBackendFactory::IsBackendAvailable(carla::ros2::DDSBackend::CycloneDDS));
}

TEST_F(DDSBackendFactoryFixture, is_backend_available_is_independent_of_active_backend) {
  // Setting the active backend to CycloneDDS does NOT make it "available" —
  // availability is a compile-time property, selection is runtime.
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_FALSE(carla::ros2::DDSBackendFactory::IsBackendAvailable(carla::ros2::DDSBackend::CycloneDDS));
}

// ===========================================================================
// Group 8: ResolveBackend() — strict validation (no fallback)
//
// In TESTING mode no backends are available, so ResolveBackend() always fails.
// The success path is implicitly tested in production builds where
// CARLA_ROS2_DDS_FASTDDS (or CYCLONEDDS) is defined.
// ResolveBackend() never falls back to a different backend — if the requested
// backend is not compiled in, it returns {false, requested}.
// ===========================================================================

TEST_F(DDSBackendFactoryFixture, resolve_backend_fails_when_no_backend_available) {
  auto result = carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::FastDDS);
  EXPECT_FALSE(result.success);
  EXPECT_EQ(result.backend, carla::ros2::DDSBackend::FastDDS);
}

TEST_F(DDSBackendFactoryFixture, resolve_backend_fails_for_cyclonedds_when_no_backend_available) {
  auto result = carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_FALSE(result.success);
  EXPECT_EQ(result.backend, carla::ros2::DDSBackend::CycloneDDS);
}

TEST_F(DDSBackendFactoryFixture, resolve_backend_preserves_requested_backend_on_failure) {
  // When no backend is available, the returned backend must match the requested one.
  auto fast = carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::FastDDS);
  auto cycl = carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_EQ(fast.backend, carla::ros2::DDSBackend::FastDDS);
  EXPECT_EQ(cycl.backend, carla::ros2::DDSBackend::CycloneDDS);
}

TEST_F(DDSBackendFactoryFixture, resolve_backend_result_struct_fields_are_correct) {
  // Validate that BackendResolution struct fields are accessible and have expected types.
  carla::ros2::DDSBackendFactory::BackendResolution result =
      carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::FastDDS);
  bool success_val = result.success;
  carla::ros2::DDSBackend backend_val = result.backend;
  EXPECT_FALSE(success_val);
  EXPECT_EQ(backend_val, carla::ros2::DDSBackend::FastDDS);
}

TEST_F(DDSBackendFactoryFixture, resolve_backend_does_not_fallback_to_other_backend) {
  // Strict mode: requesting an unavailable backend must NOT silently switch
  // to another backend. It must fail with the originally requested backend.
  auto fast = carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::FastDDS);
  EXPECT_FALSE(fast.success);
  EXPECT_EQ(fast.backend, carla::ros2::DDSBackend::FastDDS);

  auto cycl = carla::ros2::DDSBackendFactory::ResolveBackend(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_FALSE(cycl.success);
  EXPECT_EQ(cycl.backend, carla::ros2::DDSBackend::CycloneDDS);

  // Crucially: even though we tried FastDDS first, CycloneDDS resolution must
  // NOT have been influenced by the prior call — each call is independent.
}

// ===========================================================================
// Group 9: DDSBackendToString — enum-to-string conversion
// ===========================================================================

TEST(dds_backend_to_string, fastdds_returns_correct_name) {
  EXPECT_STREQ(carla::ros2::DDSBackendToString(carla::ros2::DDSBackend::FastDDS), "FastDDS");
}

TEST(dds_backend_to_string, cyclonedds_returns_correct_name) {
  EXPECT_STREQ(carla::ros2::DDSBackendToString(carla::ros2::DDSBackend::CycloneDDS), "CycloneDDS");
}

TEST(dds_backend_to_string, both_values_produce_non_empty_strings) {
  // Guard against future enum additions returning empty or null.
  const char* fast = carla::ros2::DDSBackendToString(carla::ros2::DDSBackend::FastDDS);
  const char* cycl = carla::ros2::DDSBackendToString(carla::ros2::DDSBackend::CycloneDDS);
  EXPECT_NE(fast, nullptr);
  EXPECT_NE(cycl, nullptr);
  EXPECT_GT(strlen(fast), 0u);
  EXPECT_GT(strlen(cycl), 0u);
}

TEST(dds_backend_to_string, values_are_distinct) {
  // FastDDS and CycloneDDS must produce different strings.
  EXPECT_STRNE(
    carla::ros2::DDSBackendToString(carla::ros2::DDSBackend::FastDDS),
    carla::ros2::DDSBackendToString(carla::ros2::DDSBackend::CycloneDDS));
}

// ===========================================================================
// Group 10: GetAvailableBackendsString — compile-time backend list
//
// In TESTING mode, neither CARLA_ROS2_DDS_FASTDDS nor CARLA_ROS2_DDS_CYCLONEDDS
// is defined, so the list must be "none".
// ===========================================================================

TEST_F(DDSBackendFactoryFixture, get_available_backends_string_returns_none_in_testing_mode) {
  std::string available = carla::ros2::DDSBackendFactory::GetAvailableBackendsString();
  EXPECT_EQ(available, "none");
}

TEST_F(DDSBackendFactoryFixture, get_available_backends_string_does_not_contain_fastdds_in_testing_mode) {
  std::string available = carla::ros2::DDSBackendFactory::GetAvailableBackendsString();
  EXPECT_EQ(available.find("FastDDS"), std::string::npos);
}

TEST_F(DDSBackendFactoryFixture, get_available_backends_string_does_not_contain_cyclonedds_in_testing_mode) {
  std::string available = carla::ros2::DDSBackendFactory::GetAvailableBackendsString();
  EXPECT_EQ(available.find("CycloneDDS"), std::string::npos);
}

TEST_F(DDSBackendFactoryFixture, get_available_backends_string_is_independent_of_active_backend) {
  // Setting the active backend does NOT make it "available" — availability is compile-time.
  carla::ros2::DDSBackendFactory::SetBackend(carla::ros2::DDSBackend::CycloneDDS);
  std::string available = carla::ros2::DDSBackendFactory::GetAvailableBackendsString();
  EXPECT_EQ(available, "none");
}

// ===========================================================================
// Group 11: DDSBackendFromString — string-to-enum parsing with validation
//
// Tests the parsing function that converts user-supplied strings
// (e.g. from --dds-backend=<value>) into DDSBackend enum values.
// This function is used by CarlaEngine to validate CLI input.
// ===========================================================================

TEST(dds_backend_from_string, fastdds_is_valid) {
  auto result = carla::ros2::DDSBackendFromString("fastdds");
  EXPECT_TRUE(result.valid);
  EXPECT_EQ(result.backend, carla::ros2::DDSBackend::FastDDS);
}

TEST(dds_backend_from_string, cyclonedds_is_valid) {
  auto result = carla::ros2::DDSBackendFromString("cyclonedds");
  EXPECT_TRUE(result.valid);
  EXPECT_EQ(result.backend, carla::ros2::DDSBackend::CycloneDDS);
}

TEST(dds_backend_from_string, unrecognized_value_returns_invalid) {
  auto result = carla::ros2::DDSBackendFromString("otherdds");
  EXPECT_FALSE(result.valid);
}

TEST(dds_backend_from_string, empty_string_returns_invalid) {
  auto result = carla::ros2::DDSBackendFromString("");
  EXPECT_FALSE(result.valid);
}

TEST(dds_backend_from_string, case_sensitive_rejects_uppercase) {
  // CarlaSettings.cpp already lowercases the input, but the parser
  // itself should not accept mixed/upper case.
  auto result = carla::ros2::DDSBackendFromString("FastDDS");
  EXPECT_FALSE(result.valid);
}

TEST(dds_backend_from_string, rejects_partial_match) {
  auto r1 = carla::ros2::DDSBackendFromString("fast");
  EXPECT_FALSE(r1.valid);
  auto r2 = carla::ros2::DDSBackendFromString("cyclone");
  EXPECT_FALSE(r2.valid);
}

// ===========================================================================
// Group 12: ToROS2DDSTypeName — DDS type name mangling for ROS2 interop
//
// Verifies that fastddsgen-style type names are correctly converted to the
// ROS2 rosidl DDS mangled format ("pkg::msg::dds_::Type_").
// ===========================================================================

TEST(to_ros2_dds_type_name, no_namespace) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("Image"), "dds_::Image_");
}

TEST(to_ros2_dds_type_name, two_level_namespace) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("std_msgs::msg::Header"),
            "std_msgs::msg::dds_::Header_");
}

TEST(to_ros2_dds_type_name, three_level_sensor_msgs) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("sensor_msgs::msg::Image"),
            "sensor_msgs::msg::dds_::Image_");
}

TEST(to_ros2_dds_type_name, builtin_interfaces) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("builtin_interfaces::msg::Time"),
            "builtin_interfaces::msg::dds_::Time_");
}

TEST(to_ros2_dds_type_name, carla_custom_type) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("carla_msgs::msg::CarlaCollisionEvent"),
            "carla_msgs::msg::dds_::CarlaCollisionEvent_");
}

TEST(to_ros2_dds_type_name, empty_string) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName(""), "dds_::_");
}

TEST(to_ros2_dds_type_name, single_colon_no_double_colon) {
  // No "::" found, so the entire string is treated as an unnamespaced type.
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("foo:bar"), "dds_::foo:bar_");
}

TEST(to_ros2_dds_type_name, trailing_colons_uses_last_separator) {
  // rfind finds the last "::" in "a::b::c::D"
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("a::b::c::D"), "a::b::c::dds_::D_");
}

TEST(to_ros2_dds_type_name, leading_colons) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("::Foo"), "::dds_::Foo_");
}

TEST(to_ros2_dds_type_name, deep_nesting) {
  EXPECT_EQ(carla::ros2::ToROS2DDSTypeName("a::b::c::d::E"), "a::b::c::d::dds_::E_");
}
