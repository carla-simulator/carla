// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSPublisherMiddleware.h"
#include "carla/ros2/dds/cyclonedds/CycloneDDSSertype.h"
#include "carla/ros2/types/CdrSerialization.h"
#include "carla/ros2/types/CdrTopicInfo.h"
#include "carla/Logging.h"

#ifndef CARLA_ROS2_DDS_TESTING
#ifdef CARLA_ROS2_DDS_CYCLONEDDS

#include <dds/dds.h>
#include <dds/ddsi/ddsi_serdata.h>
#include <dds/ddsi/ddsi_sertype.h>

namespace carla {
namespace ros2 {

/// CycloneDDS implementation of IDDSPublisherMiddleware.
///
/// Serializes msg::* POD structs to CDR via CdrSerialization.h and
/// publishes them as raw CDR bytes using dds_writecdr().  No IDL-generated
/// type files or CycloneDDS C++ bindings are required.
///
/// Parameterized on a traits type T that provides:
///   T::msg_type  — the message type (a carla::ros2::msg::* POD struct)
template<typename T>
class CycloneDDSPublisherMiddleware : public IDDSPublisherMiddleware {
 public:
  using msg_type = typename T::msg_type;

  ~CycloneDDSPublisherMiddleware() override {
    if (_participant != DDS_ENTITY_NIL) {
      dds_delete(_participant);  // cascades to writer, topic, sertype
    }
  }

  bool Init(const std::string& topic_name) override {
    _participant = dds_create_participant(DDS_DOMAIN_DEFAULT, nullptr, nullptr);
    if (_participant < 0) {
      log_error("CycloneDDSPublisherMiddleware: Failed to create participant "
                "(topic '", topic_name, "', code:", _participant, ")");
      return false;
    }

    const char* type_name = CdrTopicInfo<msg_type>::type_name();
    _topic = carla_cdr_create_topic(
        _participant, topic_name.c_str(), type_name, &_sertype);
    if (_topic < 0) {
      log_error("CycloneDDSPublisherMiddleware: Failed to create topic '",
                topic_name, "' (code:", _topic, ")");
      return false;
    }

    dds_qos_t* qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE,
                         DDS_SECS(1));
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);
    dds_listener_t* listener = dds_create_listener(this);
    dds_lset_publication_matched(listener, carla_on_publication_matched);

    _writer = dds_create_writer(_participant, _topic, qos, listener);
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

    dds_return_t rc = dds_writecdr(_writer, sd);
    ddsi_serdata_unref(sd);
    if (rc != DDS_RETCODE_OK) {
      log_error("CycloneDDSPublisherMiddleware::Publish (", _topic_name,
                "): dds_writecdr failed (code:", rc, ")");
      return false;
    }
    return true;
  }

  bool IsAlive() const override {
    return _alive;
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
    self->_alive = (status.current_count > 0);
  }

  dds_entity_t          _participant { DDS_ENTITY_NIL };
  dds_entity_t          _topic       { DDS_ENTITY_NIL };
  dds_entity_t          _writer      { DDS_ENTITY_NIL };
  struct ddsi_sertype*  _sertype     { nullptr };

  std::string _topic_name;
  bool        _alive { false };
};

} // namespace ros2
} // namespace carla

#endif // CARLA_ROS2_DDS_CYCLONEDDS
#endif // !CARLA_ROS2_DDS_TESTING
