// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef CARLA_ROS2_DDS_TESTING

#include "carla/ros2/dds/fastdds/FastDDSSharedParticipant.h"
#include "carla/Logging.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

namespace carla {
namespace ros2 {

namespace efd = eprosima::fastdds::dds;

// Static member definitions
std::mutex                                      FastDDSSharedParticipant::_mutex;
efd::DomainParticipant*                         FastDDSSharedParticipant::_participant { nullptr };
uint32_t                                        FastDDSSharedParticipant::_refcount { 0u };
std::unordered_map<std::string, uint32_t>       FastDDSSharedParticipant::_type_refcounts;

efd::DomainParticipant* FastDDSSharedParticipant::acquire() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_refcount == 0u) {
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    _participant =
        efd::DomainParticipantFactory::get_instance()->create_participant(0, pqos);
    if (_participant == nullptr) {
      log_error("FastDDSSharedParticipant: Failed to create DomainParticipant");
      return nullptr;
    }
  }
  ++_refcount;
  return _participant;
}

void FastDDSSharedParticipant::release() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_refcount == 0u) {
    log_error("FastDDSSharedParticipant::release() called with refcount already 0");
    return;
  }
  --_refcount;
  if (_refcount == 0u && _participant != nullptr) {
    // delete_contained_entities() drains all Topics and TypeSupports still
    // owned by the participant before the participant itself is deleted.
    // Without this call, TypeSupport objects registered with register_type()
    // but not explicitly unregistered could be leaked or double-freed.
    _participant->delete_contained_entities();
    efd::DomainParticipantFactory::get_instance()->delete_participant(_participant);
    _participant = nullptr;
    _type_refcounts.clear();
  }
}

void FastDDSSharedParticipant::retain_type(const std::string& type_name) {
  std::lock_guard<std::mutex> lock(_mutex);
  ++_type_refcounts[type_name];
}

void FastDDSSharedParticipant::release_type(const std::string& type_name) {
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = _type_refcounts.find(type_name);
  if (it == _type_refcounts.end() || it->second == 0u) {
    return;
  }
  --it->second;
  if (it->second == 0u) {
    // Unregister the type from the shared participant so the TypeSupport
    // refcount inside FastDDS reaches zero cleanly.
    if (_participant != nullptr) {
      _participant->unregister_type(type_name);
    }
    _type_refcounts.erase(it);
  }
}

} // namespace ros2
} // namespace carla

#endif // !CARLA_ROS2_DDS_TESTING
