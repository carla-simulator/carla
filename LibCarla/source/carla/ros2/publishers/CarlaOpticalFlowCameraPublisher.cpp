// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/ros2/publishers/CarlaOpticalFlowCameraPublisher.h"

#include "carla/ros2/publishers/OpticalFlowEncoding.h"

namespace carla {
namespace ros2 {

std::vector<uint8_t> CarlaOpticalFlowCameraPublisher::ComputeImage(
    uint32_t height, uint32_t width, const uint8_t *data) const {
  // The optical-flow buffer arriving from ImageSerializer holds two floats
  // (vx, vy) per pixel. ROS2.cpp's OpticalFlowCamera dispatch passes the
  // post-header pointer through CarlaCameraPublisher::WriteImage as
  // const uint8_t*, so reinterpret it back to its real layout here.
  const auto *flow = reinterpret_cast<const float *>(data);
  return EncodeFlowImageToBgra(height, width, flow);
}

}  // namespace ros2
}  // namespace carla
