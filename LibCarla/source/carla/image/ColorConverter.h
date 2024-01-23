// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/image/BoostGil.h"
#include "carla/image/CityScapesPalette.h"

namespace carla {
namespace image {

  class ColorConverter {
  public:

    struct LogarithmicLinear {
      template <typename DstPixelT>
      void operator()(const boost::gil::gray32fc_pixel_t &src, DstPixelT &dst) const {
        using namespace boost::gil;
        const float value = 1.0f + std::log(src[0u]) / 5.70378f;
        const float clamped = std::max(std::min(value, 1.0f), 0.005f);
        color_convert(gray32fc_pixel_t{clamped}, dst);
      }
    };

    struct Depth {
      template <typename SrcPixelT, typename DstPixelT>
      void operator()(const SrcPixelT &src, DstPixelT &dst) const {
        using namespace boost::gil;
        static_assert(
            sizeof(typename color_space_type<SrcPixelT>::type) == sizeof(uint8_t),
            "Invalid pixel type.");
        const float depth =
             get_color(src, red_t()) +
            (get_color(src, green_t()) * 256) +
            (get_color(src, blue_t())  * 256 * 256);
        const float normalized = depth / static_cast<float>(256 * 256 * 256 - 1);
        color_convert(gray32fc_pixel_t{normalized}, dst);
      }
    };

    struct LogarithmicDepth {};

    struct CityScapesPalette {
      template <typename SrcPixelT, typename DstPixelT>
      void operator()(const SrcPixelT &src, DstPixelT &dst) const {
        using namespace boost::gil;
        static_assert(
            sizeof(typename color_space_type<SrcPixelT>::type) == sizeof(uint8_t),
            "Invalid pixel type.");
        const auto color = image::CityScapesPalette::GetColor(get_color(src, red_t()));
        color_convert(rgb8c_pixel_t{color[0u], color[1u], color[2u]}, dst);
      }
    };
  };

} // namespace image
} // namespace carla
