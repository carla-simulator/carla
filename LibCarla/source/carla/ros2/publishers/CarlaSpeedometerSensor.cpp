#include "CarlaSpeedometerSensor.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32.hpp>

namespace carla {
namespace ros2 {

  struct CarlaSpeedometerSensorImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr _publisher;
    std_msgs::msg::Float32 _float {};
  };

  bool CarlaSpeedometerSensor::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<std_msgs::msg::Float32>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaSpeedometerSensor::Publish() {
    _impl->_publisher->publish(_impl->_float);
    return true;
  }

  void CarlaSpeedometerSensor::SetData(float data) {
    _impl->_float.data = data;
  }

  CarlaSpeedometerSensor::CarlaSpeedometerSensor(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaSpeedometerSensorImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaSpeedometerSensor::~CarlaSpeedometerSensor() {}

  CarlaSpeedometerSensor::CarlaSpeedometerSensor(const CarlaSpeedometerSensor& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaSpeedometerSensor& CarlaSpeedometerSensor::operator=(const CarlaSpeedometerSensor& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaSpeedometerSensor::CarlaSpeedometerSensor(CarlaSpeedometerSensor&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaSpeedometerSensor& CarlaSpeedometerSensor::operator=(CarlaSpeedometerSensor&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
