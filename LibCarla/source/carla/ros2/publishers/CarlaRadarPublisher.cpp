#include "CarlaRadarPublisher.h"

#include <string>

#include "carla/sensor/data/RadarData.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/point_field.hpp>

namespace carla {
namespace ros2 {

  struct CarlaRadarPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _publisher;
    sensor_msgs::msg::PointCloud2 _radar {};
  };

  struct RadarDetectionWithPosition {
    float x;
    float y;
    float z;
    carla::sensor::data::RadarDetection detection;
  };

  bool CarlaRadarPublisher::Init() {
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

  bool CarlaRadarPublisher::Publish() {
    _impl->_publisher->publish(_impl->_radar);
    return true;
  }

  void CarlaRadarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, const uint8_t* data) {
    std::vector<uint8_t> vector_data;
    const size_t size = elements * sizeof(RadarDetectionWithPosition);
    vector_data.resize(size);
    RadarDetectionWithPosition* radar_data = (RadarDetectionWithPosition*)&vector_data[0];
    carla::sensor::data::RadarDetection* detection_data = (carla::sensor::data::RadarDetection*)data;
    for (size_t i = 0; i < elements; ++i, ++radar_data, ++detection_data) {
      radar_data->x = detection_data->depth * cosf(detection_data->azimuth) * cosf(-detection_data->altitude);
      radar_data->y = detection_data->depth * sinf(-detection_data->azimuth) * cosf(detection_data->altitude);
      radar_data->z = detection_data->depth * sinf(detection_data->altitude);
      radar_data->detection = *detection_data;
    }
    SetData(seconds, nanoseconds, height, width, elements, std::move(vector_data));
  }

  void CarlaRadarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, std::vector<uint8_t>&& data) {
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
    descriptor4.name = "velocity";
    descriptor4.offset = 12;
    descriptor4.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor4.count = 1;
    sensor_msgs::msg::PointField descriptor5;
    descriptor5.name = "azimuth";
    descriptor5.offset = 16;
    descriptor5.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor5.count = 1;
    sensor_msgs::msg::PointField descriptor6;
    descriptor6.name = "altitude";
    descriptor6.offset = 20;
    descriptor6.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor6.count = 1;
    sensor_msgs::msg::PointField descriptor7;
    descriptor7.name = "depth";
    descriptor7.offset = 24;
    descriptor7.datatype = sensor_msgs::msg::PointField::FLOAT32;
    descriptor7.count = 1;

    const size_t point_size = sizeof(RadarDetectionWithPosition);
    _impl->_radar.header.stamp.sec = seconds;
    _impl->_radar.header.stamp.nanosec = nanoseconds;
    _impl->_radar.header.frame_id = _frame_id;
    _impl->_radar.width = elements;
    _impl->_radar.height = height;
    _impl->_radar.is_bigendian = false;
    _impl->_radar.fields = {descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6, descriptor7};
    _impl->_radar.point_step = point_size;
    _impl->_radar.row_step = elements * point_size;
    _impl->_radar.is_dense = false;
    _impl->_radar.data = std::move(data);
  }

  CarlaRadarPublisher::CarlaRadarPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaRadarPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaRadarPublisher::~CarlaRadarPublisher() {}

  CarlaRadarPublisher::CarlaRadarPublisher(const CarlaRadarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaRadarPublisher& CarlaRadarPublisher::operator=(const CarlaRadarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    return *this;
  }

  CarlaRadarPublisher::CarlaRadarPublisher(CarlaRadarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaRadarPublisher& CarlaRadarPublisher::operator=(CarlaRadarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    return *this;
  }
}
}
