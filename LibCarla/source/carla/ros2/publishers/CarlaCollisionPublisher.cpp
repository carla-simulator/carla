#include "CarlaCollisionPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <carla_msgs/msg/carla_collision_event.hpp>

namespace carla {
namespace ros2 {

  struct CarlaCollisionPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<carla_msgs::msg::CarlaCollisionEvent>::SharedPtr _publisher;
    carla_msgs::msg::CarlaCollisionEvent _event {};
  };

  bool CarlaCollisionPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<carla_msgs::msg::CarlaCollisionEvent>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaCollisionPublisher::Publish() {
    _impl->_publisher->publish(_impl->_event);
    return true;
  }

  void CarlaCollisionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, float x, float y, float z) {
    std::vector<float> vector_data;
    SetData(seconds, nanoseconds, actor_id, {x, y, z});
  }

  void CarlaCollisionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, std::vector<float>&& data) {
    _impl->_event.header.stamp.sec = seconds;
    _impl->_event.header.stamp.nanosec = nanoseconds;
    _impl->_event.header.frame_id = _frame_id;
    _impl->_event.other_actor_id = actor_id;
    _impl->_event.normal_impulse.x = data[0];
    _impl->_event.normal_impulse.y = data[1];
    _impl->_event.normal_impulse.z = data[2];
  }

  CarlaCollisionPublisher::CarlaCollisionPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaCollisionPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaCollisionPublisher::~CarlaCollisionPublisher() {}

  CarlaCollisionPublisher::CarlaCollisionPublisher(const CarlaCollisionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaCollisionPublisher& CarlaCollisionPublisher::operator=(const CarlaCollisionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaCollisionPublisher::CarlaCollisionPublisher(CarlaCollisionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaCollisionPublisher& CarlaCollisionPublisher::operator=(CarlaCollisionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
