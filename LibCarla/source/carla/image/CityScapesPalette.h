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
        {100u,  40u,  40u}, // fence         =   2u,
        { 55u,  90u,  80u}, // other         =   3u,
        {220u,  20u,  60u}, // pedestrian    =   4u,
        {153u, 153u, 153u}, // pole          =   5u,
        {157u, 234u,  50u}, // road line     =   6u,
        {128u,  64u, 128u}, // road          =   7u,
        {244u,  35u, 232u}, // sidewalk      =   8u,
        {107u, 142u,  35u}, // vegetation    =   9u,
        {  0u,   0u, 142u}, // vehicle       =  10u,
        {102u, 102u, 156u}, // wall          =  11u,
        {220u, 220u,   0u}, // traffic sign  =  12u,
        { 70u, 130u, 180u}, // sky           =  13u,
        { 81u,   0u,  81u}, // ground        =  14u,
        {150u, 100u, 100u}, // bridge        =  15u,
        {230u, 150u, 140u}, // rail track    =  16u,
        {180u, 165u, 180u}, // guard rail    =  17u,
        {250u, 170u,  30u}, // traffic light =  18u,
        {110u, 190u, 160u}, // static        =  19u,
        {170u, 120u,  50u}, // dynamic       =  20u,
        { 45u,  60u, 150u}, // water         =  21u,
        {145u, 170u, 100u}, // terrain       =  22u,
        // {  0u,   0u,  70u}, // truck
        // {  0u,   0u,  90u}, // caravan
        // {  0u,   0u, 110u}, // trailer
        // {  0u,   0u, 142u}, // license plate
        // {  0u,   0u, 230u}, // motorcycle
        // {  0u,  60u, 100u}, // bus
        // {  0u,  80u, 100u}, // train
        // {119u,  11u,  32u}, // bicycle
        // {150u, 120u,  90u}, // tunnel
        // {153u, 153u, 153u}, // polegroup
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
