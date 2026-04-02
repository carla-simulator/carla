#include "CarlaLineInvasionPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <carla_msgs/msg/lane_invasion_event.hpp>

namespace carla {
namespace ros2 {

  struct CarlaLineInvasionPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<carla_msgs::msg::LaneInvasionEvent>::SharedPtr _publisher;
    carla_msgs::msg::LaneInvasionEvent _event {};
  };

  bool CarlaLineInvasionPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<carla_msgs::msg::LaneInvasionEvent>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaLineInvasionPublisher::Publish() {
    _impl->_publisher->publish(_impl->_event);
    return true;
  }

  void CarlaLineInvasionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const int32_t* data) {
    _impl->_event.header.stamp.sec = seconds;
    _impl->_event.header.stamp.nanosec = nanoseconds;
    _impl->_event.header.frame_id = _frame_id;
    _impl->_event.crossed_lane_markings = {data[0], data[1], data[2]};
  }

  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaLineInvasionPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaLineInvasionPublisher::~CarlaLineInvasionPublisher() {}

  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
