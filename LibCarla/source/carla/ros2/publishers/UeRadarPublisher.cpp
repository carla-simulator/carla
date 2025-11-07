// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeRadarPublisher.h"

#include "carla/ros2/impl/DdsPublisherImpl.h"
#include "carla/sensor/data/RadarData.h"

namespace carla {
namespace ros2 {

  struct RadarDetectionWithPosition {
    float x;
    float y;
    float z;
    carla::sensor::data::RadarDetection detection;
  };



UeRadarPublisher::UeRadarPublisher(std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
                                   std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBasePointCloud(sensor_actor_definition, transform_publisher) {}

std::vector<sensor_msgs::msg::PointField> UeRadarPublisher::GetPointFields() const {
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
  descriptor4.name("velocity");
  descriptor4.offset(12);
  descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor4.count(1);
  sensor_msgs::msg::PointField descriptor5;
  descriptor5.name("azimuth");
  descriptor5.offset(16);
  descriptor5.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor5.count(1);
  sensor_msgs::msg::PointField descriptor6;
  descriptor6.name("altitude");
  descriptor6.offset(20);
  descriptor6.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor6.count(1);
  sensor_msgs::msg::PointField descriptor7;
  descriptor7.name("depth");
  descriptor7.offset(24);
  descriptor7.datatype(sensor_msgs::msg::PointField__FLOAT32);
  descriptor7.count(1);
  return {descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6, descriptor7};
}

size_t UeRadarPublisher::GetMessagePointSize() const {
  return sizeof(RadarDetectionWithPosition);
}

void UeRadarPublisher::SetPointCloudDataFromBuffer(std::shared_ptr<HeaderTypeConst>,
                                                   std::vector<DataType, DataVectorAllocator> data_vector_view) {
  _point_cloud->Message().data().resize(data_vector_view.size() * sizeof(RadarDetectionWithPosition) / sizeof(uint8_t));
  auto point_clound_data_iter = reinterpret_cast<RadarDetectionWithPosition *>(_point_cloud->Message().data().data());
  for (auto const &data_view : data_vector_view) {
    RadarDetectionWithPosition ros_data;
    ros_data.x = data_view.depth * cosf(data_view.azimuth) * cosf(-data_view.altitude);
    ros_data.y = data_view.depth * sinf(-data_view.azimuth) * cosf(data_view.altitude);
    ros_data.z = data_view.depth * sinf(data_view.altitude);
    ros_data.detection = data_view;
    *point_clound_data_iter = ros_data;
    ++point_clound_data_iter;
  }
}
}  // namespace ros2
}  // namespace carla
