#include "CarlaMapSensorPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

namespace carla {
namespace ros2 {

  struct CarlaMapSensorPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr _publisher;
    std_msgs::msg::String _string {};
  };

  bool CarlaMapSensorPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<std_msgs::msg::String>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaMapSensorPublisher::Publish() {
    _impl->_publisher->publish(_impl->_string);
    return true;
  }

  void CarlaMapSensorPublisher::SetData(const char* data) {
    _impl->_string.data = data;
  }

  CarlaMapSensorPublisher::CarlaMapSensorPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaMapSensorPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaMapSensorPublisher::~CarlaMapSensorPublisher() {}

  CarlaMapSensorPublisher::CarlaMapSensorPublisher(const CarlaMapSensorPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaMapSensorPublisher& CarlaMapSensorPublisher::operator=(const CarlaMapSensorPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaMapSensorPublisher::CarlaMapSensorPublisher(CarlaMapSensorPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaMapSensorPublisher& CarlaMapSensorPublisher::operator=(CarlaMapSensorPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
