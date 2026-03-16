// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSSubscriberBackend.h"
#include "carla/ros2/dds/DDSBackend.h"
#include "carla/ros2/dds/fastdds/FastDDSTypeMap.h"
#include "carla/Logging.h"

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

namespace carla {
namespace ros2 {

namespace efd = eprosima::fastdds::dds;
using erc = eprosima::fastrtps::types::ReturnCode_t;

/// Fast DDS implementation of IDDSSubscriberBackend.
/// Parameterized on traits type S that provides:
///   S::msg_type  — backend-neutral POD message struct
/// Native FastDDS types are resolved via FastDDSTypeMap<S::msg_type>.
template<typename S>
class FastDDSSubscriberBackend
    : public IDDSSubscriberBackend
    , public eprosima::fastdds::dds::DataReaderListener {
public:
  using msg_type             = typename S::msg_type;
  using type_map             = FastDDSTypeMap<msg_type>;
  using fastdds_type         = typename type_map::fastdds_type;
  using fastdds_pubsub_type  = typename type_map::fastdds_pubsub_type;

  void on_subscription_matched(efd::DataReader* /*reader*/, const efd::SubscriptionMatchedStatus& info) override {
    _alive = (info.total_count > 0);
  }

  void on_data_available(efd::DataReader* reader) override {
    efd::SampleInfo info;
    erc rcode = reader->take_next_sample(&_fastdds_msg, &info);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
      from_fastdds(_fastdds_msg, *_message_ptr);
      *_new_message_ptr = true;
    } else {
      log_error("FastDDSSubscriberBackend::on_data_available (", _topic_name, ") failed with code:", rcode());
    }
  }

  ~FastDDSSubscriberBackend() override {
    if (_datareader)
      _subscriber->delete_datareader(_datareader);

    if (_subscriber)
      _participant->delete_subscriber(_subscriber);

    if (_topic)
      _participant->delete_topic(_topic);

    if (_participant)
      efd::DomainParticipantFactory::get_instance()->delete_participant(_participant);
  }

  bool Init(const std::string& topic_name, void* message_ptr, bool* new_message_flag) override {
    _message_ptr     = static_cast<msg_type*>(message_ptr);
    _new_message_ptr = new_message_flag;

    if (_type == nullptr) {
      log_error("FastDDSSubscriberBackend: Invalid TypeSupport");
      return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    auto factory = efd::DomainParticipantFactory::get_instance();
    _participant = factory->create_participant(0, pqos);
    if (_participant == nullptr) {
      log_error("FastDDSSubscriberBackend: Failed to create DomainParticipant");
      return false;
    }
    std::string dds_type_name = ToROS2DDSTypeName(_type->getName());
    _type.register_type(_participant, dds_type_name);

    efd::SubscriberQos subqos = efd::SUBSCRIBER_QOS_DEFAULT;
    _subscriber = _participant->create_subscriber(subqos, nullptr);
    if (_subscriber == nullptr) {
      log_error("FastDDSSubscriberBackend: Failed to create Subscriber");
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    _topic = _participant->create_topic(topic_name, dds_type_name, tqos);
    if (_topic == nullptr) {
      log_error("FastDDSSubscriberBackend: Failed to create Topic");
      return false;
    }

    efd::DataReaderQos rqos = efd::DATAREADER_QOS_DEFAULT;
    efd::DataReaderListener* listener = static_cast<efd::DataReaderListener*>(this);
    _datareader = _subscriber->create_datareader(_topic, rqos, listener);
    if (_datareader == nullptr) {
      log_error("FastDDSSubscriberBackend: Failed to create DataReader");
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
  efd::DomainParticipant* _participant { nullptr };
  efd::Subscriber*        _subscriber  { nullptr };
  efd::Topic*             _topic       { nullptr };
  efd::DataReader*        _datareader  { nullptr };
  efd::TypeSupport        _type        { new fastdds_pubsub_type() };

  fastdds_type _fastdds_msg;
  msg_type*    _message_ptr     { nullptr };
  bool*        _new_message_ptr { nullptr };

  std::string _topic_name;
  bool        _alive { false };
};

} // namespace ros2
} // namespace carla
