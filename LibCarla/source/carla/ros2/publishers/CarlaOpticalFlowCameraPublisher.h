// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/ros2/publishers/CarlaCameraPublisher.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace carla {
namespace ros2 {

// Non-alias subclass: the UE optical flow sensor produces a height*width buffer
// of (vx, vy) float pairs (8 bytes per pixel) which CARLA encodes on the wire
// as a 4-channel BGRA uint8 image via the HSV colour wheel. The conversion is
// the same one that ue5-dev shipped before this refactor; it now lives in
// OpticalFlowEncoding.h so the math is unit-testable without FastDDS, and the
// publisher only has to call ComputeImage(...) to plug it into the unified
// base.
class CarlaOpticalFlowCameraPublisher : public CarlaCameraPublisher {
public:
  CarlaOpticalFlowCameraPublisher(std::string base_topic_name, std::string frame_id)
    : CarlaCameraPublisher(std::move(base_topic_name), std::move(frame_id)) {}

protected:
  [[nodiscard]] uint8_t GetChannels() const override { return 4; }
  [[nodiscard]] std::string GetEncoding() const override { return "bgra8"; }
  [[nodiscard]] std::vector<uint8_t> ComputeImage(
      uint32_t height, uint32_t width, const uint8_t *data) const override;
};

}  // namespace ros2
}  // namespace carla
