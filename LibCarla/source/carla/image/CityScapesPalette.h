// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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
        {  0u,   0u,   0u}, // unlabeled     =   0u,
        { 70u,  70u,  70u}, // building      =   1u,
        {190u, 153u, 153u}, // fence         =   2u,
        {250u, 170u, 160u}, // other         =   3u,
        {220u,  20u,  60u}, // pedestrian    =   4u,
        {153u, 153u, 153u}, // pole          =   5u,
        {157u, 234u,  50u}, // road line     =   6u,
        {128u,  64u, 128u}, // road          =   7u,
        {244u,  35u, 232u}, // sidewalk      =   8u,
        {107u, 142u,  35u}, // vegetation    =   9u,
        {  0u,   0u, 142u}, // car           =  10u,
        {102u, 102u, 156u}, // wall          =  11u,
        {220u, 220u,   0u}, // traffic sign  =  12u,
        // {  0u,   0u,  70u}, // truck
        // {  0u,   0u,  90u}, // caravan
        // {  0u,   0u, 110u}, // trailer
        // {  0u,   0u, 142u}, // license plate
        // {  0u,   0u, 230u}, // motorcycle
        // {  0u,  60u, 100u}, // bus
        // {  0u,  80u, 100u}, // train
        // { 70u, 130u, 180u}, // sky
        // { 81u,   0u,  81u}, // ground
        // {111u,  74u,   0u}, // dynamic
        // {119u,  11u,  32u}, // bicycle
        // {150u, 100u, 100u}, // bridge
        // {150u, 120u,  90u}, // tunnel
        // {152u, 251u, 152u}, // terrain
        // {153u, 153u, 153u}, // polegroup
        // {180u, 165u, 180u}, // guard rail
        // {230u, 150u, 140u}, // rail track
        // {250u, 170u,  30u}, // traffic light
        // {250u, 170u, 160u}, // parking
        // {255u,   0u,   0u}, // rider
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
