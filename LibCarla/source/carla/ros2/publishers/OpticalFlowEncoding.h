// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace carla {
namespace ros2 {

// HSV colour-wheel encoding for an optical-flow pixel:
//   hue   = atan2(vy, vx) + pi, mapped to [0, 360) degrees
//   sat   = 1
//   value = clamp(a * log(magnitude + shift), 0, 1)
// where shift = 0.999 and a = 1 / log(0.1 + shift), so the intensity reaches 1
// (full brightness) when magnitude == 0.1 and stays clamped at 1 above that.
// Small flows therefore brighten quickly while large ones plateau, matching
// the legacy ue5-dev wire format that existing ROS 2 subscribers depend on.
//
// Output is BGRA8: B, G, R, A with the alpha byte deliberately set to 0 to
// match the existing on-the-wire format ue5-dev clients are subscribed to.
//
// M_PI is used here, not std::numbers::pi_v, because this header is consumed
// by the carla-ros2-native ExternalProject in Ros2Native/, which does not
// configure CMAKE_CXX_STANDARD and therefore defaults to C++17 where <numbers>
// is unavailable.
[[nodiscard]] inline std::array<uint8_t, 4> EncodeFlowPixelToBgra(
    float vx, float vy) noexcept {
  constexpr float rad2ang = 360.0f / (2.0f * static_cast<float>(M_PI));
  float angle = 180.0f + std::atan2(vy, vx) * rad2ang;
  if (angle < 0.0f) {
    angle = 360.0f + angle;
  }
  angle = std::fmod(angle, 360.0f);

  const float norm = std::sqrt(vx * vx + vy * vy);
  constexpr float shift = 0.999f;
  const float a = 1.0f / std::log(0.1f + shift);
  const float intensity = std::clamp(a * std::log(norm + shift), 0.0f, 1.0f);

  const float h_60 = angle * (1.0f / 60.0f);
  const float saturation = 1.0f;
  const float value = intensity;
  const float c = value * saturation;
  const float x = c * (1.0f - std::abs(std::fmod(h_60, 2.0f) - 1.0f));
  const float m = value - c;

  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  switch (static_cast<unsigned int>(h_60)) {
    case 0:
      r = c;
      g = x;
      b = 0.0f;
      break;
    case 1:
      r = x;
      g = c;
      b = 0.0f;
      break;
    case 2:
      r = 0.0f;
      g = c;
      b = x;
      break;
    case 3:
      r = 0.0f;
      g = x;
      b = c;
      break;
    case 4:
      r = x;
      g = 0.0f;
      b = c;
      break;
    case 5:
      r = c;
      g = 0.0f;
      b = x;
      break;
    default:
      r = 1.0f;
      g = 1.0f;
      b = 1.0f;
      break;
  }

  return {
      static_cast<uint8_t>((b + m) * 255.0f),
      static_cast<uint8_t>((g + m) * 255.0f),
      static_cast<uint8_t>((r + m) * 255.0f),
      static_cast<uint8_t>(0),
  };
}

// Converts a height*width buffer of (vx, vy) float pairs into a height*width*4
// BGRA uint8 buffer suitable for sensor_msgs::Image with encoding "bgra8".
[[nodiscard]] inline std::vector<uint8_t> EncodeFlowImageToBgra(
    uint32_t height, uint32_t width, const float *data) {
  const size_t pixel_count = static_cast<size_t>(height) * static_cast<size_t>(width);
  std::vector<uint8_t> out;
  out.resize(pixel_count * 4u);
  for (size_t i = 0; i < pixel_count; ++i) {
    const float vx = data[i * 2u];
    const float vy = data[i * 2u + 1u];
    const auto bgra = EncodeFlowPixelToBgra(vx, vy);
    out[i * 4u + 0u] = bgra[0];
    out[i * 4u + 1u] = bgra[1];
    out[i * 4u + 2u] = bgra[2];
    out[i * 4u + 3u] = bgra[3];
  }
  return out;
}

}  // namespace ros2
}  // namespace carla
