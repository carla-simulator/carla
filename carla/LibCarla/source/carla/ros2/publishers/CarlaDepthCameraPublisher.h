// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaRGBCameraPublisher.h"

namespace carla {
namespace ros2 {

// Depth, SS, IS, Normals and the base RGB camera share the same on-the-wire
// shape: 4-channel BGRA uint8 packed by ImageSerializer on the UE side. The
// pixel content differs (depth packed as 24-bit + alpha, semantic labels in
// the red channel, instance ids encoded across BGR, world-space normals in
// BGR), but the ROS 2 sensor_msgs/Image fields (encoding, step, channels)
// are identical, so they alias the same publisher.
using CarlaDepthCameraPublisher = CarlaRGBCameraPublisher;

}  // namespace ros2
}  // namespace carla
