// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSPublisherBackend.h"
#include "carla/ros2/dds/DDSBackend.h"
#include "carla/ros2/dds/cyclonedds/CycloneDDSTypeMap.h"
#include "carla/Logging.h"

// Cyclone DDS C API — available after Setup.sh installs cyclonedds 0.10.5.
// Uses the C core API (libddsc) directly: dds_entity_t, dds_write, dds_delete.
// cyclonedds-cxx (C++ binding) is NOT used because the UE4 toolchain disables
// exceptions which the C++ binding requires.

#include <dds/dds.h>

#include <cstring>  // memset (used by to_cyclonedds conversions)

namespace carla {
namespace ros2 {

/// Cyclone DDS implementation of IDDSPublisherBackend.
///
/// Parameterized on a traits type T that provides:
///   T::msg_type  — backend-neutral POD message struct
///
/// Native CycloneDDS types are resolved via CycloneDDSTypeMap<T::msg_type>.
/// Serialization uses the native dds_topic_descriptor_t (idlc-generated ops
/// array), producing identical OMG CDR bytes as FastDDS.
template<typename T>
class CycloneDDSPublisherBackend : public IDDSPublisherBackend {
public:
  using msg_type = typename T::msg_type;
  using type_map = CycloneDDSTypeMap<msg_type>;
  using cdds_type = typename type_map::cdds_type;

  ~CycloneDDSPublisherBackend() override {
    if (_listener != nullptr)
      dds_delete_listener(_listener);
    // Deleting the participant cascades to publisher, topic and writer.
    if (_participant != DDS_HANDLE_NIL)
      dds_delete(_participant);
  }

  bool Init(const std::string& topic_name) override {
    // --- Participant ---
    _participant = dds_create_participant(DDS_DOMAIN_DEFAULT, nullptr, nullptr);
    if (_participant < 0) {
      log_error("CycloneDDSPublisherBackend: failed to create participant: ",
                dds_strretcode(-_participant));
      return false;
    }

    // --- Topic with native descriptor ---
    // Patch the type name to use the ROS2 DDS mangling convention
    // (e.g. "sensor_msgs::msg::dds_::Image_") for interop with native ROS2 tools.
    const dds_topic_descriptor_t* desc = type_map::descriptor();
    _dds_type_name = ToROS2DDSTypeName(desc->m_typename);
    memcpy(&_desc_copy, desc, sizeof(dds_topic_descriptor_t));
    const char* name_ptr = _dds_type_name.c_str();
    memcpy(&_desc_copy.m_typename, &name_ptr, sizeof(const char*));
    _topic = dds_create_topic(
        _participant,
        &_desc_copy,
        topic_name.c_str(),
        nullptr, nullptr);
    if (_topic < 0) {
      log_error("CycloneDDSPublisherBackend: failed to create topic '", topic_name,
                "': ", dds_strretcode(-_topic));
      return false;
    }

    // --- Publisher ---
    _publisher = dds_create_publisher(_participant, nullptr, nullptr);
    if (_publisher < 0) {
      log_error("CycloneDDSPublisherBackend: failed to create publisher: ",
                dds_strretcode(-_publisher));
      return false;
    }

    // --- QoS: reliable delivery, keep-last-1 history ---
    dds_qos_t* qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS(100));
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);

    // --- Listener: track matched subscribers to update IsAlive() ---
    _listener = dds_create_listener(this);
    dds_lset_publication_matched(_listener, CycloneDDSPublisherBackend::on_publication_matched);

    // --- DataWriter ---
    _writer = dds_create_writer(_publisher, _topic, qos, _listener);
    dds_delete_qos(qos);
    if (_writer < 0) {
      log_error("CycloneDDSPublisherBackend: failed to create writer for '", topic_name,
                "': ", dds_strretcode(-_writer));
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

  bool Publish(void* message_data) override {
    if (_writer == DDS_HANDLE_NIL) {
      return false;
    }
    auto* pod = static_cast<msg_type*>(message_data);
    cdds_type cdds_msg;
    memset(&cdds_msg, 0, sizeof(cdds_type));
    to_cyclonedds(*pod, cdds_msg);
    dds_return_t rc = dds_write(_writer, &cdds_msg);
    // Free strings/sequences allocated by to_cyclonedds, but not the stack struct itself
    dds_sample_free(&cdds_msg, type_map::descriptor(), DDS_FREE_CONTENTS);
    if (rc < 0) {
      log_error("CycloneDDSPublisherBackend::Publish ('", _topic_name,
                "') failed: ", dds_strretcode(-rc));
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
  // Called by Cyclone DDS when the number of matched subscribers changes.
  static void on_publication_matched(
      dds_entity_t /*writer*/,
      const dds_publication_matched_status_t status,
      void* arg)
  {
    auto* self = static_cast<CycloneDDSPublisherBackend<T>*>(arg);
    self->_alive = (status.total_count > 0);
  }

  dds_entity_t   _participant { DDS_HANDLE_NIL };
  dds_entity_t   _publisher   { DDS_HANDLE_NIL };
  dds_entity_t   _topic       { DDS_HANDLE_NIL };
  dds_entity_t   _writer      { DDS_HANDLE_NIL };
  dds_listener_t* _listener   { nullptr };

  dds_topic_descriptor_t _desc_copy {};
  std::string _dds_type_name;
  std::string _topic_name;
  bool        _alive { false };
};

} // namespace ros2
} // namespace carla
