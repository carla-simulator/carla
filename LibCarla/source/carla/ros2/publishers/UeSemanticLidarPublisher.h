// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBasePointCloud.h"
#include "carla/sensor/s11n/SemanticLidarSerializer.h"

namespace carla {
namespace ros2 {

class UeSemanticLidarPublisher : public UePublisherBasePointCloud<carla::sensor::s11n::SemanticLidarHeaderView,
                                                                  carla::sensor::data::SemanticLidarDetection> {
public:
  UeSemanticLidarPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                           std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeSemanticLidarPublisher() = default;

protected:
  using SemanticLidarDetection = DataType;
  std::vector<sensor_msgs::msg::PointField> GetPointFields() const override;

  void SetPointCloudDataFromBuffer(std::shared_ptr<HeaderTypeConst> header_view,
                                   std::vector<DataType, DataVectorAllocator> vector_view) override;
};
}  // namespace ros2
}  // namespace carla
