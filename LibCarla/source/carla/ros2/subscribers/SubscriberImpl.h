// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include "carla/ros2/subscribers/BaseSubscriber.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>

#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>

#include "carla/Logging.h"

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  template<typename S>
  class SubscriberImpl : public eprosima::fastdds::dds::DataReaderListener {
  public:
    using msg_type = typename S::msg_type;
    using msg_pubsub_type = typename S::msg_pubsub_type;

    efd::DomainParticipant* _participant { nullptr };
    efd::Subscriber* _subscriber { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataReader* _datareader { nullptr };
    efd::TypeSupport _type { new msg_pubsub_type() };

    void on_subscription_matched(efd::DataReader* reader, const efd::SubscriptionMatchedStatus& info) override {
      _alive = (info.total_count > 0) ? true : false;
    }

    void on_data_available(efd::DataReader* reader) override {
      efd::SampleInfo info;
      msg_type message;

      eprosima::fastrtps::types::ReturnCode_t rcode = reader->take_next_sample(&_message, &info);
      if (rcode == eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
        // TODO: Process messages directly.
        _new_message = true;
      } else {
        log_error("SubscriberImpl::on_data_available (", this->GetTopicName(), ") failed with code:", rcode());
      }
    }

    ~SubscriberImpl() {
      if (_datareader)
        _subscriber->delete_datareader(_datareader);

      if (_subscriber)
        _participant->delete_subscriber(_subscriber);

      if (_topic)
        _participant->delete_topic(_topic);

      if (_participant)
        efd::DomainParticipantFactory::get_instance()->delete_participant(_participant);
    }

    // bool Init(std::string topic_name, S *subscriber) {
    bool Init(std::string topic_name) {
      if (_type == nullptr) {
        log_error("Invalid TypeSupport");
        return false;
      }

      efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
      auto factory = efd::DomainParticipantFactory::get_instance();
      _participant = factory->create_participant(0, pqos);
      if (_participant == nullptr) {
        log_error("Failed to create DomainParticipant");
        return false;
      }
      _type.register_type(_participant);

      efd::SubscriberQos subqos = efd::SUBSCRIBER_QOS_DEFAULT;
      _subscriber = _participant->create_subscriber(subqos, nullptr);
      if (_subscriber == nullptr) {
        log_error("Failed to create Subscriber");
        return false;
      }

      efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
      _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
      if (_topic == nullptr) {
        log_error("Failed to create Topic");
        return false;
      }

      efd::DataReaderQos rqos = efd::DATAREADER_QOS_DEFAULT;
      efd::DataReaderListener* listener = (efd::DataReaderListener*)(this);
      _datareader = _subscriber->create_datareader(_topic, rqos, listener);
      if (_datareader == nullptr) {
        log_error("Failed to create DataReader");
        return false;
      }

      _topic_name = topic_name;

      // _subscriber = subscriber;
      return true;
    }

    std::string GetTopicName() {
      return _topic_name;
    }

    bool IsAlive() {
      return _alive;
    }

    msg_type GetMessage() {
      _new_message = false;
      return _message;
    }

    bool HasNewMessage() { return _new_message; }

  private:
    std::string _topic_name;

    bool _alive { false };
    bool _new_message { false };
    msg_type _message;
  };

}  // namespace ros2
}  // namespace carla
