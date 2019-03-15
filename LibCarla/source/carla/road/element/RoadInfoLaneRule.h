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

  // This record defines rules that can be applied to lanes to describe additonal properties
  // not covered by the other attributes.
  class RoadInfoLaneRule : public RoadInfo {

  public:

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    RoadInfoLaneRule(
        float s,   // start position relative to the position of the preceding lane section
        std::string value)
      : RoadInfo(s),
        _value(value) {}

    std::string GetValue() const {
      return _value;
    }

  private:

    std::string _value; // Recommended values: No Stopping At Any Time, Disabled Parking and Car Pool

  };

} // namespace element
} // namespace road
} // namespace carla
