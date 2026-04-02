#include "CarlaGNSSPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/nav_sat_fix.hpp>

namespace carla {
namespace ros2 {

  struct CarlaGNSSPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::NavSatFix>::SharedPtr _publisher;
    sensor_msgs::msg::NavSatFix _nav {};
  };

  bool CarlaGNSSPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<sensor_msgs::msg::NavSatFix>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaGNSSPublisher::Publish() {
    _impl->_publisher->publish(_impl->_nav);
    return true;
  }

  void CarlaGNSSPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const double* data) {
    _impl->_nav.header.stamp.sec = seconds;
    _impl->_nav.header.stamp.nanosec = nanoseconds;
    _impl->_nav.header.frame_id = _frame_id;
    _impl->_nav.latitude = *data++;
    _impl->_nav.longitude = *data++;
    _impl->_nav.altitude = *data++;
  }

  CarlaGNSSPublisher::CarlaGNSSPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaGNSSPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaGNSSPublisher::~CarlaGNSSPublisher() {}

  CarlaGNSSPublisher::CarlaGNSSPublisher(const CarlaGNSSPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaGNSSPublisher& CarlaGNSSPublisher::operator=(const CarlaGNSSPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaGNSSPublisher::CarlaGNSSPublisher(CarlaGNSSPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaGNSSPublisher& CarlaGNSSPublisher::operator=(CarlaGNSSPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
