// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace image {
namespace detail {

    static constexpr
#if __cplusplus >= 201703L // C++17
    inline
#endif
    // Please update documentation if you change this.
    uint8_t CITYSCAPES_PALETTE_MAP[][3u] = {
        {  0u,   0u,   0u},   // unlabeled     =   0u
        // cityscape
        {128u,  64u, 128u},   // road          =   1u
        {244u,  35u, 232u},   // sidewalk      =   2u
        { 70u,  70u,  70u},   // building      =   3u
        {102u, 102u, 156u},   // wall          =   4u
        {190u, 153u, 153u},   // fence         =   5u
        {153u, 153u, 153u},   // pole          =   6u
        {250u, 170u,  30u},   // traffic light =   7u
        {220u, 220u,   0u},   // traffic sign  =   8u
        {107u, 142u,  35u},   // vegetation    =   9u
        {152u, 251u, 152u},   // terrain       =  10u
        { 70u, 130u, 180u},   // sky           =  11u
        {220u,  20u,  60u},   // pedestrian    =  12u
        {255u,   0u,   0u},   // rider         =  13u
        {  0u,   0u, 142u},   // Car           =  14u
        {  0u,   0u,  70u},   // truck         =  15u
        {  0u,  60u, 100u},   // bus           =  16u
        {  0u,  80u, 100u},   // train         =  17u
        {  0u,   0u, 230u},   // motorcycle    =  18u
        {119u,  11u,  32u},   // bicycle       =  19u
        // custom
        {110u, 190u, 160u},   // static        =  20u
        {170u, 120u,  50u},   // dynamic       =  21u
        { 55u,  90u,  80u},   // other         =  22u
        { 45u,  60u, 150u},   // water         =  23u
        {157u, 234u,  50u},   // road line     =  24u
        { 81u,   0u,  81u},   // ground        =  25u
        {150u, 100u, 100u},   // bridge        =  26u
        {230u, 150u, 140u},   // rail track    =  27u
        {180u, 165u, 180u},   // guard rail    =  28u
        {180u, 130u,  70u},   // rock          =  29u
      };

} // namespace detail

  class CityScapesPalette {
  public:

    static constexpr auto GetNumberOfTags() {
      return sizeof(detail::CITYSCAPES_PALETTE_MAP) /
          sizeof(*detail::CITYSCAPES_PALETTE_MAP);
    }

    /// Return an RGB uint8_t array.
    ///
    /// @warning It overflows if @a tag is greater than GetNumberOfTags().
    static constexpr auto GetColor(uint8_t tag) {
      return detail::CITYSCAPES_PALETTE_MAP[tag % GetNumberOfTags()];
    }
  };

} // namespace image
} // namespace carla
