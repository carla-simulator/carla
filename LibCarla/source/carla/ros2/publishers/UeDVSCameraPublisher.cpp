// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeDVSCameraPublisher.h"

#include <cstddef>

#include "carla/ros2/impl/DdsPublisherImpl.h"
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

bool UeDVSCameraPublisher::SubscribersConnected() const {
  return UePublisherBaseCamera::SubscribersConnected() || _point_cloud->SubscribersConnected();
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
  SetPointCloudData(data_vector_view);
}

void UeDVSCameraPublisher::SetImageData(std::vector<DVSEvent, DVSEventVectorAllocator> &data_vector_view) {
  std::vector<uint8_t> im_data(image_size(), 0u);
  for (size_t i = 0; i < data_vector_view.size(); ++i) {
    uint32_t index = (data_vector_view[i].y * width() + data_vector_view[i].x) * num_channels() +
                     (static_cast<uint32_t>(data_vector_view[i].pol) * 2u);
    im_data[index] = 255u;
  }
  _image->Message().data(std::move(im_data));
}

void UeDVSCameraPublisher::SetPointCloudData(std::vector<DVSEvent, DVSEventVectorAllocator> &data_vector_view) {
  _point_cloud->Message().header(_image->Message().header());
  _point_cloud->SetMessageUpdated();

  sensor_msgs::msg::PointField descriptor1;
  descriptor1.name("x");
  descriptor1.offset(offsetof(DVSEvent, x));
  descriptor1.datatype(sensor_msgs::msg::PointField__UINT16);
  descriptor1.count(1);
  sensor_msgs::msg::PointField descriptor2;
  descriptor2.name("y");
  descriptor2.offset(offsetof(DVSEvent, y));
  descriptor2.datatype(sensor_msgs::msg::PointField__UINT16);
  descriptor2.count(1);
  sensor_msgs::msg::PointField descriptor3;
  descriptor3.name("t");
  descriptor3.offset(offsetof(DVSEvent, t));
  descriptor3.datatype(
      sensor_msgs::msg::PointField__FLOAT64);  // PointField__INT64 is not existing, but would be required here!!
  descriptor3.count(1);
  sensor_msgs::msg::PointField descriptor4;
  descriptor4.name("pol");
  descriptor4.offset(offsetof(DVSEvent, pol));
  descriptor4.datatype(sensor_msgs::msg::PointField__INT8);
  descriptor4.count(1);


#pragma pack(push, 1)
  // definition of the actual data type to be put into the point_cloud (which is different to DVSEvent!!)
  struct DVSPointCloudData {
    explicit DVSPointCloudData(DVSEvent event)
     : x (event.x)
     , y (event.y)
     , t (event.t)
     , pol (event.pol)
     {}
    std::uint16_t x;
    std::uint16_t y;
    double t;
    std::int8_t pol;
  };
#pragma pack(pop)

  DEBUG_ASSERT_EQ(num_channels(), 4);
  const uint32_t point_size = sizeof(DVSPointCloudData);
  _point_cloud->Message().width(width());
  _point_cloud->Message().height(height());
  _point_cloud->Message().is_bigendian(false);
  _point_cloud->Message().fields({descriptor1, descriptor2, descriptor3, descriptor4});
  _point_cloud->Message().point_step(point_size);
  _point_cloud->Message().row_step(width() * point_size);
  _point_cloud->Message().is_dense(false);
  std::vector<uint8_t> pcl_data_uint8_t;
  pcl_data_uint8_t.resize(data_vector_view.size()*point_size);
  for (size_t i = 0; i < data_vector_view.size(); ++i) {
    // convert the DVSEvent format to DVSPointCloudData putting it directly into the desired array to be sent out
    *(reinterpret_cast<DVSPointCloudData*>(pcl_data_uint8_t.data()+i*point_size)) = DVSPointCloudData(data_vector_view[i]);
  }
  _point_cloud->Message().data(std::move(pcl_data_uint8_t));
}
}  // namespace ros2
}  // namespace carla
