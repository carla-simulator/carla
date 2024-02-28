// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeDVSCameraPublisher.h"

#include <cstddef>

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"
#include "carla/sensor/s11n/DVSEventArraySerializer.h"

namespace carla {
namespace ros2 {

UeDVSCameraPublisher::UeDVSCameraPublisher(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseCamera(sensor_actor_definition, transform_publisher),
    _point_cloud(std::make_shared<UePointCloudFromBufferPublisherImpl>()) {}

bool UeDVSCameraPublisher::Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return UePublisherBaseCamera::Init(domain_participant) &&
         _point_cloud->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name("point_cloud"),
                                                                    get_topic_qos());
}

bool UeDVSCameraPublisher::Publish() {
  return UePublisherBaseCamera::Publish() && _point_cloud->Publish();
}

bool UeDVSCameraPublisher::SubsribersConnected() const {
  return UePublisherBaseCamera::SubsribersConnected() || _point_cloud->SubsribersConnected();
}

void UeDVSCameraPublisher::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  // TODO: we have to overwrite the camera basic stuff, because we don't have a buffer of Image type at hand,
  // but rather DVSEventArraySerializer
  auto header_view = this->header_view(buffer_view);
  auto data_vector_view = this->vector_view(buffer_view);

  const sensor_msgs::msg::CameraInfo camera_info(header_view->height, header_view->width, header_view->fov_angle);
  auto const stamp = GetTime(sensor_header);
  UpdateCameraInfo(stamp, camera_info);
  UpdateImageHeader(stamp, camera_info);

  SetImageData(data_vector_view);
  SetPointCloudData(buffer_view);
}

void UeDVSCameraPublisher::SetImageData(std::vector<DVSEventConst, DVSEventVectorAllocator> &data_vector_view) {
  std::vector<uint8_t> im_data(image_size(), 0u);
  for (size_t i = 0; i < data_vector_view.size(); ++i) {
    uint32_t index = (data_vector_view[i].y * width() + data_vector_view[i].x) * num_channels() +
                     (static_cast<uint32_t>(data_vector_view[i].pol) * 2u);
    im_data[index] = 255u;
  }
  _image->Message().data(std::move(im_data));
}

void UeDVSCameraPublisher::SetPointCloudData(carla::SharedBufferView &buffer_view) {
  _point_cloud->Message().header(_image->Message().header());

  sensor_msgs::msg::PointField descriptor1;
  descriptor1.name("x");
  descriptor1.offset(offsetof(DVSEventConst, x));
  descriptor1.datatype(sensor_msgs::msg::PointField__UINT16);
  descriptor1.count(1);
  sensor_msgs::msg::PointField descriptor2;
  descriptor2.name("y");
  descriptor2.offset(offsetof(DVSEventConst, y));
  descriptor2.datatype(sensor_msgs::msg::PointField__UINT16);
  descriptor2.count(1);
  sensor_msgs::msg::PointField descriptor3;
  descriptor3.name("t");
  descriptor3.offset(offsetof(DVSEventConst, t));
  descriptor3.datatype(
      sensor_msgs::msg::PointField__FLOAT64);  // PointField__INT64 is not existing, but would be required here!!
  descriptor3.count(1);
  sensor_msgs::msg::PointField descriptor4;
  descriptor3.name("pol");
  descriptor3.offset(offsetof(DVSEventConst, pol));
  descriptor3.datatype(sensor_msgs::msg::PointField__INT8);
  descriptor3.count(1);

  DEBUG_ASSERT_EQ(num_channels(), 4);
  DEBUG_ASSERT_EQ(sizeof(DVSEventConst), 2 * sizeof(uint16_t) + sizeof(double) + sizeof(int8_t));
  const uint32_t point_size = sizeof(DVSEventConst);
  _point_cloud->Message().width(width());
  _point_cloud->Message().height(height());
  _point_cloud->Message().is_bigendian(false);
  _point_cloud->Message().fields({descriptor1, descriptor2, descriptor3, descriptor4});
  _point_cloud->Message().point_step(point_size);
  _point_cloud->Message().row_step(width() * point_size);
  _point_cloud->Message().is_dense(false);  // True if there are not invalid points

  _point_cloud->Message().data(carla::sensor::data::buffer_data_accessed_by_vector<uint8_t>(
      buffer_view, carla::sensor::s11n::DVSEventArraySerializer::header_offset));
}
}  // namespace ros2
}  // namespace carla
