// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>

#include "carla/Logging.h"
#include "carla/ros2/FastDDSAliases.h"

namespace carla {
namespace ros2 {

// PublisherImpl wraps the per-message-type FastDDS plumbing (DomainParticipant,
// Publisher, Topic, DataWriter, TypeSupport) into a single template parameterised by
// a Traits struct that exposes `msg_type` and `msg_pubsub_type` typedefs. Concrete
// publishers (CarlaIMUPublisher, future CarlaCameraPublisher, etc.) hold a
// std::shared_ptr<PublisherImpl<Traits>> rather than re-implementing the boilerplate
// each time. The class inherits FastDDS's DataWriterListener directly so the
// publication-matched callback updates the alive flag.
template <typename Traits>
class PublisherImpl : public efd::DataWriterListener {
public:
  using msg_type = typename Traits::msg_type;
  using msg_pubsub_type = typename Traits::msg_pubsub_type;

  void on_publication_matched(
      efd::DataWriter * /*writer*/,
      const efd::PublicationMatchedStatus &info) override {
    _alive.store(info.current_count > 0, std::memory_order_release);
  }

  ~PublisherImpl() override {
    if (_datawriter)
      _publisher->delete_datawriter(_datawriter);

    if (_publisher)
      _participant->delete_publisher(_publisher);

    if (_topic)
      _participant->delete_topic(_topic);

    if (_participant)
      efd::DomainParticipantFactory::get_instance()->delete_participant(_participant);
  }

  bool Init(std::string topic_name) {
    if (_type == nullptr) {
      log_error("PublisherImpl::Init invalid TypeSupport");
      return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    auto factory = efd::DomainParticipantFactory::get_instance();
    _participant = factory->create_participant(0, pqos);
    if (_participant == nullptr) {
      log_error("PublisherImpl::Init failed to create DomainParticipant");
      return false;
    }

    erc type_rcode = _type.register_type(_participant);
    if (type_rcode != erc::ReturnCodeValue::RETCODE_OK) {
      log_error("PublisherImpl::Init failed to register type with code:", type_rcode());
      return false;
    }

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _publisher = _participant->create_publisher(pubqos, nullptr);
    if (_publisher == nullptr) {
      log_error("PublisherImpl::Init failed to create Publisher");
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    if (_topic == nullptr) {
      log_error("PublisherImpl::Init failed to create Topic");
      return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener *listener = static_cast<efd::DataWriterListener *>(this);
    _datawriter = _publisher->create_datawriter(_topic, wqos, listener);
    if (_datawriter == nullptr) {
      log_error("PublisherImpl::Init failed to create DataWriter");
      return false;
    }

    _topic_name = std::move(topic_name);
    return true;
  }

  [[nodiscard]] const std::string &GetTopicName() const noexcept { return _topic_name; }

  [[nodiscard]] bool IsAlive() const noexcept { return _alive.load(std::memory_order_acquire); }

  msg_type *GetMessage() { return &_message; }

  bool Publish() {
    if (_datawriter == nullptr) {
      // A failed Init() leaves the publisher cached, so Publish() is invoked
      // once per frame for the lifetime of the sensor. Log this only once to
      // avoid flooding the server log every frame.
      if (!_init_error_logged) {
        log_error(
            "PublisherImpl::Publish (", _topic_name,
            ") called before successful Init(); suppressing further messages for this publisher.");
        _init_error_logged = true;
      }
      return false;
    }
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    erc rcode = _datawriter->write(&_message, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
      return true;
    }
    log_error("PublisherImpl::Publish (", _topic_name, ") failed with code:", rcode());
    return false;
  }

private:
  efd::DomainParticipant *_participant{nullptr};
  efd::Publisher *_publisher{nullptr};
  efd::Topic *_topic{nullptr};
  efd::DataWriter *_datawriter{nullptr};
  efd::TypeSupport _type{new msg_pubsub_type()};

  std::string _topic_name;
  std::atomic<bool> _alive{false};
  bool _init_error_logged{false};
  msg_type _message{};
};

}  // namespace ros2
}  // namespace carla
