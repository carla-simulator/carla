#include "CarlaISCameraPublisher.h"

#include <string>
#include <cmath>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>

namespace carla {
namespace ros2 {

  struct CarlaISCameraPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr _publisher;
    sensor_msgs::msg::Image _image {};
  };

  struct CarlaCameraInfoPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr _publisher;
    bool _init {false};
    sensor_msgs::msg::CameraInfo _info {};
  };

  bool CarlaISCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }

  void CarlaISCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info.height = height;
    _impl_info->_info.width = width;
    _impl_info->_info.distortion_model = "plumb_bob";

    const double cx = static_cast<double>(width) / 2.0;
    const double cy = static_cast<double>(height) / 2.0;
    const double fx = static_cast<double>(width) / (2.0 * std::tan(fov) * M_PI / 360.0);
    const double fy = fx;

    _impl_info->_info.d = {0.0, 0.0, 0.0, 0.0, 0.0};
    _impl_info->_info.k = {fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0};
    _impl_info->_info.r = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    _impl_info->_info.p = {fx, 0.0, cx, 0.0, 0.0, fy, cy, 0.0, 0.0, 0.0, 1.0, 0.0};
    _impl_info->_info.binning_x = 0;
    _impl_info->_info.binning_y = 0;

    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }

  bool CarlaISCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }

  bool CarlaISCameraPublisher::InitImage() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name + "_is_image");
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += "/image";
    _impl->_publisher = _impl->_node->create_publisher<sensor_msgs::msg::Image>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaISCameraPublisher::InitInfo() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl_info->_node = rclcpp::Node::make_shared(_name + "_is_camera_info");
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += "/camera_info";
    _impl_info->_publisher = _impl_info->_node->create_publisher<sensor_msgs::msg::CameraInfo>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaISCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }

  bool CarlaISCameraPublisher::PublishImage() {
    _impl->_publisher->publish(_impl->_image);
    return true;
  }

  bool CarlaISCameraPublisher::PublishInfo() {
    _impl_info->_publisher->publish(_impl_info->_info);
    return true;
  }

  void CarlaISCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> vector_data;
    const size_t size = height * width * 4;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaISCameraPublisher::SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    _impl_info->_info.roi.x_offset = x_offset;
    _impl_info->_info.roi.y_offset = y_offset;
    _impl_info->_info.roi.height = height;
    _impl_info->_info.roi.width = width;
    _impl_info->_info.roi.do_rectify = do_rectify;
  }

  void CarlaISCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    _impl->_image.header.stamp.sec = seconds;
    _impl->_image.header.stamp.nanosec = nanoseconds;
    _impl->_image.header.frame_id = _frame_id;
    _impl->_image.width = width;
    _impl->_image.height = height;
    _impl->_image.encoding = "bgra8";
    _impl->_image.is_bigendian = 0;
    _impl->_image.step = _impl->_image.width * sizeof(uint8_t) * 4;
    _impl->_image.data = std::move(data);
  }

  void CarlaISCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    _impl_info->_info.header.stamp.sec = seconds;
    _impl_info->_info.header.stamp.nanosec = nanoseconds;
    _impl_info->_info.header.frame_id = _frame_id;
  }

  CarlaISCameraPublisher::CarlaISCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaISCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaISCameraPublisher::~CarlaISCameraPublisher() {}

  CarlaISCameraPublisher::CarlaISCameraPublisher(const CarlaISCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(const CarlaISCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
    return *this;
  }

  CarlaISCameraPublisher::CarlaISCameraPublisher(CarlaISCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
  }

  CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(CarlaISCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
    return *this;
  }
}
}
