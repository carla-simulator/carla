#pragma once
#include <cstdint>
#include "carla/ros2/types/msg/Header.h"

namespace carla {
namespace ros2 {
namespace msg {

struct CarlaEgoVehicleControl {
  Header header;
  float throttle = 0.0f;
  float steer = 0.0f;
  float brake = 0.0f;
  bool hand_brake = false;
  bool reverse = false;
  int32_t gear = 0;
  bool manual_gear_shift = false;
};

} // namespace msg
} // namespace ros2
} // namespace carla
