#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "carla/ros2/types/msg/Header.h"
#include "carla/ros2/types/msg/RegionOfInterest.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CameraInfo {
  Header header;
  uint32_t height = 0;
  uint32_t width = 0;
  std::string distortion_model;
  std::vector<double> d;
  std::array<double, 9> k = {};
  std::array<double, 9> r = {};
  std::array<double, 12> p = {};
  uint32_t binning_x = 0;
  uint32_t binning_y = 0;
  RegionOfInterest roi;
};

} // namespace msg
} // namespace ros2
} // namespace carla
