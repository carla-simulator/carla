// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include <string>

namespace carla {
namespace road {
namespace element {

  // This record defines access restrictions for certain types of road users. The record can be used to
  // complement restrictions resulting from signs or signals in order to control the traffic flow in a scenario.
  // Each entry is valid in direction of the increasing s co-ordinate until a new entry is defined. If multiple
  // entries are defined, they must be listed in increasing order.
  class RoadInfoLaneAccess : public RoadInfo {

  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoLaneAccess(
        float s,   // start position relative to the position of the preceding lane section
        std::string restriction)
      : RoadInfo(s),
        _restriction(restriction) {}

    std::string GetRestriction() const {
      return _restriction;
    }

  private:

    std::string _restriction; // Examples: Simulator, Autonomous Traffic, Pedestrian and None

  };

} // namespace element
} // namespace road
} // namespace carla
