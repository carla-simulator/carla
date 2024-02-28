// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeLidarPublisher.h"

#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeLidarPublisher::UeLidarPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                                   std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBasePointCloud(sensor_actor_definition, transform_publisher) {}

std::vector<sensor_msgs::msg::PointField> UeLidarPublisher::GetPointFields() const {
  sensor_msgs::msg::PointField descriptor1;
  descriptor1.name("x");
  descriptor1.offset(0);
  descriptor1.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor1.count(1);
  sensor_msgs::msg::PointField descriptor2;
  descriptor2.name("y");
  descriptor2.offset(4);
  descriptor2.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor2.count(1);
  sensor_msgs::msg::PointField descriptor3;
  descriptor3.name("z");
  descriptor3.offset(8);
  descriptor3.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor3.count(1);
  sensor_msgs::msg::PointField descriptor4;
  descriptor4.name("intensity");
  descriptor4.offset(12);
  descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor4.count(1);

  return {descriptor1, descriptor2, descriptor3, descriptor4};
}

void UeLidarPublisher::SetPointCloudDataFromBuffer(std::shared_ptr<HeaderTypeConst> header_view,
                                                   std::vector<DataType, DataVectorAllocator> data_vector_view) {
  DEBUG_ASSERT_EQ(header_view->GetChannelCount(), 4);
  DEBUG_ASSERT_EQ(sizeof(LidarDetection), 4 * sizeof(float));

  _point_cloud->Message().data().resize(data_vector_view.size() * sizeof(LidarDetection) / sizeof(uint8_t));
  auto point_clound_data_iter = reinterpret_cast<LidarDetection *>(_point_cloud->Message().data().data());
  for (auto const &data_view : data_vector_view) {
    LidarDetection ros_data(CoordinateSystemTransform::TransformLinearAxixVector3D(data_view.point),
                            data_view.intensity);
    *point_clound_data_iter = ros_data;
    ++point_clound_data_iter;
  }
}
}  // namespace ros2
}  // namespace carla
