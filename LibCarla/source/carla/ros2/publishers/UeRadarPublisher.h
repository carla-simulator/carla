// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/UePublisherBasePointCloud.h"
#include "carla/sensor/s11n/RadarSerializer.h"

namespace carla {
namespace ros2 {

/// A view over the header of a Lidar measurement.
class RadarDummyHeaderView {
public:
  size_t GetHeaderOffset() const {
    return 0u;
  }
  float GetHorizontalAngle() const {
    return 0.f;
  }
  uint32_t GetChannelCount() const {
    return 0u;
  }

  uint32_t GetPointCount(size_t) const {
    return 0u;
  }

  size_t GetDataSize() const {
    return 0u;
  }

  RadarDummyHeaderView(const uint32_t *) {}
};

class UeRadarPublisher : public UePublisherBasePointCloud<RadarDummyHeaderView, carla::sensor::data::RadarDetection> {
public:
  UeRadarPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                   std::shared_ptr<TransformPublisher> transform_publisher);
  virtual ~UeRadarPublisher() = default;

protected:
  std::vector<sensor_msgs::msg::PointField> GetPointFields() const override;

  void SetPointCloudDataFromBuffer(std::shared_ptr<HeaderTypeConst> header_view,
                                   std::vector<DataType, DataVectorAllocator> vector_view) override;
};
}  // namespace ros2
}  // namespace carla
