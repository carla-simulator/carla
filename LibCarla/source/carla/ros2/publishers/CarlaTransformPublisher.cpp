#include "CarlaTransformPublisher.h"

#include <string>
#include <cmath>
#include <rclcpp/rclcpp.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

namespace carla {
namespace ros2 {

  struct CarlaTransformPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<tf2_msgs::msg::TFMessage>::SharedPtr _publisher;
    tf2_msgs::msg::TFMessage _transform {};

    float last_translation[3] = {0.0f};
    float last_rotation[3] = {0.0f};
    geometry_msgs::msg::Vector3 vec_translation;
    geometry_msgs::msg::Quaternion vec_rotation;
  };

  bool CarlaTransformPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    _impl->_publisher = _impl->_node->create_publisher<tf2_msgs::msg::TFMessage>(
      "tf", 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaTransformPublisher::Publish() {
    _impl->_publisher->publish(_impl->_transform);
    return true;
  }

  void CarlaTransformPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation) {
    int same_translation = std::memcmp(translation, _impl->last_translation, sizeof(float) * 3);
    int same_rotation = std::memcmp(rotation, _impl->last_rotation, sizeof(float) * 3);
    if (same_translation != 0 || same_rotation != 0) {
        std::memcpy(_impl->last_translation, translation, sizeof(float) * 3);
        std::memcpy(_impl->last_rotation, rotation, sizeof(float) * 3);

        const float tx = *translation++;
        const float ty = *translation++;
        const float tz = *translation++;

        const float rx = ((*rotation++) * -1.0f) * (M_PIf32 / 180.0f);
        const float ry = ((*rotation++) * -1.0f) * (M_PIf32 / 180.0f);
        const float rz = *rotation++ * (M_PIf32 / 180.0f);

        const float cr = cosf(rz * 0.5f);
        const float sr = sinf(rz * 0.5f);
        const float cp = cosf(rx * 0.5f);
        const float sp = sinf(rx * 0.5f);
        const float cy = cosf(ry * 0.5f);
        const float sy = sinf(ry * 0.5f);

        _impl->vec_translation.x = tx;
        _impl->vec_translation.y = -ty;
        _impl->vec_translation.z = tz;

        _impl->vec_rotation.w = cr * cp * cy + sr * sp * sy;
        _impl->vec_rotation.x = sr * cp * cy - cr * sp * sy;
        _impl->vec_rotation.y = cr * sp * cy + sr * cp * sy;
        _impl->vec_rotation.z = cr * cp * sy - sr * sp * cy;
    }

    geometry_msgs::msg::TransformStamped ts;
    ts.header.stamp.sec = seconds;
    ts.header.stamp.nanosec = nanoseconds;
    ts.header.frame_id = _parent;
    ts.child_frame_id = _frame_id;
    ts.transform.translation = _impl->vec_translation;
    ts.transform.rotation = _impl->vec_rotation;
    _impl->_transform.transforms = {ts};
  }

  CarlaTransformPublisher::CarlaTransformPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaTransformPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaTransformPublisher::~CarlaTransformPublisher() {}

  CarlaTransformPublisher::CarlaTransformPublisher(const CarlaTransformPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaTransformPublisher& CarlaTransformPublisher::operator=(const CarlaTransformPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaTransformPublisher::CarlaTransformPublisher(CarlaTransformPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaTransformPublisher& CarlaTransformPublisher::operator=(CarlaTransformPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
