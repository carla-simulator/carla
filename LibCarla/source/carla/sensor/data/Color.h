// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/Color.h"
#include "carla/rpc/FloatColor.h"

#include <cstdint>

namespace carla {
namespace sensor {
namespace data {

#pragma pack(push, 1)
  /// A 32-bit BGRA color.
  struct Color {
    Color() = default;
    Color(const Color &) = default;

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255u)
      : b(b), g(g), r(r), a(a) {}

    Color &operator=(const Color &) = default;

    bool operator==(const Color &rhs) const  {
      return (r == rhs.r) && (g == rhs.g) && (b == rhs.b);
    }

    bool operator!=(const Color &rhs) const  {
      return !(*this == rhs);
    }

    operator rpc::Color() const {
      return {r, g, b};
    }
    operator rpc::FloatColor() const {
      return {r/255.f, g/255.f, b/255.f, a/255.f};
    }

    uint8_t b = 0u;
    uint8_t g = 0u;
    uint8_t r = 0u;
    uint8_t a = 0u;
    MSGPACK_DEFINE_ARRAY(r, g, b, a);
  };
#pragma pack(pop)

static_assert(sizeof(Color) == sizeof(uint32_t), "Invalid color size!");

#pragma pack(push, 1)
  /// Optical flow pixel format. 2 channel float data.
  struct OpticalFlowPixel {
    OpticalFlowPixel() = default;
    OpticalFlowPixel(const OpticalFlowPixel &) = default;

    OpticalFlowPixel(float x, float y)
      : x(x), y(y) {}

    OpticalFlowPixel &operator=(const OpticalFlowPixel &) = default;

    bool operator==(const OpticalFlowPixel &rhs) const  {
      return (x == rhs.x) && (y == rhs.y);
    }

    bool operator!=(const OpticalFlowPixel &rhs) const  {
      return !(*this == rhs);
    }

    float x = 0;
    float y = 0;
    MSGPACK_DEFINE_ARRAY(x, y);
  };
#pragma pack(pop)

#pragma pack(push, 1)
  /// Distance pixel format: a single float channel holding a metric distance in
  /// METRES. Produced by the path-traced distance sensor
  /// (sensor.camera.rt_lens_distance) as the Euclidean distance from the camera
  /// origin to the primary hit along that pixel's lens ray. Rays that hit
  /// nothing report a very large value rather than 0.
  struct DistancePixel {
    DistancePixel() = default;
    DistancePixel(const DistancePixel &) = default;

    explicit DistancePixel(float distance)
      : distance(distance) {}

    DistancePixel &operator=(const DistancePixel &) = default;

    bool operator==(const DistancePixel &rhs) const {
      return distance == rhs.distance;
    }

    bool operator!=(const DistancePixel &rhs) const {
      return !(*this == rhs);
    }

    float distance = 0.0f;
    MSGPACK_DEFINE_ARRAY(distance);
  };
#pragma pack(pop)

static_assert(sizeof(DistancePixel) == sizeof(float), "Invalid distance pixel size!");

} // namespace data
} // namespace sensor
} // namespace carla
