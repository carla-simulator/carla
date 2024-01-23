// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  /// This record defines rules that can be applied to lanes to describe
  /// additonal properties not covered by the other attributes.
  class RoadInfoLaneRule final : public RoadInfo {
  public:

    RoadInfoLaneRule(
        double s,   // start position relative to the position of the preceding
                    // lane section
        std::string value)
      : RoadInfo(s),
        _value(std::move(value)) {}

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    /// Recommended values: No Stopping At Any Time, Disabled Parking, and Car
    /// Pool.
    const std::string &GetValue() const {
      return _value;
    }

  private:

    const std::string _value;
  };

} // namespace element
} // namespace road
} // namespace carla
