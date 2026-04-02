#include "CarlaClockPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <rosgraph_msgs/msg/clock.hpp>

namespace carla {
namespace ros2 {

  struct CarlaClockPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<rosgraph_msgs::msg::Clock>::SharedPtr _publisher;
    rosgraph_msgs::msg::Clock _clock {};
  };

  bool CarlaClockPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    _impl->_publisher = _impl->_node->create_publisher<rosgraph_msgs::msg::Clock>(
      "clock", 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaClockPublisher::Publish() {
    _impl->_publisher->publish(_impl->_clock);
    return true;
  }

  void CarlaClockPublisher::SetData(int32_t sec, uint32_t nanosec) {
    _impl->_clock.clock.sec = sec;
    _impl->_clock.clock.nanosec = nanosec;
  }

  CarlaClockPublisher::CarlaClockPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaClockPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaClockPublisher::~CarlaClockPublisher() {}

  CarlaClockPublisher::CarlaClockPublisher(const CarlaClockPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaClockPublisher& CarlaClockPublisher::operator=(const CarlaClockPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaClockPublisher::CarlaClockPublisher(CarlaClockPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaClockPublisher& CarlaClockPublisher::operator=(CarlaClockPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
