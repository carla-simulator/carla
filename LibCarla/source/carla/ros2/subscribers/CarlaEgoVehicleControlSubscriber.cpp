#include "CarlaEgoVehicleControlSubscriber.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <carla_msgs/msg/carla_ego_vehicle_control.hpp>

namespace carla {
namespace ros2 {

  struct CarlaEgoVehicleControlSubscriberImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Subscription<carla_msgs::msg::CarlaEgoVehicleControl>::SharedPtr _subscription;
    VehicleControl _control {};
    bool _new_message {false};
    bool _alive {true};
    void* _vehicle {nullptr};
  };

  bool CarlaEgoVehicleControlSubscriber::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_subscription = _impl->_node->create_subscription<carla_msgs::msg::CarlaEgoVehicleControl>(
      topic_name, 10,
      [this](const carla_msgs::msg::CarlaEgoVehicleControl::SharedPtr msg) {
        VehicleControl control;
        control.throttle = msg->throttle;
        control.steer = msg->steer;
        control.brake = msg->brake;
        control.hand_brake = msg->hand_brake;
        control.reverse = msg->reverse;
        control.gear = msg->gear;
        control.manual_gear_shift = msg->manual_gear_shift;
        ForwardMessage(control);
      });
    return true;
  }

  bool CarlaEgoVehicleControlSubscriber::Read() {
    rclcpp::spin_some(_impl->_node);
    return _impl->_new_message;
  }

  void CarlaEgoVehicleControlSubscriber::ForwardMessage(VehicleControl control) {
    _impl->_control = control;
    _impl->_new_message = true;
  }

  void CarlaEgoVehicleControlSubscriber::DestroySubscriber() {
    _impl->_alive = false;
  }

  VehicleControl CarlaEgoVehicleControlSubscriber::GetMessage() {
    _impl->_new_message = false;
    return _impl->_control;
  }

  bool CarlaEgoVehicleControlSubscriber::IsAlive() {
    return _impl->_alive;
  }

  bool CarlaEgoVehicleControlSubscriber::HasNewMessage() {
    return _impl->_new_message;
  }

  void* CarlaEgoVehicleControlSubscriber::GetVehicle() {
    return _impl->_vehicle;
  }

  CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(void* vehicle, const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaEgoVehicleControlSubscriberImpl>()) {
    _impl->_vehicle = vehicle;
    _name = ros_name;
    _parent = parent;
  }

  CarlaEgoVehicleControlSubscriber::~CarlaEgoVehicleControlSubscriber() {}

  CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(const CarlaEgoVehicleControlSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaEgoVehicleControlSubscriber& CarlaEgoVehicleControlSubscriber::operator=(const CarlaEgoVehicleControlSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(CarlaEgoVehicleControlSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaEgoVehicleControlSubscriber& CarlaEgoVehicleControlSubscriber::operator=(CarlaEgoVehicleControlSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
