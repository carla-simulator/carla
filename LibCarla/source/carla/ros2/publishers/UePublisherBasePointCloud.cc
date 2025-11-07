// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/UePublisherBasePointCloud.h"
#include "carla/ros2/impl/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

template <class HEADER_TYPE, class DATA_TYPE>
UePublisherBasePointCloud<HEADER_TYPE, DATA_TYPE>::UePublisherBasePointCloud(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseSensor(sensor_actor_definition, transform_publisher),
    _point_cloud(std::make_shared<UePublisherPointCloudImpl>()) {}

template <class HEADER_TYPE, class DATA_TYPE>
bool UePublisherBasePointCloud<HEADER_TYPE, DATA_TYPE>::Init(
    std::shared_ptr<DdsDomainParticipantImpl> domain_participant) {
  return _point_cloud->InitHistoryPreallocatedWithReallocMemoryMode(domain_participant, get_topic_name(),
                                                                    get_topic_qos());
}

template <class HEADER_TYPE, class DATA_TYPE>
bool UePublisherBasePointCloud<HEADER_TYPE, DATA_TYPE>::Publish() {
  return _point_cloud->Publish();
}

template <class HEADER_TYPE, class DATA_TYPE>
bool UePublisherBasePointCloud<HEADER_TYPE, DATA_TYPE>::SubscribersConnected() const {
  return _point_cloud->SubscribersConnected();
}

template <class HEADER_TYPE, class DATA_TYPE>
void UePublisherBasePointCloud<HEADER_TYPE, DATA_TYPE>::UpdateSensorData(
    std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
    carla::SharedBufferView buffer_view) {
  auto header_view = this->header_view(buffer_view);
  auto data_vector_view = this->vector_view(buffer_view);

  _point_cloud->SetMessageHeader(GetTime(sensor_header), frame_id());
  const size_t point_size = GetMessagePointSize();
  _point_cloud->Message().width(1);
  _point_cloud->Message().height(uint32_t(data_vector_view.size()));
  _point_cloud->Message().is_bigendian(false);
  _point_cloud->Message().fields(GetPointFields());
  _point_cloud->Message().point_step(point_size);
  _point_cloud->Message().row_step(_point_cloud->Message().width() * point_size);
  _point_cloud->Message().is_dense(false);  // True if there are not invalid points

  SetPointCloudDataFromBuffer(header_view, data_vector_view);
}
}  // namespace ros2
}  // namespace carla
