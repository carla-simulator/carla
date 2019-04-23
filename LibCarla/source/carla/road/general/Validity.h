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
        uint32_t parent_id,
        road::LaneId from_lane,
        road::LaneId to_lane)
      : _parent_id(parent_id),
        _from_lane(from_lane),
        _to_lane(to_lane) {}

  private:

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-private-field"
#endif
    uint32_t _parent_id;
    road::LaneId _from_lane;
    road::LaneId _to_lane;
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
  };

} // general
} // road
} // carla
