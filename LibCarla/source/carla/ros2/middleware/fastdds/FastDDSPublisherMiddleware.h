// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/middleware/IPublisherMiddleware.h"
#include "carla/ros2/middleware/fastdds/FastDDSSharedParticipant.h"
#include "carla/ros2/middleware/fastdds/GenericCdrPubSubType.h"
#include "carla/ros2/types/UserDataFormat.h"
#include "carla/Logging.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>

#include <atomic>

namespace carla {
namespace ros2 {

namespace efd = eprosima::fastdds::dds;
using erc = eprosima::fastrtps::types::ReturnCode_t;

/// FastDDS implementation of IPublisherMiddleware.
/// Parameterized on a traits type T that provides:
///   T::msg_type  — the message type (a carla::ros2::msg::* POD struct)
/// Serialization is handled by GenericCdrPubSubType<msg_type> via CdrSerialization.h.
///
/// Uses FastDDSSharedParticipant for a single refcounted DomainParticipant
/// across all FastDDS endpoints, mirroring CycloneDDS's shared-participant
/// model.  This avoids the discovery storm that occurred when N independent
/// participants were destroyed back-to-back on ROS2::Shutdown().
///
/// Sets USER_DATA QoS on the DataWriter to the REP-2016 KV payload
/// "typehash=RIHS01_<hex>;" (PID_USER_DATA = 0x002c in DDSI-RTPS v2.5
/// §9.6.2.2.2) so that Jazzy rmw_cyclonedds_cpp and rmw_fastrtps_cpp can
/// perform REP-2011 type-hash-based endpoint matching without warning.
template<typename T>
class FastDDSPublisherMiddleware
    : public IPublisherMiddleware,
      public eprosima::fastdds::dds::DataWriterListener {
 public:
  using msg_type = typename T::msg_type;

  FastDDSPublisherMiddleware() = default;
  FastDDSPublisherMiddleware(const FastDDSPublisherMiddleware&) = delete;
  FastDDSPublisherMiddleware& operator=(const FastDDSPublisherMiddleware&) = delete;

  void on_publication_matched(
      efd::DataWriter* writer,
      const efd::PublicationMatchedStatus& info) override {
    _alive.store(info.current_count > 0, std::memory_order_relaxed);
  }

  ~FastDDSPublisherMiddleware() override {
    if (_datawriter) {
      // Detach listener before deletion so that on_publication_matched cannot
      // fire on a partially-destroyed object.
      _datawriter->set_listener(nullptr);
      _publisher->delete_datawriter(_datawriter);
    }
    if (_publisher) {
      _participant->delete_publisher(_publisher);
    }
    if (_topic) {
      _participant->delete_topic(_topic);
    }
    if (_participant) {
      // Release the per-type refcount so unregister_type() fires when the last
      // user of this TypeSupport goes away, before the shared participant drops.
      FastDDSSharedParticipant::release_type(_type->getName());
      FastDDSSharedParticipant::release();
      _participant = nullptr;
    }
  }

  bool Init(
      const std::string& topic_name,
      const PublisherQos& publisher_qos) override {
    if (_type == nullptr) {
      log_error("FastDDSPublisherMiddleware: Invalid TypeSupport");
      return false;
    }

    _participant = FastDDSSharedParticipant::acquire();
    if (_participant == nullptr) {
      log_error("FastDDSPublisherMiddleware: Shared participant unavailable");
      return false;
    }

    _type.register_type(_participant);
    FastDDSSharedParticipant::retain_type(_type->getName());

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _publisher = _participant->create_publisher(pubqos, nullptr);
    if (_publisher == nullptr) {
      log_error("FastDDSPublisherMiddleware: Failed to create Publisher");
      return false;
    }

    // Multiple endpoints may share the same topic on the shared participant
    // (e.g. every actor publishes on rt/tf). FastDDS's create_topic rejects a
    // duplicate name, so probe first and reuse via find_topic if it already
    // exists; find_topic increments an internal refcount that the matching
    // delete_topic in the destructor balances.
    if (_participant->lookup_topicdescription(topic_name) != nullptr) {
      _topic = _participant->find_topic(topic_name, eprosima::fastrtps::Duration_t{0, 0});
    } else {
      efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
      _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    }
    if (_topic == nullptr) {
      log_error("FastDDSPublisherMiddleware: Failed to create Topic");
      return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    // Keep-last writer history with the requested depth. The default depth 1
    // matches DATAWRITER_QOS_DEFAULT, so volatile publishers are unchanged.
    wqos.history().kind = efd::KEEP_LAST_HISTORY_QOS;
    wqos.history().depth =
        static_cast<int32_t>(publisher_qos.effective_history_depth());

    if (publisher_qos.durability == DurabilityKind::TransientLocal) {
      // Latched topic: the writer keeps the last history_depth samples and
      // hands them to late-joining subscribers that request transient_local.
      wqos.durability().kind = efd::TRANSIENT_LOCAL_DURABILITY_QOS;
    }

    if (publisher_qos.reliability == ReliabilityKind::BestEffort) {
      // High-rate sensor stream: drop samples instead of blocking the publish
      // call on retransmissions to slow subscribers. The default stays
      // RELIABLE (DATAWRITER_QOS_DEFAULT), unchanged for existing publishers.
      wqos.reliability().kind = efd::BEST_EFFORT_RELIABILITY_QOS;
    }

    // Set USER_DATA (PID_USER_DATA = 0x002c per OMG DDSI-RTPS v2.5 §9.6.2.2.2)
    // to the REP-2011/REP-2016 type-hash KV payload "typehash=RIHS01_<hex>;".
    // Jazzy rmw_cyclonedds_cpp / rmw_fastrtps_cpp parse this during SEDP
    // endpoint discovery to verify type compatibility.
    auto ud = build_user_data_for<msg_type>();
    if (!ud.empty()) {
      wqos.user_data().data_vec(ud);
    }

    efd::DataWriterListener* listener =
        static_cast<efd::DataWriterListener*>(this);
    _datawriter = _publisher->create_datawriter(_topic, wqos, listener);
    if (_datawriter == nullptr) {
      log_error("FastDDSPublisherMiddleware: Failed to create DataWriter");
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

  bool Publish(void* message_data) override {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    erc rcode = _datawriter->write(message_data, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
      return true;
    }
    log_error("FastDDSPublisherMiddleware::Publish (",
        _topic_name, ") failed with code:", rcode());
    return false;
  }

  bool IsAlive() const override {
    return _alive.load(std::memory_order_relaxed);
  }

  std::string GetTopicName() const override {
    return _topic_name;
  }

 private:
  efd::DomainParticipant* _participant { nullptr };
  efd::Publisher*         _publisher   { nullptr };
  efd::Topic*             _topic       { nullptr };
  efd::DataWriter*        _datawriter  { nullptr };
  efd::TypeSupport        _type        { new GenericCdrPubSubType<msg_type>() };

  std::string        _topic_name;
  std::atomic<bool>  _alive { false };
};

} // namespace ros2
} // namespace carla
