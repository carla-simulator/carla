// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "image_converter_types.h"

namespace image_converter {
namespace detail {

  // Someday this will be in a config file or something.
  constexpr static Color LABEL_COLOR_MAP[] = {
      {  0u,   0u,   0u}, // None         =   0u,
      { 70u,  70u,  70u}, // Buildings    =   1u,
      {190u, 153u, 153u}, // Fences       =   2u,
      {250u, 170u, 160u}, // Other        =   3u,
      {220u,  20u,  60u}, // Pedestrians  =   4u,
      {153u, 153u, 153u}, // Poles        =   5u,
      {153u, 153u, 153u}, // RoadLines    =   6u,
      {128u,  64u, 128u}, // Roads        =   7u,
      {244u,  35u, 232u}, // Sidewalks    =   8u,
      {107u, 142u,  35u}, // Vegetation   =   9u,
      {  0u,   0u, 142u}, // Vehicles     =  10u,
      {102u, 102u, 156u}, // Walls        =  11u,
      {220u, 220u,   0u}  // TrafficSigns =  12u,
  };

} // namespace detail

struct label_pixel_converter {
  void operator()(boost::gil::rgb8_pixel_t &pixel) const {
    using namespace detail;
    constexpr auto size = sizeof(LABEL_COLOR_MAP)/sizeof(*LABEL_COLOR_MAP);
    const auto index = pixel[Color::Red] % size;
    LABEL_COLOR_MAP[index].copy_to_pixel(pixel);
  }
};

} // namespace image_converter
