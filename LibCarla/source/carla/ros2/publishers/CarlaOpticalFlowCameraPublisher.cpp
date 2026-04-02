#include "CarlaOpticalFlowCameraPublisher.h"

#include <string>
#include <cmath>
#include <algorithm>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>

namespace carla {
namespace ros2 {

  struct CarlaOpticalFlowCameraPublisherImpl {
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

  bool CarlaOpticalFlowCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }

  void CarlaOpticalFlowCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
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

  bool CarlaOpticalFlowCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }

  bool CarlaOpticalFlowCameraPublisher::InitImage() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name + "_optflow_image");
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

  bool CarlaOpticalFlowCameraPublisher::InitInfo() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl_info->_node = rclcpp::Node::make_shared(_name + "_optflow_camera_info");
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

  bool CarlaOpticalFlowCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }

  bool CarlaOpticalFlowCameraPublisher::PublishImage() {
    _impl->_publisher->publish(_impl->_image);
    return true;
  }

  bool CarlaOpticalFlowCameraPublisher::PublishInfo() {
    _impl_info->_publisher->publish(_impl_info->_info);
    return true;
  }

  void CarlaOpticalFlowCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const float* data) {
    constexpr float pi = 3.1415f;
    constexpr float rad2ang = 360.0f/(2.0f*pi);
    const size_t max_index = width * height * 2;
    std::vector<uint8_t> vector_data;
    vector_data.resize(height * width * 4);
    size_t data_index = 0;
    for (size_t index = 0; index < max_index; index += 2) {
        const float vx = data[index];
        const float vy = data[index + 1];
        float angle = 180.0f + std::atan2(vy, vx) * rad2ang;
        if (angle < 0) {
            angle = 360.0f + angle;
        }
        angle = std::fmod(angle, 360.0f);

        const float norm = std::sqrt(vx * vx + vy * vy);
        const float shift = 0.999f;
        const float a = 1.0f / std::log(0.1f + shift);
        const float intensity = std::clamp(a * std::log(norm + shift), 0.0f, 1.0f);

        const float& H = angle;
        const float S = 1.0f;
        const float V = intensity;
        const float H_60 = H * (1.0f / 60.0f);

        const float C = V * S;
        const float X = C * (1.0f - std::abs(std::fmod(H_60, 2.0f) - 1.0f));
        const float m = V - C;

        float r = 0, g = 0, b = 0;
        const unsigned int angle_case = static_cast<const unsigned int>(H_60);
        switch (angle_case) {
        case 0: r = C; g = X; b = 0; break;
        case 1: r = X; g = C; b = 0; break;
        case 2: r = 0; g = C; b = X; break;
        case 3: r = 0; g = X; b = C; break;
        case 4: r = X; g = 0; b = C; break;
        case 5: r = C; g = 0; b = X; break;
        default: r = 1; g = 1; b = 1; break;
        }

        const uint8_t R = static_cast<uint8_t>((r + m) * 255.0f);
        const uint8_t G = static_cast<uint8_t>((g + m) * 255.0f);
        const uint8_t B = static_cast<uint8_t>((b + m) * 255.0f);

        vector_data[data_index++] = B;
        vector_data[data_index++] = G;
        vector_data[data_index++] = R;
        vector_data[data_index++] = 0;
    }
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaOpticalFlowCameraPublisher::SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    _impl_info->_info.roi.x_offset = x_offset;
    _impl_info->_info.roi.y_offset = y_offset;
    _impl_info->_info.roi.height = height;
    _impl_info->_info.roi.width = width;
    _impl_info->_info.roi.do_rectify = do_rectify;
  }

  void CarlaOpticalFlowCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
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

  void CarlaOpticalFlowCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    _impl_info->_info.header.stamp.sec = seconds;
    _impl_info->_info.header.stamp.nanosec = nanoseconds;
    _impl_info->_info.header.frame_id = _frame_id;
  }

  CarlaOpticalFlowCameraPublisher::CarlaOpticalFlowCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaOpticalFlowCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaOpticalFlowCameraPublisher::~CarlaOpticalFlowCameraPublisher() {}

  CarlaOpticalFlowCameraPublisher::CarlaOpticalFlowCameraPublisher(const CarlaOpticalFlowCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaOpticalFlowCameraPublisher& CarlaOpticalFlowCameraPublisher::operator=(const CarlaOpticalFlowCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
    return *this;
  }

  CarlaOpticalFlowCameraPublisher::CarlaOpticalFlowCameraPublisher(CarlaOpticalFlowCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
  }

  CarlaOpticalFlowCameraPublisher& CarlaOpticalFlowCameraPublisher::operator=(CarlaOpticalFlowCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
    return *this;
  }
}
}
