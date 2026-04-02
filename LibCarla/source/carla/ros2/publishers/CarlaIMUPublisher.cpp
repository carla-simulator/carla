#include "CarlaIMUPublisher.h"

#include <string>
#include <cmath>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>

namespace carla {
namespace ros2 {

  struct CarlaIMUPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr _publisher;
    sensor_msgs::msg::Imu _imu {};
  };

  bool CarlaIMUPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<sensor_msgs::msg::Imu>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaIMUPublisher::Publish() {
    _impl->_publisher->publish(_impl->_imu);
    return true;
  }

  void CarlaIMUPublisher::SetData(int32_t seconds, uint32_t nanoseconds, float* pAccelerometer, float* pGyroscope, float compass) {
    const float ax = *pAccelerometer++;
    const float ay = *pAccelerometer++;
    const float az = *pAccelerometer++;
    _impl->_imu.linear_acceleration.x = ax;
    _impl->_imu.linear_acceleration.y = ay;
    _impl->_imu.linear_acceleration.z = az;

    const float gx = *pGyroscope++;
    const float gy = *pGyroscope++;
    const float gz = *pGyroscope++;
    _impl->_imu.angular_velocity.x = gx;
    _impl->_imu.angular_velocity.y = gy;
    _impl->_imu.angular_velocity.z = gz;

    _impl->_imu.header.stamp.sec = seconds;
    _impl->_imu.header.stamp.nanosec = nanoseconds;
    _impl->_imu.header.frame_id = _frame_id;

    const float rx = 0.0f;
    const float ry = (float(M_PI_2) / 2.0f) - compass;
    const float rz = 0.0f;

    const float cr = cosf(rz * 0.5f);
    const float sr = sinf(rz * 0.5f);
    const float cp = cosf(rx * 0.5f);
    const float sp = sinf(rx * 0.5f);
    const float cy = cosf(ry * 0.5f);
    const float sy = sinf(ry * 0.5f);

    _impl->_imu.orientation.w = cr * cp * cy + sr * sp * sy;
    _impl->_imu.orientation.x = sr * cp * cy - cr * sp * sy;
    _impl->_imu.orientation.y = cr * sp * cy + sr * cp * sy;
    _impl->_imu.orientation.z = cr * cp * sy - sr * sp * cy;
  }

  CarlaIMUPublisher::CarlaIMUPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaIMUPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaIMUPublisher::~CarlaIMUPublisher() {}

  CarlaIMUPublisher::CarlaIMUPublisher(const CarlaIMUPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaIMUPublisher& CarlaIMUPublisher::operator=(const CarlaIMUPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaIMUPublisher::CarlaIMUPublisher(CarlaIMUPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaIMUPublisher& CarlaIMUPublisher::operator=(CarlaIMUPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
