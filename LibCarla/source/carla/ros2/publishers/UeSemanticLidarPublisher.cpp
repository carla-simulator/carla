// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeSemanticLidarPublisher.h"

#include "carla/Debug.h"
#include "carla/ros2/impl/fastdds/DdsPublisherImpl.h"

namespace carla {
namespace ros2 {

UeSemanticLidarPublisher::UeSemanticLidarPublisher(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBasePointCloud(sensor_actor_definition, transform_publisher) {}

std::vector<sensor_msgs::msg::PointField> UeSemanticLidarPublisher::GetPointFields() const {
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
  descriptor4.name("cos_inc_angle");
  descriptor4.offset(12);
  descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor4.count(1);
  sensor_msgs::msg::PointField descriptor5;
  descriptor5.name("object_idx");
  descriptor5.offset(16);
  descriptor5.datatype(sensor_msgs::msg::PointField__UINT32);
  descriptor5.count(1);
  sensor_msgs::msg::PointField descriptor6;
  descriptor6.name("object_tag");
  descriptor6.offset(20);
  descriptor6.datatype(sensor_msgs::msg::PointField__UINT32);
  descriptor6.count(1);

  return {descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6};
}

void UeSemanticLidarPublisher::SetPointCloudDataFromBuffer(std::shared_ptr<HeaderTypeConst> header_view,
                                                           std::vector<DataType, DataVectorAllocator> vector_view) {
  DEBUG_ASSERT_EQ(header_view->GetChannelCount(), 6);
  DEBUG_ASSERT_EQ(sizeof(SemanticLidarDetection), 4 * sizeof(float) + 2 * sizeof(uint32_t));

  _point_cloud->Message().data().resize(vector_view.size() * sizeof(SemanticLidarDetection) / sizeof(uint8_t));
  auto point_clound_data_iter = reinterpret_cast<SemanticLidarDetection *>(_point_cloud->Message().data().data());
  for (auto const &data_view : vector_view) {
    SemanticLidarDetection ros_data(CoordinateSystemTransform::TransformLinearAxixVector3D(data_view.point),
                                    data_view.cos_inc_angle, data_view.object_idx, data_view.object_tag);
    *point_clound_data_iter = ros_data;
    ++point_clound_data_iter;
  }
}
}  // namespace ros2
}  // namespace carla
