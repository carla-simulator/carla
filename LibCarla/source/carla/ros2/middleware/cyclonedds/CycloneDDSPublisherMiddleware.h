// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/middleware/IPublisherMiddleware.h"
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

/// CycloneDDS implementation of IPublisherMiddleware.
///
/// Serializes msg::* POD structs to CDR via CdrSerialization.h and
/// publishes them as raw CDR bytes using dds_writecdr().  No IDL-generated
/// type files or CycloneDDS C++ bindings are required.
///
/// Parameterized on a traits type T that provides:
///   T::msg_type  — the message type (a carla::ros2::msg::* POD struct)
template<typename T>
class CycloneDDSPublisherMiddleware : public IPublisherMiddleware {
 public:
  using msg_type = typename T::msg_type;

  CycloneDDSPublisherMiddleware() = default;
  CycloneDDSPublisherMiddleware(const CycloneDDSPublisherMiddleware&) = delete;
  CycloneDDSPublisherMiddleware& operator=(const CycloneDDSPublisherMiddleware&) = delete;

  ~CycloneDDSPublisherMiddleware() override {
    // Delete only our own entities. The shared participant is NOT deleted here;
    // it lives for the process lifetime so that sertypes interned by CycloneDDS
    // are never freed while other publishers are still using them.
    if (_writer > 0) {
      // Detach the listener before deleting the writer.  dds_set_listener with
      // a null listener acquires the entity lock and waits for any in-flight
      // carla_on_publication_matched callback to complete before returning,
      // preventing a use-after-free of `this` on the CycloneDDS receive thread.
      dds_set_listener(_writer, nullptr);
      dds_delete(_writer);
    }
    if (_topic  > 0) { dds_delete(_topic);  }
  }

  bool Init(const std::string& topic_name) override {
    dds_entity_t participant = carla_cdr_get_participant();
    if (participant < 0) {
      log_error("CycloneDDSPublisherMiddleware: Shared participant unavailable "
                "(topic '", topic_name, "', code:", participant, ")");
      return false;
    }

    const char* type_name = CdrTopicInfo<msg_type>::type_name();
    _topic = carla_cdr_create_topic(
        participant, topic_name.c_str(), type_name, &_sertype);
    if (_topic < 0) {
      log_error("CycloneDDSPublisherMiddleware: Failed to create topic '",
                topic_name, "' (code:", _topic, ")");
      return false;
    }

    dds_qos_t* qos = dds_create_qos();
    // Reliable delivery with keep-last history depth 1, matching the FastDDS
    // publisher defaults so both middlewares behave identically. Per-topic QoS
    // tuning (best-effort sensor streams, latched topics) arrives with the
    // PublisherQos layer in a later PR of this series.
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);
    // Set USER_DATA (PID_USER_DATA = 0x002c per OMG DDSI-RTPS v2.5 §9.6.2.2.2)
    // to the REP-2016 type-hash KV payload "typehash=RIHS01_<hex>;".
    auto ud = build_user_data_for<msg_type>();
    if (!ud.empty()) {
      dds_qset_userdata(qos, ud.data(), ud.size());
    }
    dds_listener_t* listener = dds_create_listener(this);
    dds_lset_publication_matched(listener, carla_on_publication_matched);

    _writer = dds_create_writer(participant, _topic, qos, listener);
    dds_delete_listener(listener);
    dds_delete_qos(qos);
    if (_writer < 0) {
      log_error("CycloneDDSPublisherMiddleware: Failed to create writer "
                "(topic '", topic_name, "', code:", _writer, ")");
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

  bool Publish(void* message_data) override {
    const msg_type* msg = static_cast<const msg_type*>(message_data);
    std::vector<uint8_t> cdr = serialize_to_cdr(*msg);

    struct ddsi_serdata* sd = carla_cdr_wrap(
        _sertype,
        cdr.data(),
        static_cast<uint32_t>(cdr.size()));
    if (!sd) {
      log_error("CycloneDDSPublisherMiddleware::Publish (", _topic_name,
                "): allocation failed");
      return false;
    }

    // dds_writecdr() consumes exactly 1 reference from sd on success
    // (see dds_write.c: "consumes 1 refc from din in all paths").
    // On failure the reference is NOT consumed, so we must unref manually.
    dds_return_t rc = dds_writecdr(_writer, sd);
    if (rc != DDS_RETCODE_OK) {
      ddsi_serdata_unref(sd);
      log_error("CycloneDDSPublisherMiddleware::Publish (", _topic_name,
                "): dds_writecdr failed (code:", rc, ")");
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
  static void carla_on_publication_matched(
      dds_entity_t /*writer*/,
      const dds_publication_matched_status_t status,
      void* arg)
  {
    CycloneDDSPublisherMiddleware* self =
        static_cast<CycloneDDSPublisherMiddleware*>(arg);
    self->_alive.store(status.current_count > 0, std::memory_order_relaxed);
  }

  dds_entity_t          _topic   { DDS_ENTITY_NIL };
  dds_entity_t          _writer  { DDS_ENTITY_NIL };
  struct ddsi_sertype*  _sertype { nullptr };

  std::string        _topic_name;
  std::atomic<bool>  _alive { false };
};

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_MIDDLEWARE_CYCLONEDDS
#endif // !CARLA_ROS2_MIDDLEWARE_TESTING
