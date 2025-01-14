#include "BasicSubscriber.h"

#include "carla/ros2/types/String.h"
#include "carla/ros2/types/StringPubSubTypes.h"
#include "carla/ros2/listeners/BasicListener.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>


namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  struct BasicSubscriberImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Subscriber* _subscriber { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataReader* _datareader { nullptr };
    efd::TypeSupport _type { new std_msgs::msg::StringPubSubType() };
    BasicListener _listener {nullptr};
    std_msgs::msg::String _event {};
    std::string _message {};
    bool _new_message {false};
    bool _alive {true};
    void* _actor {nullptr};
  };

  bool BasicSubscriber::Init() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl->_type.register_type(_impl->_participant);

    efd::SubscriberQos subqos = efd::SUBSCRIBER_QOS_DEFAULT;
    _impl->_subscriber = _impl->_participant->create_subscriber(subqos, nullptr);
    if (_impl->_subscriber == nullptr) {
      std::cerr << "Failed to create Subscriber" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string base { "rt/carla/" };
    const std::string subscriber_type {"/basic_subscriber_example"};
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += subscriber_type;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    efd::DataReaderQos rqos = efd::DATAREADER_QOS_DEFAULT;
    efd::DataReaderListener* listener = (efd::DataReaderListener*)_impl->_listener._impl.get();
    _impl->_datareader = _impl->_subscriber->create_datareader(_impl->_topic, rqos, listener);
    if (_impl->_datareader == nullptr) {
        std::cerr << "Failed to create DataReader" << std::endl;
        return false;
    }
    return true;
  }

  void BasicSubscriber::ForwardMessage(const std::string& message) {
    _impl->_message = message;
    _impl->_new_message = true;
  }

  void BasicSubscriber::DestroySubscriber() {
    _impl->_alive = false;
  }

  const char* BasicSubscriber::GetMessage() {
    _impl->_new_message = false;
    return _impl->_message.c_str();
  }

  bool BasicSubscriber::IsAlive() {
    return _impl->_alive;
  }

  bool BasicSubscriber::HasNewMessage() {
    return _impl->_new_message;
  }

  void* BasicSubscriber::GetActor() {
    return _impl->_actor;
  }

  BasicSubscriber::BasicSubscriber(void* actor, const char* ros_name, const char* parent) :
     _impl(std::make_shared<BasicSubscriberImpl>()) {
    _impl->_listener.SetOwner(this);
    _impl->_actor = actor;
    _name = ros_name;
    _parent = parent;
  }

  BasicSubscriber::~BasicSubscriber() {
      if (!_impl)
          return;

      if (_impl->_datareader)
          _impl->_subscriber->delete_datareader(_impl->_datareader);

      if (_impl->_subscriber)
          _impl->_participant->delete_subscriber(_impl->_subscriber);

      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);

      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);
  }

  BasicSubscriber::BasicSubscriber(const BasicSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl->_listener.SetOwner(this);
  }

  BasicSubscriber& BasicSubscriber::operator=(const BasicSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl->_listener.SetOwner(this);

    return *this;
  }

  BasicSubscriber::BasicSubscriber(BasicSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl->_listener.SetOwner(this);
  }

  BasicSubscriber& BasicSubscriber::operator=(BasicSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl->_listener.SetOwner(this);

    return *this;
  }
}
}
