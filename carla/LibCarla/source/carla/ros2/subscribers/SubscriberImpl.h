// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>

#include "carla/Logging.h"
#include "carla/ros2/FastDDSAliases.h"

namespace carla {
namespace ros2 {

// SubscriberImpl mirrors PublisherImpl on the subscriber side: it bundles the FastDDS
// DomainParticipant/Subscriber/Topic/DataReader plumbing into a template parameterised
// by a Traits struct that exposes `msg_type` and `msg_pubsub_type` typedefs. The
// class inherits FastDDS's DataReaderListener directly so on_data_available is a
// member; arriving messages flip _new_message true and the next ProcessMessages call
// on the owning BaseSubscriber drains them.
template <typename Traits>
class SubscriberImpl : public efd::DataReaderListener {
public:
  using msg_type = typename Traits::msg_type;
  using msg_pubsub_type = typename Traits::msg_pubsub_type;

  void on_subscription_matched(
      efd::DataReader * /*reader*/,
      const efd::SubscriptionMatchedStatus &info) override {
    _alive.store(info.current_count > 0, std::memory_order_release);
  }

  void on_data_available(efd::DataReader *reader) override {
    // FastDDS invokes this on a DDS listener thread. Take into a stack-local
    // sample first so the lock only spans the copy into _message, and so a
    // dispose/unregister notification (RETCODE_OK with !info.valid_data) does
    // not flip _new_message. The _new_message store is held inside the lock
    // so a concurrent GetMessage cannot race-clear the flag after we publish
    // a fresh sample.
    efd::SampleInfo info;
    msg_type sample{};
    erc rcode = reader->take_next_sample(&sample, &info);
    if (rcode != erc::ReturnCodeValue::RETCODE_OK) {
      log_error("SubscriberImpl::on_data_available (", _topic_name, ") failed with code:", rcode());
      return;
    }
    if (!info.valid_data) {
      return;
    }
    std::lock_guard<std::mutex> lock(_message_mutex);
    _message = std::move(sample);
    _new_message.store(true, std::memory_order_release);
  }

  ~SubscriberImpl() override {
    if (_datareader)
      _subscriber->delete_datareader(_datareader);

    if (_subscriber)
      _participant->delete_subscriber(_subscriber);

    if (_topic)
      _participant->delete_topic(_topic);

    if (_participant)
      efd::DomainParticipantFactory::get_instance()->delete_participant(_participant);
  }

  bool Init(std::string topic_name) {
    if (_type == nullptr) {
      log_error("SubscriberImpl::Init invalid TypeSupport");
      return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    auto factory = efd::DomainParticipantFactory::get_instance();
    _participant = factory->create_participant(0, pqos);
    if (_participant == nullptr) {
      log_error("SubscriberImpl::Init failed to create DomainParticipant");
      return false;
    }

    erc type_rcode = _type.register_type(_participant);
    if (type_rcode != erc::ReturnCodeValue::RETCODE_OK) {
      log_error("SubscriberImpl::Init failed to register type with code:", type_rcode());
      return false;
    }

    efd::SubscriberQos subqos = efd::SUBSCRIBER_QOS_DEFAULT;
    _subscriber = _participant->create_subscriber(subqos, nullptr);
    if (_subscriber == nullptr) {
      log_error("SubscriberImpl::Init failed to create Subscriber");
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    if (_topic == nullptr) {
      log_error("SubscriberImpl::Init failed to create Topic");
      return false;
    }

    efd::DataReaderQos rqos = efd::DATAREADER_QOS_DEFAULT;
    efd::DataReaderListener *listener = static_cast<efd::DataReaderListener *>(this);
    _datareader = _subscriber->create_datareader(_topic, rqos, listener);
    if (_datareader == nullptr) {
      log_error("SubscriberImpl::Init failed to create DataReader");
      return false;
    }

    _topic_name = std::move(topic_name);
    return true;
  }

  [[nodiscard]] const std::string &GetTopicName() const noexcept { return _topic_name; }

  [[nodiscard]] bool IsAlive() const noexcept { return _alive.load(std::memory_order_acquire); }

  msg_type GetMessage() {
    std::lock_guard<std::mutex> lock(_message_mutex);
    msg_type copy = _message;
    _new_message.store(false, std::memory_order_release);
    return copy;
  }

  [[nodiscard]] bool HasNewMessage() const noexcept {
    return _new_message.load(std::memory_order_acquire);
  }

private:
  efd::DomainParticipant *_participant{nullptr};
  efd::Subscriber *_subscriber{nullptr};
  efd::Topic *_topic{nullptr};
  efd::DataReader *_datareader{nullptr};
  efd::TypeSupport _type{new msg_pubsub_type()};

  std::string _topic_name;
  std::atomic<bool> _alive{false};
  std::atomic<bool> _new_message{false};
  std::mutex _message_mutex;
  msg_type _message{};
};

}  // namespace ros2
}  // namespace carla
