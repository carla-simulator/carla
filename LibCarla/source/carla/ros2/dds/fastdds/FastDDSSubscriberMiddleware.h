// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/dds/IDDSSubscriberMiddleware.h"
#include "carla/ros2/dds/fastdds/FastDDSSharedParticipant.h"
#include "carla/ros2/dds/fastdds/GenericCdrPubSubType.h"
#include "carla/ros2/types/UserDataFormat.h"
#include "carla/Logging.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
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

#include <atomic>

namespace carla {
namespace ros2 {

namespace efd = eprosima::fastdds::dds;
using erc = eprosima::fastrtps::types::ReturnCode_t;

/// FastDDS implementation of IDDSSubscriberMiddleware.
/// Parameterized on traits type S that provides:
///   S::msg_type  — the message type (a carla::ros2::msg::* POD struct)
/// Deserialization is handled by GenericCdrPubSubType<msg_type> via CdrSerialization.h.
///
/// Uses FastDDSSharedParticipant for a single refcounted DomainParticipant
/// across all FastDDS endpoints.  See FastDDSPublisherMiddleware.h for the
template<typename S>
class FastDDSSubscriberMiddleware
    : public IDDSSubscriberMiddleware,
      public eprosima::fastdds::dds::DataReaderListener {
 public:
  using msg_type = typename S::msg_type;

  FastDDSSubscriberMiddleware() = default;
  FastDDSSubscriberMiddleware(const FastDDSSubscriberMiddleware&) = delete;
  FastDDSSubscriberMiddleware& operator=(const FastDDSSubscriberMiddleware&) = delete;

  void on_subscription_matched(
      efd::DataReader* reader,
      const efd::SubscriptionMatchedStatus& info) override {
    _alive.store(info.current_count > 0, std::memory_order_relaxed);
  }

  void on_data_available(efd::DataReader* reader) override {
    efd::SampleInfo info;
    erc rcode = reader->take_next_sample(_message_ptr, &info);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
      *_new_message_ptr = true;
    } else {
      log_error("FastDDSSubscriberMiddleware::on_data_available (",
          _topic_name, ") failed with code:", rcode());
    }
  }

  ~FastDDSSubscriberMiddleware() override {
    if (_datareader) {
      // Detach listener before deletion so that on_data_available cannot
      // fire on a partially-destroyed object.
      _datareader->set_listener(nullptr);
      _subscriber->delete_datareader(_datareader);
    }
    if (_subscriber) {
      _participant->delete_subscriber(_subscriber);
    }
    if (_topic) {
      _participant->delete_topic(_topic);
    }
    if (_participant) {
      FastDDSSharedParticipant::release_type(_type->getName());
      FastDDSSharedParticipant::release();
      _participant = nullptr;
    }
  }

  bool Init(
      const std::string& topic_name,
      void* message_ptr,
      bool* new_message_flag) override {
    _message_ptr     = static_cast<msg_type*>(message_ptr);
    _new_message_ptr = new_message_flag;

    if (_type == nullptr) {
      log_error("FastDDSSubscriberMiddleware: Invalid TypeSupport");
      return false;
    }

    _participant = FastDDSSharedParticipant::acquire();
    if (_participant == nullptr) {
      log_error("FastDDSSubscriberMiddleware: Shared participant unavailable");
      return false;
    }

    _type.register_type(_participant);
    FastDDSSharedParticipant::retain_type(_type->getName());

    efd::SubscriberQos subqos = efd::SUBSCRIBER_QOS_DEFAULT;
    _subscriber = _participant->create_subscriber(subqos, nullptr);
    if (_subscriber == nullptr) {
      log_error("FastDDSSubscriberMiddleware: Failed to create Subscriber");
      return false;
    }

    // Reuse an existing topic on the shared participant when present: FastDDS
    // rejects duplicate create_topic for the same name. find_topic bumps the
    // participant's topic refcount, balanced by delete_topic in the destructor.
    if (_participant->lookup_topicdescription(topic_name) != nullptr) {
      _topic = _participant->find_topic(topic_name, eprosima::fastrtps::Duration_t{0, 0});
    } else {
      efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
      _topic = _participant->create_topic(topic_name, _type->getName(), tqos);
    }
    if (_topic == nullptr) {
      log_error("FastDDSSubscriberMiddleware: Failed to create Topic");
      return false;
    }

    efd::DataReaderQos rqos = efd::DATAREADER_QOS_DEFAULT;

    // Set USER_DATA (PID_USER_DATA = 0x002c per OMG DDSI-RTPS v2.5 §9.6.2.2.2)
    // to the REP-2016 type-hash KV payload "typehash=RIHS01_<hex>;".
    auto ud = build_user_data_for<msg_type>();
    if (!ud.empty()) {
      rqos.user_data().data_vec(ud);
    }

    efd::DataReaderListener* listener =
        static_cast<efd::DataReaderListener*>(this);
    _datareader = _subscriber->create_datareader(_topic, rqos, listener);
    if (_datareader == nullptr) {
      log_error("FastDDSSubscriberMiddleware: Failed to create DataReader");
      return false;
    }

    _topic_name = topic_name;
    return true;
  }

  bool IsAlive() const override {
    return _alive.load(std::memory_order_relaxed);
  }

  std::string GetTopicName() const override {
    return _topic_name;
  }

 private:
  efd::DomainParticipant* _participant { nullptr };
  efd::Subscriber*        _subscriber  { nullptr };
  efd::Topic*             _topic       { nullptr };
  efd::DataReader*        _datareader  { nullptr };
  efd::TypeSupport        _type        { new GenericCdrPubSubType<msg_type>() };

  msg_type* _message_ptr     { nullptr };
  bool*     _new_message_ptr { nullptr };

  std::string        _topic_name;
  std::atomic<bool>  _alive { false };
};

} // namespace ros2
} // namespace carla
