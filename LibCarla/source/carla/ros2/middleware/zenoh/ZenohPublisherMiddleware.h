// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/middleware/IPublisherMiddleware.h"
#include "carla/ros2/middleware/zenoh/ZenohSharedSession.h"
#include "carla/ros2/middleware/zenoh/ZenohWireFormat.h"
#include "carla/ros2/types/CdrSerialization.h"
#include "carla/ros2/types/CdrTopicInfo.h"
#include "carla/Logging.h"

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

#ifndef CARLA_ROS2_MIDDLEWARE_TESTING
#ifdef CARLA_ROS2_MIDDLEWARE_ZENOH

#include <zenoh.h>

namespace carla {
namespace ros2 {

/// Zenoh implementation of IPublisherMiddleware, compatible with rmw_zenoh.
///
/// Serializes msg::* POD structs to CDR via CdrSerialization.h and publishes
/// them on a keyexpr formatted per the rmw_zenoh design doc:
/// "<domain>/<topic>/<type>/<hash>".  A liveliness token is declared so the
/// endpoint shows up in rmw_zenoh's graph cache.
///
/// Parameterized on a traits type T that provides:
///   T::msg_type — the message type (a carla::ros2::msg::* POD struct)
template<typename T>
class ZenohPublisherMiddleware : public IPublisherMiddleware {
 public:
  using msg_type = typename T::msg_type;

  ZenohPublisherMiddleware() {
    // Gravestone owned handles so destructor's z_drop is safe if Init never ran.
    z_internal_null(&_publisher);
    z_internal_null(&_liveliness_token);
    z_internal_null(&_matching_listener);
  }
  ZenohPublisherMiddleware(const ZenohPublisherMiddleware&) = delete;
  ZenohPublisherMiddleware& operator=(const ZenohPublisherMiddleware&) = delete;

  ~ZenohPublisherMiddleware() override {
    // zenoh-c's z_drop on a publisher does not wait for in-flight z_publisher_put
    // calls (unlike FastDDS/CycloneDDS writers, which synchronize internally), so
    // serialize against Publish() to avoid a use-after-free when a render-thread
    // publish races with sensor teardown on the main thread.
    std::lock_guard<std::mutex> lock(_publish_mutex);
    z_drop(z_move(_matching_listener));
    z_drop(z_move(_liveliness_token));
    z_drop(z_move(_publisher));
  }

  bool Init(const std::string& topic_name) override {
    return InitImpl(topic_name, kZenohDefaultQos);
  }

  bool Init(const std::string& topic_name, const QosProfile& qos) override {
    // The profile is advertised in the liveliness token so rmw_zenoh peers see
    // a compatible endpoint; zenoh delivery semantics are not differentiated
    // (see zenoh_qos_keyexpr).
    return InitImpl(topic_name, zenoh_qos_keyexpr(qos));
  }

 private:
  bool InitImpl(const std::string& topic_name, const std::string& qos_str) {
    const z_loaned_session_t* session = zenoh_get_shared_session();

    const std::string topic_no_rt = zenoh_strip_rt_prefix(topic_name);
    const char* type_name = CdrTopicInfo<msg_type>::type_name();
    const char* type_hash = CdrTopicInfo<msg_type>::type_hash();
    if (type_hash == nullptr) {
      log_error("ZenohPublisherMiddleware: no type_hash for topic '",
                topic_name, "'");
      return false;
    }

    const std::string ke_str = zenoh_make_topic_keyexpr(
        zenoh_ros_domain_id(), topic_no_rt, type_name, type_hash);
    z_view_keyexpr_t ke;
    if (z_view_keyexpr_from_str(&ke, ke_str.c_str()) != Z_OK) {
      log_error("ZenohPublisherMiddleware (", topic_name,
                "): invalid keyexpr '", ke_str, "'");
      return false;
    }

    z_publisher_options_t pub_opts;
    z_publisher_options_default(&pub_opts);
    if (z_declare_publisher(session, &_publisher, z_loan(ke), &pub_opts)
        != Z_OK) {
      log_error("ZenohPublisherMiddleware (", topic_name,
                "): z_declare_publisher failed");
      return false;
    }

    const std::string lv_ke_str = zenoh_make_topic_liveliness_keyexpr(
        zenoh_ros_domain_id(), zenoh_session_zid(), zenoh_next_entity_id(),
        kZenohEntityKindPub, topic_no_rt, type_name, type_hash,
        qos_str.c_str());
    z_view_keyexpr_t lv_ke;
    if (z_view_keyexpr_from_str(&lv_ke, lv_ke_str.c_str()) != Z_OK) {
      log_error("ZenohPublisherMiddleware (", topic_name,
                "): invalid liveliness keyexpr");
      return false;
    }
    if (z_liveliness_declare_token(session, &_liveliness_token,
                                   z_loan(lv_ke), nullptr) != Z_OK) {
      log_error("ZenohPublisherMiddleware (", topic_name,
                "): z_liveliness_declare_token failed");
      return false;
    }

    z_owned_closure_matching_status_t closure;
    z_closure_matching_status(&closure, &on_matching_status, nullptr, this);
    if (z_publisher_declare_matching_listener(
            z_loan(_publisher), &_matching_listener, z_move(closure))
        != Z_OK) {
      log_error("ZenohPublisherMiddleware (", topic_name,
                "): z_publisher_declare_matching_listener failed");
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

 public:
  bool Publish(void* message_data) override {
    std::lock_guard<std::mutex> lock(_publish_mutex);
    if (!z_internal_check(_publisher)) {
      return false;
    }

    const msg_type* msg = static_cast<const msg_type*>(message_data);
    std::vector<uint8_t> cdr = serialize_to_cdr(*msg);

    z_owned_bytes_t payload;
    z_bytes_copy_from_buf(&payload, cdr.data(), cdr.size());

    const std::vector<uint8_t> attach = zenoh_attachment();
    z_owned_bytes_t attachment;
    z_bytes_copy_from_buf(&attachment, attach.data(), attach.size());

    z_publisher_put_options_t opts;
    z_publisher_put_options_default(&opts);
    opts.attachment = z_move(attachment);

    if (z_publisher_put(z_loan(_publisher), z_move(payload), &opts) != Z_OK) {
      log_error("ZenohPublisherMiddleware::Publish (", _topic_name,
                "): z_publisher_put failed");
      return false;
    }
    return true;
  }

  bool IsAlive() const override {
    return _alive.load(std::memory_order_relaxed);
  }

  std::string GetTopicName() const override {
    return _topic_name;
  }

 private:
  static void on_matching_status(const z_matching_status_t* status,
                                 void* context) {
    auto* self = static_cast<ZenohPublisherMiddleware*>(context);
    self->_alive.store(status->matching, std::memory_order_relaxed);
  }

  z_owned_publisher_t         _publisher;
  z_owned_liveliness_token_t  _liveliness_token;
  z_owned_matching_listener_t _matching_listener;

  mutable std::mutex _publish_mutex;
  std::atomic<bool>  _alive { false };
  std::string        _topic_name;
};

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_ZENOH
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
