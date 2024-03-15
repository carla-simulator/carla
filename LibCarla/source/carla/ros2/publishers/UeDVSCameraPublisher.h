// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBaseCamera.h"
#include "carla/sensor/s11n/DVSEventArraySerializer.h"
#include "sensor_msgs/msg/PointCloud2PubSubTypes.h"

namespace carla {
namespace ros2 {

using UePointCloudFromBufferPublisherImpl =
    DdsPublisherImpl<sensor_msgs::msg::PointCloud2FromBuffer, sensor_msgs::msg::PointCloud2PubSubTypeFromBuffer>;

class UeDVSCameraPublisher : public UePublisherBaseCamera<sensor_msgs::msg::Image::allocator_type> {
public:
  UeDVSCameraPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                       std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeDVSCameraPublisher() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubsribersConnected interface
   */
  bool SubsribersConnected() const override;

  /**
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

protected:
  /**
   * Overrides UePublisherBaseCamera::encoding()
   */
  Encoding encoding() const override {
    return Encoding::BGR8;
  }

private:
  using DVSEvent = carla::sensor::data::DVSEvent;
  using DVSHeaderConst = carla::sensor::s11n::DVSEventArraySerializer::DVSHeader const;
  using DVSEventVectorAllocator = carla::sensor::data::SerializerVectorAllocator<DVSEvent>;

  std::shared_ptr<DVSHeaderConst> header_view(const carla::SharedBufferView buffer_view) {
    return std::shared_ptr<DVSHeaderConst>(buffer_view, reinterpret_cast<DVSHeaderConst *>(buffer_view.get()->data()));
  }

  std::vector<DVSEvent, DVSEventVectorAllocator> vector_view(const carla::SharedBufferView buffer_view) {
    return carla::sensor::data::buffer_data_accessed_by_vector<DVSEvent>(
        buffer_view, carla::sensor::s11n::DVSEventArraySerializer::header_offset);
  }

  void SetImageData(std::vector<DVSEvent, DVSEventVectorAllocator> &data_vector_view);
  void SetPointCloudData(carla::SharedBufferView &buffer_view);

  std::shared_ptr<UePointCloudFromBufferPublisherImpl> _point_cloud;
};
}  // namespace ros2
}  // namespace carla
