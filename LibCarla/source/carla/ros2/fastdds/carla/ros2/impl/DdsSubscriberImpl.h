// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <set>

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
#include "carla/ros2/subscribers/SubscriberImplBase.h"

namespace carla {
namespace ros2 {

template <typename MESSAGE_TYPE, typename MESSAGE_PUB_TYPE>
class DdsSubscriberImpl : public SubscriberImplBase<MESSAGE_TYPE>, public eprosima::fastdds::dds::DataReaderListener {
public:
  using SubscriberImplBase<MESSAGE_TYPE>::AddPublisher;
  using SubscriberImplBase<MESSAGE_TYPE>::NumberPublishersConnected;
  using SubscriberImplBase<MESSAGE_TYPE>::RemovePublisher;
  using SubscriberImplBase<MESSAGE_TYPE>::HasPublishersConnected;
  using SubscriberImplBase<MESSAGE_TYPE>::AddMessage;

  DdsSubscriberImpl(SubscriberBase<MESSAGE_TYPE>& parent) : SubscriberImplBase<MESSAGE_TYPE>(parent) {}

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

  void on_subscription_matched(eprosima::fastdds::dds::DataReader* reader,
                               const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override {
    carla::rpc::synchronization_client_id_type const client_id = GetClientId(info.last_publication_handle);
    bool had_connected_publisher = HasPublishersConnected();

    if (info.current_count_change < 0) {
      RemovePublisher(client_id);
      carla::log_debug("DdsSubscriberImpl[", _topic->get_name(), "]::on_subscription_matched(", client_id,
                       ") publisher disconnected. Connected publisher remaining: ", NumberPublishersConnected());
    } else {
      AddPublisher(client_id);
      carla::log_debug("DdsSubscriberImpl[", _topic->get_name(), "]::on_subscription_matched(", client_id,
                       ") publisher connected. Connected publisher: ", NumberPublishersConnected());
    }

    if (info.current_count != NumberPublishersConnected()) {
      carla::log_error("DdsSubscriberImpl[", _topic->get_name(), "]::on_subscription_matched(", client_id,
                       "): current_count=", info.current_count,
                       ", but publisher list not yet empty. Connected publisher: ", NumberPublishersConnected());
    }
    carla::log_debug("DdsSubscriberImpl[", _topic->get_name(), "]::on_subscription_matched(", client_id,
                     "): interface has"
                     " total_count=",
                     info.total_count, " total_count_change=", info.total_count_change,
                     " current_count=", info.current_count, " current_count_change=", info.current_count_change,
                     " handle-id=", info.last_publication_handle, " matched subscriptions and currently ",
                     NumberPublishersConnected(), " publisher connected.");
  }

  void on_data_available(eprosima::fastdds::dds::DataReader* reader) override {
    eprosima::fastdds::dds::SampleInfo info;
    MESSAGE_TYPE message;
    auto rcode = reader->take_next_sample(&message, &info);
    carla::rpc::synchronization_client_id_type const client_id = GetClientId(info.publication_handle);
    if (rcode == eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK) {
      AddMessage(client_id, message);
      carla::log_debug("DdsSubscriberImpl[", _topic->get_name(), "]::on_data_available(): from client ", client_id,
                       "and handle: ", info.publication_handle);
    } else {
      carla::log_error("DdsSubscriberImpl[", _topic->get_name(), "]::on_data_available(): Error ",
                       std::to_string(rcode));
    }
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

  carla::rpc::synchronization_client_id_type GetClientId(
      eprosima::fastdds::dds::InstanceHandle_t const& instance_handle) {
    auto insert_result = _instance_handles.insert(instance_handle);
    return reinterpret_cast<carla::rpc::synchronization_client_id_type const>(&(*insert_result.first));
  }

  eprosima::fastdds::dds::DomainParticipant* _participant{nullptr};
  eprosima::fastdds::dds::Subscriber* _subscriber{nullptr};
  eprosima::fastdds::dds::Topic* _topic{nullptr};
  eprosima::fastdds::dds::DataReader* _datareader{nullptr};
  eprosima::fastdds::dds::TypeSupport _type{new MESSAGE_PUB_TYPE()};

  std::set<eprosima::fastdds::dds::InstanceHandle_t> _instance_handles;
};
}  // namespace ros2
}  // namespace carla
