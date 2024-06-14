// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "builtin_interfaces/msg/Time.h"
#include "carla/ros2/impl/DdsDomainParticipantImpl.h"
#include "carla/ros2/impl/DdsQoS.h"
#include "carla/ros2/impl/DdsReturnCode.h"
#include "carla/ros2/publishers/PublisherInterface.h"

namespace carla {
namespace ros2 {

template <typename MESSAGE_TYPE, typename MESSAGE_PUB_TYPE>
class DdsPublisherImpl : public PublisherInterface, eprosima::fastdds::dds::DataWriterListener {
public:
  DdsPublisherImpl() = default;

  virtual ~DdsPublisherImpl() {
    carla::log_info("DdsPublisherImpl[", _topic != nullptr ? _topic->get_name() : "nulltopic", "]::Destructor()");
    if (_datawriter) {
      _publisher->delete_datawriter(_datawriter);
      _datawriter = nullptr;
    }

    if (_publisher) {
      _participant->delete_publisher(_publisher);
      _publisher = nullptr;
    }

    if (_topic) {
      _participant->delete_topic(_topic);
      _topic = nullptr;
    }
  }

  /**
   * Initialize with PREALLOCATED_WITH_REALLOC_MEMORY_MODE memory policy.
   * Use this initialization mode when dealing with larger sequence data types
   * See //https://github.com/eProsima/Fast-DDS/issues/2330 for details
   */
  bool InitHistoryPreallocatedWithReallocMemoryMode(std::shared_ptr<DdsDomainParticipantImpl> domain_participant,
                                                    std::string topic_name, ROS2QoS qos) {
    auto pubqos = PublisherQos(qos);
    auto wqos = DataWriterQos(qos);
    auto tqos = TopicQos(qos);
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    return InitInternal(domain_participant, topic_name, tqos, pubqos, wqos);
  }

  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant, std::string topic_name, ROS2QoS qos) {
    auto pubqos = PublisherQos(qos);
    auto wqos = DataWriterQos(qos);
    auto tqos = TopicQos(qos);
    return InitInternal(domain_participant, topic_name, tqos, pubqos, wqos);
  }

  bool Publish() override {
    if (_message_updated) {
      carla::log_debug("DdsPublisherImpl[", _topic->get_name(), "]::Publishing() updated message");
      eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
      auto rcode = _datawriter->write(&_message, instance_handle);
      if (rcode == eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
        _message_updated = false;
      } else {
        carla::log_error("DdsPublisherImpl[", _topic->get_name(), "]::Publish() Failed to write data; Error ",
                         std::to_string(rcode));
      }
      carla::log_debug("DdsPublisherImpl[", _topic->get_name(), "]::Publishing() done");
    }
    return !_message_updated;
  }

  /**
   * Mark the message as updated. This is required to ensure the Publish() call sends the message actually out.
   */
  void SetMessageUpdated() {
    _message_updated = true;
  }

  /**
   * If the last message was sent out or the message has never been set to updated, this returns \c true 
   * indicating the publisher to be able to overwrite the message.
   */
  bool WasMessagePublished() {
    return !_message_updated;
  }

  /**
   * Initialize the message header. This function is only valid if the message type provided supports a header!
   * Implicitly calls SetMessageUpdated() to mark the message to be updated, so that it is published by Publish().
   */
  void SetMessageHeader(const builtin_interfaces::msg::Time& stamp, const std::string& frame_id) {
    _message.header().stamp(stamp);
    _message.header().frame_id(frame_id);
    SetMessageUpdated();
  }

  /**
   * Access the message data
   */
  MESSAGE_TYPE& Message() {
    return _message;
  }

  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override {
    return _matched > 0;
  }

private:
  bool InitInternal(std::shared_ptr<DdsDomainParticipantImpl> domain_participant, std::string topic_name,
                    eprosima::fastdds::dds::TopicQos const& tqos, eprosima::fastdds::dds::PublisherQos const& pubqos,
                    eprosima::fastdds::dds::DataWriterQos const& wqos) {
    carla::log_info("DdsPublisherImpl[", topic_name, "]::Init()");

    if (_type == nullptr) {
      carla::log_error("DdsPublisherImpl::Init() Invalid TypeSupport");
      return false;
    }

    _participant = domain_participant->GetDomainParticipant();
    if (_participant == nullptr) {
      carla::log_error("DdsPublisherImpl[", _type->getName(), "]::Init() Invalid Participant");
      return false;
    }

    _type.register_type(_participant);

    _publisher = _participant->create_publisher(pubqos);
    if (_publisher == nullptr) {
      carla::log_error("DdsPublisherImpl[", _type->getName(), "]::Init() Failed to create Publisher");
      return false;
    }

    _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    if (_topic == nullptr) {
      carla::log_error("DdsPublisherImpl[", _type->getName(), "]::Init() Failed to create Topic for ", topic_name);
      return false;
    }

    eprosima::fastdds::dds::DataWriterListener* listener =
        static_cast<eprosima::fastdds::dds::DataWriterListener*>(this);
    _datawriter = _publisher->create_datawriter(_topic, wqos, listener);
    if (_datawriter == nullptr) {
      carla::log_error("DdsPublisherImpl[", _topic->get_name(), "]::Init() Failed to create DataWriter");
      return false;
    }
    return true;
  }

  void on_publication_matched(eprosima::fastdds::dds::DataWriter*,
                              const eprosima::fastdds::dds::PublicationMatchedStatus& info) override {
    _matched = info.current_count;
    carla::log_info("DdsPublisherImpl[", _topic->get_name(), "]::on_publication_matched(): ", _matched);
  }

  eprosima::fastdds::dds::DomainParticipant* _participant{nullptr};
  eprosima::fastdds::dds::Publisher* _publisher{nullptr};
  eprosima::fastdds::dds::Topic* _topic{nullptr};
  eprosima::fastdds::dds::DataWriter* _datawriter{nullptr};
  eprosima::fastdds::dds::TypeSupport _type{new MESSAGE_PUB_TYPE()};
  MESSAGE_TYPE _message{};
  int _matched{0};
  bool _message_updated{false};
};
}  // namespace ros2
}  // namespace carla
