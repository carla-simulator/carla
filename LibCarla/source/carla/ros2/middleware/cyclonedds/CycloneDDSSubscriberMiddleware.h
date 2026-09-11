// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/middleware/ISubscriberMiddleware.h"
#include "carla/ros2/middleware/cyclonedds/CycloneDDSSertype.h"
#include "carla/ros2/types/CdrSerialization.h"
#include "carla/ros2/types/CdrTopicInfo.h"
#include "carla/ros2/types/UserDataFormat.h"
#include "carla/Logging.h"

#include <atomic>

#ifndef CARLA_ROS2_MIDDLEWARE_TESTING
#ifdef CARLA_ROS2_MIDDLEWARE_CYCLONEDDS

#include <dds/dds.h>
#include <dds/ddsi/ddsi_serdata.h>
#include <dds/ddsi/ddsi_sertype.h>

namespace carla {
namespace ros2 {

/// CycloneDDS implementation of ISubscriberMiddleware.
///
/// Receives raw CDR bytes via dds_takecdr() and deserializes them into the
/// caller-supplied msg::* POD struct via CdrSerialization.h.  No IDL-generated
/// type files or CycloneDDS C++ bindings are required.
///
/// Parameterized on a traits type S that provides:
///   S::msg_type  — the message type (a carla::ros2::msg::* POD struct)
template<typename S>
class CycloneDDSSubscriberMiddleware : public ISubscriberMiddleware {
 public:
  using msg_type = typename S::msg_type;

  CycloneDDSSubscriberMiddleware() = default;
  CycloneDDSSubscriberMiddleware(const CycloneDDSSubscriberMiddleware&) = delete;
  CycloneDDSSubscriberMiddleware& operator=(const CycloneDDSSubscriberMiddleware&) = delete;

  ~CycloneDDSSubscriberMiddleware() override {
    // Delete only our own entities. The shared participant is NOT deleted here;
    // it lives for the process lifetime so that sertypes interned by CycloneDDS
    // are never freed while other subscribers are still using them.
    if (_reader > 0) {
      // Detach the listener before deleting the reader.  dds_set_listener with
      // a null listener acquires the entity lock and waits for any in-flight
      // carla_on_data_available / carla_on_subscription_matched callback to
      // complete before returning, preventing a use-after-free of `this` on
      // the CycloneDDS receive thread.
      dds_set_listener(_reader, nullptr);
      dds_delete(_reader);
    }
    if (_topic  > 0) { dds_delete(_topic);  }
  }

  bool Init(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag) override
  {
    return InitImpl(topic_name, message_ptr, new_message_flag, nullptr);
  }

  bool Init(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag,
      const QosProfile& qos) override
  {
    return InitImpl(topic_name, message_ptr, new_message_flag, &qos);
  }

 private:
  /// Shared Init body. When @a qos_profile is non-null the profile overrides
  /// the RELIABLE / KEEP_LAST-1 reader defaults below; when null the defaults
  /// are kept, preserving the pre-QoS behavior for every existing subscriber.
  bool InitImpl(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag,
      const QosProfile* qos_profile)
  {
    _message_ptr     = static_cast<msg_type*>(message_ptr);
    _new_message_ptr = new_message_flag;

    dds_entity_t participant = carla_cdr_get_participant();
    if (participant < 0) {
      log_error("CycloneDDSSubscriberMiddleware: Shared participant unavailable "
                "(topic '", topic_name, "', code:", participant, ")");
      return false;
    }

    const char* type_name = CdrTopicInfo<msg_type>::type_name();
    _topic = carla_cdr_create_topic(
        participant, topic_name.c_str(), type_name, &_sertype);
    if (_topic < 0) {
      log_error("CycloneDDSSubscriberMiddleware: Failed to create topic '",
                topic_name, "' (code:", _topic, ")");
      return false;
    }

    dds_qos_t* qos = dds_create_qos();
    if (qos_profile != nullptr) {
      dds_qset_reliability(qos,
          qos_profile->reliability == QosProfile::Reliability::Reliable
              ? DDS_RELIABILITY_RELIABLE
              : DDS_RELIABILITY_BEST_EFFORT,
          DDS_SECS(1));
      dds_qset_durability(qos,
          qos_profile->durability == QosProfile::Durability::TransientLocal
              ? DDS_DURABILITY_TRANSIENT_LOCAL
              : DDS_DURABILITY_VOLATILE);
      dds_qset_history(qos,
          qos_profile->history == QosProfile::History::KeepLast
              ? DDS_HISTORY_KEEP_LAST
              : DDS_HISTORY_KEEP_ALL,
          qos_profile->history_depth);
    } else {
      dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
      dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);
    }
    // Set USER_DATA (PID_USER_DATA = 0x002c per OMG DDSI-RTPS v2.5 §9.6.2.2.2)
    // to the REP-2016 type-hash KV payload "typehash=RIHS01_<hex>;".
    auto ud = build_user_data_for<msg_type>();
    if (!ud.empty()) {
      dds_qset_userdata(qos, ud.data(), ud.size());
    }
    dds_listener_t* listener = dds_create_listener(this);
    dds_lset_subscription_matched(listener, carla_on_subscription_matched);
    dds_lset_data_available(listener, carla_on_data_available);

    _reader = dds_create_reader(participant, _topic, qos, listener);
    dds_delete_listener(listener);
    dds_delete_qos(qos);
    if (_reader < 0) {
      log_error("CycloneDDSSubscriberMiddleware: Failed to create reader "
                "(topic '", topic_name, "', code:", _reader, ")");
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

 public:
  bool IsAlive() const override {
    return _alive.load(std::memory_order_relaxed);
  }

  std::string GetTopicName() const override {
    return _topic_name;
  }

 private:
  static void carla_on_subscription_matched(
      dds_entity_t /*reader*/,
      const dds_subscription_matched_status_t status,
      void* arg)
  {
    CycloneDDSSubscriberMiddleware* self =
        static_cast<CycloneDDSSubscriberMiddleware*>(arg);
    self->_alive.store(status.current_count > 0, std::memory_order_relaxed);
  }

  static void carla_on_data_available(dds_entity_t reader, void* arg) {
    CycloneDDSSubscriberMiddleware* self =
        static_cast<CycloneDDSSubscriberMiddleware*>(arg);

    struct ddsi_serdata* sd = nullptr;
    dds_sample_info_t   info;
    dds_return_t rc = dds_takecdr(reader, &sd, 1u, &info, DDS_ANY_STATE);
    if (rc <= 0 || !sd) { return; }

    if (info.valid_data) {
      const uint8_t* data = carla_cdr_data(sd);
      const uint32_t size = carla_cdr_size(sd);
      if (!deserialize_from_cdr(data, static_cast<size_t>(size),
                                *self->_message_ptr)) {
        log_error("CycloneDDSSubscriberMiddleware::on_data_available (",
                  self->_topic_name, "): deserialization failed");
      } else {
        *self->_new_message_ptr = true;
      }
    }
    ddsi_serdata_unref(sd);
  }

  dds_entity_t          _topic   { DDS_ENTITY_NIL };
  dds_entity_t          _reader  { DDS_ENTITY_NIL };
  struct ddsi_sertype*  _sertype { nullptr };

  msg_type* _message_ptr     { nullptr };
  bool*     _new_message_ptr { nullptr };

  std::string        _topic_name;
  std::atomic<bool>  _alive { false };
};

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_CYCLONEDDS
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
