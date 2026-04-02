#include "BasicSubscriber.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

namespace carla {
namespace ros2 {

  struct BasicSubscriberImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr _subscription;
    std::string _message {};
    bool _new_message {false};
    bool _alive {true};
    void* _actor {nullptr};
  };

  bool BasicSubscriber::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += "/basic_subscriber_example";
    _impl->_subscription = _impl->_node->create_subscription<std_msgs::msg::String>(
      topic_name, 10,
      [this](const std_msgs::msg::String::SharedPtr msg) {
        ForwardMessage(msg->data);
      });
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
    _impl->_actor = actor;
    _name = ros_name;
    _parent = parent;
  }

  BasicSubscriber::~BasicSubscriber() {}

  BasicSubscriber::BasicSubscriber(const BasicSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  BasicSubscriber& BasicSubscriber::operator=(const BasicSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  BasicSubscriber::BasicSubscriber(BasicSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  BasicSubscriber& BasicSubscriber::operator=(BasicSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
