// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>
#include <type_traits>

#include <boost/gil/typedefs.hpp>

namespace image_converter {

  // ===========================================================================
  // -- Basic types ------------------------------------------------------------
  // ===========================================================================

  using uint8 = uint8_t;

  using uint32 = uint_fast32_t;

  // ===========================================================================
  // -- Color ------------------------------------------------------------------
  // ===========================================================================

  struct Color {
    enum Channels {
      Red,
      Green,
      Blue,
      NUMBER_OF_CHANNELS
    };

    constexpr Color(uint8 R, uint8 G, uint8 B) : data{R, G, B} {}

    constexpr explicit Color(uint8 Grey) : data{Grey, Grey, Grey} {}

    constexpr const uint8 &operator[](uint_fast8_t i) const {
      return data[i];
    }

    constexpr uint8 &operator[](uint_fast8_t i) {
      return data[i];
    }

    void copy_to_pixel(boost::gil::rgb8_pixel_t &pixel) const {
      pixel[Red]   = data[Red];
      pixel[Green] = data[Green];
      pixel[Blue]  = data[Blue];
    }

    uint8 data[NUMBER_OF_CHANNELS];
  };

} // namespace image_converter
