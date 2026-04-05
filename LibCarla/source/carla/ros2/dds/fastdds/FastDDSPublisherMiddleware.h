// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSPublisherMiddleware.h"
#include "carla/ros2/dds/fastdds/GenericCdrPubSubType.h"
#include "carla/Logging.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
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

namespace carla {
namespace ros2 {

namespace efd = eprosima::fastdds::dds;
using erc = eprosima::fastrtps::types::ReturnCode_t;

/// FastDDS implementation of IDDSPublisherMiddleware.
/// Parameterized on a traits type T that provides:
///   T::msg_type  — the message type (a carla::ros2::msg::* POD struct)
/// Serialization is handled by GenericCdrPubSubType<msg_type> via CdrSerialization.h.
template<typename T>
class FastDDSPublisherMiddleware
    : public IDDSPublisherMiddleware,
      public eprosima::fastdds::dds::DataWriterListener {
 public:
  using msg_type = typename T::msg_type;

  void on_publication_matched(
      efd::DataWriter* writer,
      const efd::PublicationMatchedStatus& info) override {
    _alive = (info.total_count > 0);
  }

  ~FastDDSPublisherMiddleware() override {
    if (_datawriter) {
      _publisher->delete_datawriter(_datawriter);
    }
    if (_publisher) {
      _participant->delete_publisher(_publisher);
    }
    if (_topic) {
      _participant->delete_topic(_topic);
    }
    if (_participant) {
      efd::DomainParticipantFactory::get_instance()->delete_participant(_participant);
    }
  }

  bool Init(const std::string& topic_name) override {
    if (_type == nullptr) {
      log_error("FastDDSPublisherMiddleware: Invalid TypeSupport");
      return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    auto factory = efd::DomainParticipantFactory::get_instance();
    _participant = factory->create_participant(0, pqos);
    if (_participant == nullptr) {
      log_error("FastDDSPublisherMiddleware: Failed to create DomainParticipant");
      return false;
    }
    _type.register_type(_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _publisher = _participant->create_publisher(pubqos, nullptr);
    if (_publisher == nullptr) {
      log_error("FastDDSPublisherMiddleware: Failed to create Publisher");
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    if (_topic == nullptr) {
      log_error("FastDDSPublisherMiddleware: Failed to create Topic");
      return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
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
    return _alive;
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

  std::string _topic_name;
  bool        _alive { false };
};

} // namespace ros2
} // namespace carla
