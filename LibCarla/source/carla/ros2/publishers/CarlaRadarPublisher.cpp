// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRadarPublisher.h"

#include "carla/sensor/data/RadarData.h"

namespace carla {
namespace ros2 {

  struct RadarDetectionWithPosition {
    float x;
    float y;
    float z;
    carla::sensor::data::RadarDetection detection;
  };

const size_t CarlaRadarPublisher::GetPointSize() {
  return sizeof(RadarDetectionWithPosition);
}

std::vector<sensor_msgs::msg::PointField> CarlaRadarPublisher::GetFields() {

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

std::vector<uint8_t> CarlaRadarPublisher::ComputePointCloud(uint32_t height, uint32_t width, float *data) {
  std::vector<uint8_t> vector_data;
  const size_t size = width * sizeof(RadarDetectionWithPosition);
  vector_data.resize(size);
  RadarDetectionWithPosition* radar_data = (RadarDetectionWithPosition*)&vector_data[0];
  carla::sensor::data::RadarDetection* detection_data = (carla::sensor::data::RadarDetection*)data;
  for (size_t i = 0; i < width; ++i, ++radar_data, ++detection_data) {
    radar_data->x = detection_data->depth * cosf(detection_data->azimuth) * cosf(-detection_data->altitude);
    radar_data->y = detection_data->depth * sinf(-detection_data->azimuth) * cosf(detection_data->altitude);
    radar_data->z = detection_data->depth * sinf(detection_data->altitude);
    radar_data->detection = *detection_data;
  }
  return vector_data;
}

}  // namespace ros2
}  // namespace carla
