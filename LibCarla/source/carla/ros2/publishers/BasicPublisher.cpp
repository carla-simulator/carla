#include "BasicPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

namespace carla {
namespace ros2 {

  struct BasicPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;
    std_msgs::msg::String _message {};
  };

  bool BasicPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    _impl->_publisher = _impl->_node->create_publisher<std_msgs::msg::String>(
      "basic_publisher_example", 10);
    _frame_id = _name;
    return true;
  }

  bool BasicPublisher::Publish() {
    _impl->_publisher->publish(_impl->_message);
    return true;
  }

  void BasicPublisher::SetData(const char* msg) {
    _impl->_message.data = std::string(msg);
  }

  BasicPublisher::BasicPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<BasicPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  BasicPublisher::~BasicPublisher() {}

  BasicPublisher::BasicPublisher(const BasicPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  BasicPublisher& BasicPublisher::operator=(const BasicPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  BasicPublisher::BasicPublisher(BasicPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  BasicPublisher& BasicPublisher::operator=(BasicPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
