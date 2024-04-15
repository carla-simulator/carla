// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "carla/Logging.h"
#include "carla/ros2/impl/DdsDomainParticipantImpl.h"
#include "carla/ros2/impl/DdsQoS.h"
#include "carla/ros2/impl/DdsReturnCode.h"
#include "carla/ros2/subscribers/SubscriberInterface.h"

namespace carla {
namespace ros2 {

template <typename MESSAGE_TYPE, typename MESSAGE_PUB_TYPE>
class DdsSubscriberImpl : public SubscriberInterface<MESSAGE_TYPE>, public eprosima::fastdds::dds::DataReaderListener {
public:
  DdsSubscriberImpl() = default;

  virtual ~DdsSubscriberImpl() {
    carla::log_info("DdsSubscriberImpl[", _topic->get_name(), "]::Destructor()");

    if (_datareader) {
      _subscriber->delete_datareader(_datareader);
      _datareader = nullptr;
    }

    if (_subscriber) {
      _participant->delete_subscriber(_subscriber);
      _subscriber = nullptr;
    }

    if (_topic) {
      _participant->delete_topic(_topic);
      _topic = nullptr;
    }
  }

  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant, std::string topic_name, ROS2QoS qos) {
    auto subqos = SubscriberQos(qos);
    auto rqos = DataReaderQos(qos);
    auto tqos = TopicQos(qos);
    return InitInternal(domain_participant, topic_name, tqos, subqos, rqos);
  }

  void on_subscription_matched(eprosima::fastdds::dds::DataReader*,
                               const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override {
    if ((_matched == 0u) && (info.total_count > 1)) {
      _first_connected = true;
    }
    _matched = info.total_count;
    if (_matched == 0) {
      carla::log_error("DdsSubscriberImpl[", _topic->get_name(),
                       "]::on_subscription_matched(): interface disconnected");
      _alive = false;
    } else if (!_alive) {
      carla::log_error("DdsSubscriberImpl[", _topic->get_name(),
                       "]::on_subscription_matched(): interface (re-)connected");
      _alive = true;
    }
  }

  void on_data_available(eprosima::fastdds::dds::DataReader* reader) override {
    eprosima::fastdds::dds::SampleInfo info;
    auto rcode = reader->take_next_sample(&_message, &info);
    if (rcode == eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
      _new_message = true;
    } else {
      carla::log_error("DdsSubscriberImpl[", _topic->get_name(), "]::on_data_available(): Error ",
                       std::to_string(rcode));
    }
  }

  /**
   * Implements SubscriberInterface::IsAlive() interface
   */
  bool IsAlive() const override {
    return _alive;
  }
  /**
   * Implements SubscriberInterface::HasNewMessage() interface
   */
  bool HasNewMessage() const override {
    return _new_message;
  }
  /**
   * Implements SubscriberInterface::GetMessage() interface
   */
  const MESSAGE_TYPE& GetMessage() override {
    _new_message = false;
    return _message;
  }

  bool InitInternal(std::shared_ptr<DdsDomainParticipantImpl> domain_participant, std::string topic_name,
                    eprosima::fastdds::dds::TopicQos const& tqos, eprosima::fastdds::dds::SubscriberQos const& subqos,
                    eprosima::fastdds::dds::DataReaderQos const& rqos) {
    carla::log_info("DdsSubscriberImpl[", topic_name, "]::Init()");

    _participant = domain_participant->GetDomainParticipant();
    if (_participant == nullptr) {
      carla::log_error("DdsSubscriberImpl[", topic_name, "]::Init(): Invalid Participant");
      return false;
    }

    if (_type == nullptr) {
      carla::log_error("DdsSubscriberImpl[", topic_name, "]::Init(): Invalid TypeSupport");
      return false;
    }

    _type.register_type(_participant);

    _subscriber = _participant->create_subscriber(subqos);
    if (_subscriber == nullptr) {
      carla::log_error("DdsSubscriberImpl[", topic_name, "]::Init(): Failed to create Subscriber");
      return false;
    }

    _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    if (_topic == nullptr) {
      carla::log_error("DdsSubscriberImpl[", topic_name, "]::Init(): Failed to create Topic");
      return false;
    }

    eprosima::fastdds::dds::DataReaderListener* listener =
        static_cast<eprosima::fastdds::dds::DataReaderListener*>(this);
    _datareader = _subscriber->create_datareader(_topic, rqos, listener);
    if (_datareader == nullptr) {
      carla::log_error("DdsSubscriberImpl[", topic_name, "]::Init(): Failed to create DataReader");
      return false;
    }
    return true;
  }

  eprosima::fastdds::dds::DomainParticipant* _participant{nullptr};
  eprosima::fastdds::dds::Subscriber* _subscriber{nullptr};
  eprosima::fastdds::dds::Topic* _topic{nullptr};
  eprosima::fastdds::dds::DataReader* _datareader{nullptr};
  eprosima::fastdds::dds::TypeSupport _type{new MESSAGE_PUB_TYPE()};

  MESSAGE_TYPE _message{};
  int _matched{0};
  bool _first_connected{false};
  bool _new_message{false};
  bool _alive{true};
};
}  // namespace ros2
}  // namespace carla
