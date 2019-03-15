// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h"
#include <string>
#include <vector>
#include "carla/road/RoadTypes.h"

namespace carla {
namespace road {
namespace general {

  class Validity : private MovableNonCopyable {
  public:

    Validity(
        int32_t parent_id,
        int32_t from_lane,
        int32_t to_lane)
        : _parent_id(parent_id),
          _from_lane(from_lane),
          _to_lane(to_lane) {}

  private:

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    int32_t _parent_id;
    int32_t _from_lane;
    int32_t _to_lane;
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
  };



} // general
} // road
} // carla
