#include "CarlaDVSCameraPublisher.h"

#include <string>
#include <cmath>

#include "carla/sensor/data/DVSEvent.h"

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/msg/point_field.hpp>

namespace carla {
namespace ros2 {

  struct CarlaDVSCameraPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr _publisher;
    sensor_msgs::msg::Image _image {};
  };

  struct CarlaCameraInfoPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr _publisher;
    bool _init {false};
    sensor_msgs::msg::CameraInfo _ci {};
  };

  struct CarlaPointCloudPublisherImpl {
    rclcpp::Node::SharedPtr _node;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr _publisher;
    sensor_msgs::msg::PointCloud2 _pc {};
  };

  bool CarlaDVSCameraPublisher::HasBeenInitialized() const {
    return _info->_init;
  }

  void CarlaDVSCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _info->_ci.height = height;
    _info->_ci.width = width;
    _info->_ci.distortion_model = "plumb_bob";

    const double cx = static_cast<double>(width) / 2.0;
    const double cy = static_cast<double>(height) / 2.0;
    const double fx = static_cast<double>(width) / (2.0 * std::tan(fov) * M_PI / 360.0);
    const double fy = fx;

    _info->_ci.d = {0.0, 0.0, 0.0, 0.0, 0.0};
    _info->_ci.k = {fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0};
    _info->_ci.r = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    _info->_ci.p = {fx, 0.0, cx, 0.0, 0.0, fy, cy, 0.0, 0.0, 0.0, 1.0, 0.0};
    _info->_ci.binning_x = 0;
    _info->_ci.binning_y = 0;

    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _info->_init = true;
  }

  bool CarlaDVSCameraPublisher::Init() {
    return InitImage() && InitInfo() && InitPointCloud();
  }

  bool CarlaDVSCameraPublisher::InitImage() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _impl->_node = rclcpp::Node::make_shared(_name + "_dvs_image");
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

  bool CarlaDVSCameraPublisher::InitInfo() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _info->_node = rclcpp::Node::make_shared(_name + "_dvs_camera_info");
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += "/camera_info";
    _info->_publisher = _info->_node->create_publisher<sensor_msgs::msg::CameraInfo>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaDVSCameraPublisher::InitPointCloud() {
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }
    _point_cloud->_node = rclcpp::Node::make_shared(_name + "_dvs_point_cloud");
    std::string topic_name = "carla/";
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += "/point_cloud";
    _point_cloud->_publisher = _point_cloud->_node->create_publisher<sensor_msgs::msg::PointCloud2>(
      topic_name, 10);
    _frame_id = _name;
    return true;
  }

  bool CarlaDVSCameraPublisher::Publish() {
    return PublishImage() && PublishInfo() && PublishPointCloud();
  }

  bool CarlaDVSCameraPublisher::PublishImage() {
    _impl->_publisher->publish(_impl->_image);
    return true;
  }

  bool CarlaDVSCameraPublisher::PublishInfo() {
    _info->_publisher->publish(_info->_ci);
    return true;
  }

  bool CarlaDVSCameraPublisher::PublishPointCloud() {
    _point_cloud->_publisher->publish(_point_cloud->_pc);
    return true;
  }

  void CarlaDVSCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> im_data;
    const size_t im_size = width * height * 3;
    im_data.resize(im_size);
    carla::sensor::data::DVSEvent* vec_event = (carla::sensor::data::DVSEvent*)&data[0];
    for (size_t i = 0; i < elements; ++i, ++vec_event) {
        size_t index = (vec_event->y * width + vec_event->x) * 3 + (static_cast<int>(vec_event->pol) * 2);
        im_data[index] = 255;
    }

    SetData(seconds, nanoseconds, height, width, std::move(im_data));
  }

  void CarlaDVSCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    _impl->_image.header.stamp.sec = seconds;
    _impl->_image.header.stamp.nanosec = nanoseconds;
    _impl->_image.header.frame_id = _frame_id;
    _info->_ci.header.stamp.sec = seconds;
    _info->_ci.header.stamp.nanosec = nanoseconds;
    _info->_ci.header.frame_id = _frame_id;
    _point_cloud->_pc.header.stamp.sec = seconds;
    _point_cloud->_pc.header.stamp.nanosec = nanoseconds;
    _point_cloud->_pc.header.frame_id = _frame_id;

    _impl->_image.width = width;
    _impl->_image.height = height;
    _impl->_image.encoding = "bgr8";
    _impl->_image.is_bigendian = 0;
    _impl->_image.step = _impl->_image.width * sizeof(uint8_t) * 3;
    _impl->_image.data = std::move(data);
  }

  void CarlaDVSCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    _info->_ci.header.stamp.sec = seconds;
    _info->_ci.header.stamp.nanosec = nanoseconds;
    _info->_ci.header.frame_id = _frame_id;
  }

  void CarlaDVSCameraPublisher::SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    _info->_ci.roi.x_offset = x_offset;
    _info->_ci.roi.y_offset = y_offset;
    _info->_ci.roi.height = height;
    _info->_ci.roi.width = width;
    _info->_ci.roi.do_rectify = do_rectify;
  }

  void CarlaDVSCameraPublisher::SetPointCloudData(size_t height, size_t width, size_t elements, const uint8_t* data) {
    std::vector<uint8_t> vector_data;
    const size_t size = height * width;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);

    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name = "x";
    descriptor1.offset = 0;
    descriptor1.datatype = sensor_msgs::msg::PointField::UINT16;
    descriptor1.count = 1;
    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name = "y";
    descriptor2.offset = 2;
    descriptor2.datatype = sensor_msgs::msg::PointField::UINT16;
    descriptor2.count = 1;
    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name = "t";
    descriptor3.offset = 4;
    descriptor3.datatype = sensor_msgs::msg::PointField::FLOAT64;
    descriptor3.count = 1;
    sensor_msgs::msg::PointField descriptor4;
    descriptor4.name = "pol";
    descriptor4.offset = 12;
    descriptor4.datatype = sensor_msgs::msg::PointField::INT8;
    descriptor4.count = 1;

    const size_t point_size = sizeof(carla::sensor::data::DVSEvent);
    _point_cloud->_pc.width = width;
    _point_cloud->_pc.height = height;
    _point_cloud->_pc.is_bigendian = false;
    _point_cloud->_pc.fields = {descriptor1, descriptor2, descriptor3, descriptor4};
    _point_cloud->_pc.point_step = point_size;
    _point_cloud->_pc.row_step = width * point_size;
    _point_cloud->_pc.is_dense = false;
    _point_cloud->_pc.data = std::move(vector_data);
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaDVSCameraPublisherImpl>()),
  _info(std::make_shared<CarlaCameraInfoPublisherImpl>()),
  _point_cloud(std::make_shared<CarlaPointCloudPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaDVSCameraPublisher::~CarlaDVSCameraPublisher() {}

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _info = other._info;
    _point_cloud = other._point_cloud;
  }

  CarlaDVSCameraPublisher& CarlaDVSCameraPublisher::operator=(const CarlaDVSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _info = other._info;
    _point_cloud = other._point_cloud;
    return *this;
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(CarlaDVSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _info = std::move(other._info);
    _point_cloud = std::move(other._point_cloud);
  }

  CarlaDVSCameraPublisher& CarlaDVSCameraPublisher::operator=(CarlaDVSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _info = std::move(other._info);
    _point_cloud = std::move(other._point_cloud);
    return *this;
  }
}
}
