// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/middleware/ISubscriberMiddleware.h"
#include "carla/ros2/middleware/zenoh/ZenohSharedSession.h"
#include "carla/ros2/middleware/zenoh/ZenohWireFormat.h"
#include "carla/ros2/types/CdrSerialization.h"
#include "carla/ros2/types/CdrTopicInfo.h"
#include "carla/Logging.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>

#ifndef CARLA_ROS2_MIDDLEWARE_TESTING
#ifdef CARLA_ROS2_MIDDLEWARE_ZENOH

#include <zenoh.h>

namespace carla {
namespace ros2 {

/// Zenoh implementation of ISubscriberMiddleware.
///
/// Declares a subscriber on the rmw_zenoh-compatible keyexpr
/// "<domain>/<topic>/<type>/<hash>". Each received sample is deserialized from CDR
/// directly into the caller-supplied msg::* POD struct (no extra copy), and
/// a liveliness token is declared so the endpoint appears in rmw_zenoh's
/// graph cache.
///
/// Parameterized on a traits type S that provides:
///   S::msg_type  — the message type (a carla::ros2::msg::* POD struct)
template<typename S>
class ZenohSubscriberMiddleware : public ISubscriberMiddleware {
 public:
  using msg_type = typename S::msg_type;

  ZenohSubscriberMiddleware() {
    // Gravestone owned handles so destructor's z_drop is safe if Init never ran.
    z_internal_null(&_subscriber);
    z_internal_null(&_liveliness_token);
  }
  ZenohSubscriberMiddleware(const ZenohSubscriberMiddleware&) = delete;
  ZenohSubscriberMiddleware& operator=(const ZenohSubscriberMiddleware&) = delete;

  ~ZenohSubscriberMiddleware() override {
    // zenoh-c's subscriber drop blocks until any in-flight callback returns,
    // so it is safe to let `*_message_ptr` go out of scope after this.
    z_drop(z_move(_liveliness_token));
    z_drop(z_move(_subscriber));
  }

  bool Init(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag) override
  {
    _message_ptr     = static_cast<msg_type*>(message_ptr);
    _new_message_ptr = new_message_flag;

    const z_loaned_session_t* session = zenoh_get_shared_session();

    const std::string topic_no_rt = zenoh_strip_rt_prefix(topic_name);
    const char* type_name = CdrTopicInfo<msg_type>::type_name();
    const char* type_hash = CdrTopicInfo<msg_type>::type_hash();
    if (type_hash == nullptr) {
      log_error("ZenohSubscriberMiddleware: no type_hash for topic '",
                topic_name, "'");
      return false;
    }

    const std::string ke_str = zenoh_make_topic_keyexpr(
        zenoh_ros_domain_id(), topic_no_rt, type_name, type_hash);
    z_view_keyexpr_t ke;
    if (z_view_keyexpr_from_str(&ke, ke_str.c_str()) != Z_OK) {
      log_error("ZenohSubscriberMiddleware (", topic_name,
                "): invalid keyexpr '", ke_str, "'");
      return false;
    }

    z_owned_closure_sample_t closure;
    z_closure_sample(&closure, &on_sample, nullptr, this);

    z_subscriber_options_t sub_opts;
    z_subscriber_options_default(&sub_opts);
    if (z_declare_subscriber(session, &_subscriber, z_loan(ke),
                             z_move(closure), &sub_opts) != Z_OK) {
      log_error("ZenohSubscriberMiddleware (", topic_name,
                "): z_declare_subscriber failed");
      return false;
    }

    const std::string lv_ke_str = zenoh_make_topic_liveliness_keyexpr(
        zenoh_ros_domain_id(), zenoh_session_zid(), zenoh_next_entity_id(),
        kZenohEntityKindSub, topic_no_rt, type_name, type_hash,
        kZenohDefaultQos);
    z_view_keyexpr_t lv_ke;
    if (z_view_keyexpr_from_str(&lv_ke, lv_ke_str.c_str()) != Z_OK) {
      log_error("ZenohSubscriberMiddleware (", topic_name,
                "): invalid liveliness keyexpr");
      return false;
    }
    if (z_liveliness_declare_token(session, &_liveliness_token,
                                   z_loan(lv_ke), nullptr) != Z_OK) {
      log_error("ZenohSubscriberMiddleware (", topic_name,
                "): z_liveliness_declare_token failed");
      return false;
    }

    // zenoh-c does not expose a publisher-matched signal on the subscriber
    // side, so mark alive once declared. The publisher-side IsAlive() in
    // ZenohPublisherMiddleware already gates wasted work.
    _alive.store(true, std::memory_order_relaxed);
    _topic_name = topic_name;
    return true;
  }

  bool IsAlive() const override {
    return _alive.load(std::memory_order_relaxed);
  }

  std::string GetTopicName() const override {
    return _topic_name;
  }

 private:
  static void on_sample(z_loaned_sample_t* sample, void* context) {
    auto* self = static_cast<ZenohSubscriberMiddleware*>(context);
    const z_loaned_bytes_t* payload = z_sample_payload(sample);
    z_owned_slice_t slice;
    if (z_bytes_to_slice(payload, &slice) != Z_OK) {
      log_error("ZenohSubscriberMiddleware (", self->_topic_name,
                "): z_bytes_to_slice failed");
      return;
    }
    const z_loaned_slice_t* ls = z_slice_loan(&slice);
    const uint8_t* data = z_slice_data(ls);
    const size_t   len  = z_slice_len(ls);
    if (!deserialize_from_cdr(data, len, *self->_message_ptr)) {
      log_error("ZenohSubscriberMiddleware (", self->_topic_name,
                "): deserialization failed");
    } else {
      *self->_new_message_ptr = true;
    }
    z_drop(z_move(slice));
  }

  z_owned_subscriber_t       _subscriber;
  z_owned_liveliness_token_t _liveliness_token;

  msg_type* _message_ptr     { nullptr };
  bool*     _new_message_ptr { nullptr };

  std::atomic<bool> _alive { false };
  std::string       _topic_name;
};

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_ZENOH
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
