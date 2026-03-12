// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/UePublisherBaseCamera.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/sensor/s11n/ImageSerializer.h"

namespace carla {
namespace ros2 {

template <class ALLOCATOR>
UePublisherBaseCamera<ALLOCATOR>::UePublisherBaseCamera(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBase(sensor_actor_definition, transform_publisher),
    _image(std::make_shared<UeImagePublisherImpl<ALLOCATOR> >()),
    _camera_info(std::make_shared<UeCameraInfoPublisherImpl>()) {}

template <class ALLOCATOR>
bool UePublisherBaseCamera<ALLOCATOR>::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _image->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name("image"),
                                                              get_topic_qos()) &&
         // camera info uses standard publisher qos
         _camera_info->Init(domain_participant, get_topic_name("camera_info"), PublisherBase::get_topic_qos());
}

template <class ALLOCATOR>
bool UePublisherBaseCamera<ALLOCATOR>::Publish() {
  return _camera_info_initialized && _image->Publish() && _camera_info->Publish();
}

template <class ALLOCATOR>
bool UePublisherBaseCamera<ALLOCATOR>::SubscribersConnected() const {
  return _image->SubscribersConnected() || _camera_info->SubscribersConnected();
}

template <class ALLOCATOR>
sensor_msgs::msg::CameraInfo UePublisherBaseCamera<ALLOCATOR>::CreateCameraInfo(uint32_t height, uint32_t width, double fov)
{
  sensor_msgs::msg::CameraInfo camera_info;
  camera_info.height(height);
  camera_info.width(width);
  camera_info.distortion_model("plumb_bob");

  const double cx = static_cast<double>(width) / 2.0;
  const double cy = static_cast<double>(height) / 2.0;
  const double fx = static_cast<double>(width) / (2.0 * std::tan(fov) * M_PI / 360.0);
  const double fy = fx;

  camera_info.d({ 0.0, 0.0, 0.0, 0.0, 0.0 });
  camera_info.k({fx, 0.0, cx, 0.0, fy, cy, 0.0, 0.0, 1.0});
  camera_info.r({ 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 });
  camera_info.p({fx, 0.0, cx, 0.0, 0.0, fy, cy, 0.0, 0.0, 0.0, 1.0, 0.0});

  camera_info.binning_x(0);
  camera_info.binning_y(0);

  camera_info.roi().x_offset(0);  // up-to-data: constantly 0
  camera_info.roi().y_offset(0);  // up-to-data: constantly 0
  camera_info.roi().height(camera_info.height());
  camera_info.roi().width(camera_info.width());
  camera_info.roi().do_rectify(true);  // up-to-data: constantly true

  return camera_info;
}

template <class ALLOCATOR>
void UePublisherBaseCamera<ALLOCATOR>::UpdateCameraInfo(const builtin_interfaces::msg::Time &stamp,
                                                        sensor_msgs::msg::CameraInfo const &camera_info) {
  _camera_info->Message() = camera_info;
  _camera_info->SetMessageHeader(stamp, frame_id());
  _camera_info_initialized = true;
}

template <class ALLOCATOR>
void UePublisherBaseCamera<ALLOCATOR>::UpdateImageHeader(const builtin_interfaces::msg::Time &stamp,
                                                         sensor_msgs::msg::CameraInfo const &camera_info) {
  // Handle image data
  _image->SetMessageHeader(stamp, frame_id());
  _image->Message().width(camera_info.width());
  _image->Message().height(camera_info.height());
  _image->Message().encoding(encoding_as_string());
  _image->Message().is_bigendian(0);
  _image->Message().step(line_stride());
}

template <class ALLOCATOR>
void UePublisherBaseCamera<ALLOCATOR>::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  auto header_view = UePublisherBaseCamera<ALLOCATOR>::header_view(buffer_view);
  if (!header_view) {
    return;
  }

  const auto camera_info = CreateCameraInfo(header_view->height, header_view->width, header_view->fov_angle);
  auto const stamp = GetTime(sensor_header);
  UpdateCameraInfo(stamp, camera_info);
  UpdateImageHeader(stamp, _camera_info->Message());

  SetImageDataFromBuffer(buffer_view);
}

template <class ALLOCATOR>
void UePublisherBaseCamera<ALLOCATOR>::SetImageDataFromBuffer(const carla::SharedBufferView buffer_view) {
  _image->Message().data(buffer_data_2_vector<uint8_t>(buffer_view));
}

template <class ALLOCATOR>
uint32_t UePublisherBaseCamera<ALLOCATOR>::width() const {
  return _image->Message().width();
}

template <class ALLOCATOR>
uint32_t UePublisherBaseCamera<ALLOCATOR>::height() const {
  return _image->Message().height();
}
}  // namespace ros2
}  // namespace carla
