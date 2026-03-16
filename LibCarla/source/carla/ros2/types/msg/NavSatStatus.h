#pragma once
#include <cstdint>

namespace carla {
namespace ros2 {
namespace msg {

struct NavSatStatus {
  static const uint8_t STATUS_NO_FIX = 255;
  static const uint8_t STATUS_FIX = 0;
  static const uint8_t STATUS_SBAS_FIX = 1;
  static const uint8_t STATUS_GBAS_FIX = 2;
  static const uint16_t SERVICE_GPS = 1;
  static const uint16_t SERVICE_GLONASS = 2;
  static const uint16_t SERVICE_COMPASS = 4;
  static const uint16_t SERVICE_GALILEO = 8;

  uint8_t status = 0;
  uint16_t service = 0;
};

} // namespace msg
} // namespace ros2
} // namespace carla
