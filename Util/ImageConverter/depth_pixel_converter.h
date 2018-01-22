// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>

#include "image_converter_types.h"

namespace image_converter {
namespace detail {

  template <typename T>
  static inline float cast(T i) {
    return static_cast<float>(i);
  }

  static auto normalized_depth(boost::gil::rgb8_pixel_t &pixel) {
    const auto depth =
        (cast(pixel[Color::Red]) +
        (cast(pixel[Color::Green]) * 256.0f) +
        (cast(pixel[Color::Blue])  * 256.0f * 256.0f));
    return depth / cast(256 * 256 * 256 - 1);
  }

  static float logdepth(float depth) {
    return 1.0f + std::log(depth) / 5.70378f;
  }

  static float clamp(float value) {
    return std::max(std::min(value, 1.0f), 0.0f);
  }

  static void copy_to_pixel(float depth, boost::gil::rgb8_pixel_t &pixel) {
    const auto grayscale = Color(static_cast<uint8>(255.0f * depth));
    grayscale.copy_to_pixel(pixel);
  }

} // namespace detail

struct depth_pixel_converter {
  void operator()(boost::gil::rgb8_pixel_t &pixel) const {
    using namespace detail;
    copy_to_pixel(normalized_depth(pixel), pixel);
  }
};

struct logarithmic_depth_pixel_converter {
  void operator()(boost::gil::rgb8_pixel_t &pixel) const {
    using namespace detail;
    const auto depth = clamp(logdepth(normalized_depth(pixel)));
    copy_to_pixel(depth, pixel);
  }
};

} // namespace image_converter
