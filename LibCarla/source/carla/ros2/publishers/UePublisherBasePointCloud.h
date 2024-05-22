// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBaseSensor.h"
#include "sensor_msgs/msg/PointCloud2PubSubTypes.h"

namespace carla {
namespace ros2 {

using UePublisherPointCloudImpl =
    DdsPublisherImpl<sensor_msgs::msg::PointCloud2, sensor_msgs::msg::PointCloud2PubSubType>;

/**
  A Publisher base class for point cloud publisher sensors.
  Extends UePublisherBaseSensor by an point cloud publisher.
*/
template <class HEADER_TYPE, class DATA_TYPE>
class UePublisherBasePointCloud : public UePublisherBaseSensor {
public:
  UePublisherBasePointCloud(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                            std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UePublisherBasePointCloud() = default;

  /**
   * Implements ROS2NameRecord::Init() interface
   */
  bool Init(std::shared_ptr<DdsDomainParticipantImpl> domain_participant) override;

  /**
   * Implement PublisherInterface::Publish interface
   */
  bool Publish() override;
  /**
   * Implement PublisherInterface::SubscribersConnected interface
   */
  bool SubscribersConnected() const override;

  /**
   * Implements UePublisherBaseSensor::UpdateSensorData() interface
   */
  void UpdateSensorData(std::shared_ptr<carla::sensor::s11n::SensorHeaderSerializer::Header const> sensor_header,
                        const carla::SharedBufferView buffer_view) override;

protected:
  using DataType = DATA_TYPE;
  using HeaderTypeConst = HEADER_TYPE const;
  using DataVectorAllocator = carla::sensor::data::SerializerVectorAllocator<DataType>;

  std::shared_ptr<HeaderTypeConst> header_view(const carla::SharedBufferView buffer_view) const {
    return std::shared_ptr<HeaderTypeConst>(
        buffer_view, new HeaderTypeConst(reinterpret_cast<uint32_t const *>(buffer_view.get()->data())));
  }

  std::vector<DataType, DataVectorAllocator> vector_view(const carla::SharedBufferView buffer_view) const {
    auto header_view = UePublisherBasePointCloud::header_view(buffer_view);
    auto const header_offset = header_view->GetHeaderOffset();
    return carla::sensor::data::buffer_data_accessed_by_vector<DataType>(buffer_view, header_offset);
  }

  virtual std::vector<sensor_msgs::msg::PointField> GetPointFields() const = 0;
  virtual size_t GetMessagePointSize() const { return sizeof(DataType); }

  virtual void SetPointCloudDataFromBuffer(std::shared_ptr<HeaderTypeConst> header_view,
                                           std::vector<DataType, DataVectorAllocator> vector_view) = 0;

  std::shared_ptr<UePublisherPointCloudImpl> _point_cloud;
};
}  // namespace ros2
}  // namespace carla

#include "UePublisherBasePointCloud.cc"