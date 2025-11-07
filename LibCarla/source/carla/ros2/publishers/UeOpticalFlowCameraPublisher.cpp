// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "UeOpticalFlowCameraPublisher.h"

#include <cmath>

#include "carla/ros2/impl/DdsPublisherImpl.h"

template <typename T>
T CLAMP(const T& value, const T& low, const T& high) {
  return value < low ? low : (value > high ? high : value);
}

namespace carla {
namespace ros2 {

UeOpticalFlowCameraPublisher::UeOpticalFlowCameraPublisher(
    std::shared_ptr<carla::ros2::types::SensorActorDefinition> sensor_actor_definition,
    std::shared_ptr<TransformPublisher> transform_publisher)
  : UePublisherBaseCamera(sensor_actor_definition, transform_publisher) {}

void UeOpticalFlowCameraPublisher::SetImageDataFromBuffer(const carla::SharedBufferView buffer_view) {
  constexpr float pi = 3.1415f;
  constexpr float rad2ang = 360.0f / (2.0f * pi);
  auto data = carla::sensor::data::buffer_data_accessed_by_vector<float>(
      buffer_view, carla::sensor::s11n::ImageSerializer::header_offset);

  std::vector<uint8_t> image_data(image_size());
  size_t data_index = 0;
  for (size_t index = 0; index < data.size() && data_index < image_data.size() - 4; index += 2) {
    const float vx = data[index];
    const float vy = data[index + 1];
    float angle = 180.0f + std::atan2(vy, vx) * rad2ang;
    if (angle < 0) {
      angle = 360.0f + angle;
    }
    angle = std::fmod(angle, 360.0f);

    const float norm = std::sqrt(vx * vx + vy * vy);
    const float shift = 0.999f;
    const float a = 1.0f / std::log(0.1f + shift);
    const float intensity = CLAMP<float>(a * std::log(norm + shift), 0.0f, 1.0f);

    const float& H = angle;
    const float S = 1.0f;
    const float V = intensity;
    const float H_60 = H * (1.0f / 60.0f);

    const float C = V * S;
    const float X = C * (1.0f - std::abs(std::fmod(H_60, 2.0f) - 1.0f));
    const float m = V - C;

    float r = 0;
    float g = 0;
    float b = 0;
    const unsigned int angle_case = static_cast<const unsigned int>(H_60);
    switch (angle_case) {
      case 0:
        r = C;
        g = X;
        b = 0;
        break;
      case 1:
        r = X;
        g = C;
        b = 0;
        break;
      case 2:
        r = 0;
        g = C;
        b = X;
        break;
      case 3:
        r = 0;
        g = X;
        b = C;
        break;
      case 4:
        r = X;
        g = 0;
        b = C;
        break;
      case 5:
        r = C;
        g = 0;
        b = X;
        break;
      default:
        r = 1;
        g = 1;
        b = 1;
        break;
    }

    const uint8_t R = static_cast<uint8_t>((r + m) * 255.0f);
    const uint8_t G = static_cast<uint8_t>((g + m) * 255.0f);
    const uint8_t B = static_cast<uint8_t>((b + m) * 255.0f);

    image_data[data_index++] = B;
    image_data[data_index++] = G;
    image_data[data_index++] = R;
    image_data[data_index++] = 0;
  }
  DEBUG_ASSERT_EQ(data_index, image_data.size());
  _image->Message().data(std::move(image_data));
}
}  // namespace ros2
}  // namespace carla
