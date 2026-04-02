#include "CarlaSemanticLidarPublisher.h"

#include <string>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/point_field.hpp>

namespace carla {
namespace ros2 {

  struct CarlaSemanticLidarPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _publisher;
    sensor_msgs::msg::PointCloud2 _lidar {};
  };

  bool CarlaSemanticLidarPublisher::Init() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name);
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_publisher = _impl->_node->create_publisher<sensor_msgs::msg::PointCloud2>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaSemanticLidarPublisher::Publish() {
    _impl->_publisher->publish(_impl->_lidar);
    return true;
  }

  void CarlaSemanticLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, float* data) {
    float* it = data;
    float* end = &data[height * width * elements];
    for (++it; it < end; it += elements) {
        *it *= -1.0f;
    }

    std::vector<uint8_t> vector_data;
    const size_t size = height * width * sizeof(float) * elements;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaSemanticLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name = "x";
    descriptor1.offset = 0;
    descriptor1.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor1.count = 1;
    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name = "y";
    descriptor2.offset = 4;
    descriptor2.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor2.count = 1;
    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name = "z";
    descriptor3.offset = 8;
    descriptor3.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor3.count = 1;
    sensor_msgs::msg::PointField descriptor4;
    descriptor4.name = "cos_inc_angle";
    descriptor4.offset = 12;
    descriptor4.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor4.count = 1;
    sensor_msgs::msg::PointField descriptor5;
    descriptor5.name = "object_idx";
    descriptor5.offset = 16;
    descriptor5.datatype = sensor_msgs::msg::PointField::UINT32;
    descriptor5.count = 1;
    sensor_msgs::msg::PointField descriptor6;
    descriptor6.name = "object_tag";
    descriptor6.offset = 20;
    descriptor6.datatype = sensor_msgs::msg::PointField::UINT32;
    descriptor6.count = 1;

    const size_t point_size = 6 * sizeof(float);
    _impl->_lidar.header.stamp.sec = seconds;
    _impl->_lidar.header.stamp.nanosec = nanoseconds;
    _impl->_lidar.header.frame_id = _frame_id;
    _impl->_lidar.width = width;
    _impl->_lidar.height = height;
    _impl->_lidar.is_bigendian = false;
    _impl->_lidar.fields = {descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6};
    _impl->_lidar.point_step = point_size;
    _impl->_lidar.row_step = width * point_size;
    _impl->_lidar.is_dense = false;
    _impl->_lidar.data = std::move(data);
  }

  CarlaSemanticLidarPublisher::CarlaSemanticLidarPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaSemanticLidarPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaSemanticLidarPublisher::~CarlaSemanticLidarPublisher() {}

  CarlaSemanticLidarPublisher::CarlaSemanticLidarPublisher(const CarlaSemanticLidarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaSemanticLidarPublisher& CarlaSemanticLidarPublisher::operator=(const CarlaSemanticLidarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaSemanticLidarPublisher::CarlaSemanticLidarPublisher(CarlaSemanticLidarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaSemanticLidarPublisher& CarlaSemanticLidarPublisher::operator=(CarlaSemanticLidarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
