// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSSubscriberBackend.h"
#include "carla/ros2/dds/DDSBackend.h"
#include "carla/ros2/dds/cyclonedds/CycloneDDSTypeMap.h"
#include "carla/Logging.h"

// Cyclone DDS C API — available after Setup.sh installs cyclonedds 0.10.5.
// Uses the C core API (libddsc) directly. See CycloneDDSPublisherBackend.h for notes.

#include <dds/dds.h>

namespace carla {
namespace ros2 {

/// Cyclone DDS implementation of IDDSSubscriberBackend.
///
/// Parameterized on a traits type S that provides:
///   S::msg_type  — backend-neutral POD message struct
///
/// Native CycloneDDS types are resolved via CycloneDDSTypeMap<S::msg_type>.
/// On data arrival, the native C sample is converted to POD via from_cyclonedds().
template<typename S>
class CycloneDDSSubscriberBackend : public IDDSSubscriberBackend {
public:
  using msg_type  = typename S::msg_type;
  using type_map  = CycloneDDSTypeMap<msg_type>;
  using cdds_type = typename type_map::cdds_type;

  ~CycloneDDSSubscriberBackend() override {
    if (_listener != nullptr)
      dds_delete_listener(_listener);
    // Deleting the participant cascades to subscriber, topic and reader.
    if (_participant != DDS_HANDLE_NIL)
      dds_delete(_participant);
  }

  bool Init(const std::string& topic_name, void* message_ptr, bool* new_message_flag) override {
    _message_ptr     = static_cast<msg_type*>(message_ptr);
    _new_message_ptr = new_message_flag;

    // --- Participant ---
    _participant = dds_create_participant(DDS_DOMAIN_DEFAULT, nullptr, nullptr);
    if (_participant < 0) {
      log_error("CycloneDDSSubscriberBackend: failed to create participant: ",
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
      log_error("CycloneDDSSubscriberBackend: failed to create topic '", topic_name,
                "': ", dds_strretcode(-_topic));
      return false;
    }

    // --- Subscriber ---
    _subscriber = dds_create_subscriber(_participant, nullptr, nullptr);
    if (_subscriber < 0) {
      log_error("CycloneDDSSubscriberBackend: failed to create subscriber: ",
                dds_strretcode(-_subscriber));
      return false;
    }

    // --- QoS: reliable, keep-last-1 ---
    dds_qos_t* qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_MSECS(100));
    dds_qset_history(qos, DDS_HISTORY_KEEP_LAST, 1);

    // --- Listener: receive new-data notifications ---
    _listener = dds_create_listener(this);
    dds_lset_data_available(_listener, CycloneDDSSubscriberBackend::on_data_available);

    // --- DataReader ---
    _reader = dds_create_reader(_subscriber, _topic, qos, _listener);
    dds_delete_qos(qos);
    if (_reader < 0) {
      log_error("CycloneDDSSubscriberBackend: failed to create reader for '", topic_name,
                "': ", dds_strretcode(-_reader));
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

  bool IsAlive() const override {
    return _alive;
  }

  std::string GetTopicName() const override {
    return _topic_name;
  }

private:
  // Called by Cyclone DDS on the middleware thread whenever new data is available.
  // dds_take() returns native C-typed samples deserialized by the ops array.
  // We convert to POD via from_cyclonedds(), then return the samples.
  static void on_data_available(dds_entity_t reader, void* arg)
  {
    auto* self = static_cast<CycloneDDSSubscriberBackend<S>*>(arg);
    void* buf[1]          = { nullptr };
    dds_sample_info_t si[1];
    dds_return_t n = dds_take(reader, buf, si, 1, 1);
    if (n > 0) {
      if (si[0].valid_data && buf[0] != nullptr) {
        from_cyclonedds(*static_cast<cdds_type*>(buf[0]), *self->_message_ptr);
        *self->_new_message_ptr = true;
        self->_alive            = true;
      }
      dds_return_loan(reader, buf, n);
    }
  }

  dds_entity_t    _participant { DDS_HANDLE_NIL };
  dds_entity_t    _subscriber  { DDS_HANDLE_NIL };
  dds_entity_t    _topic       { DDS_HANDLE_NIL };
  dds_entity_t    _reader      { DDS_HANDLE_NIL };
  dds_listener_t* _listener    { nullptr };

  dds_topic_descriptor_t _desc_copy {};
  std::string _dds_type_name;

  msg_type* _message_ptr     { nullptr };
  bool*     _new_message_ptr { nullptr };

  std::string _topic_name;
  bool        _alive { false };
};

} // namespace ros2
} // namespace carla
