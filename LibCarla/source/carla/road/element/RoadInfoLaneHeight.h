// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include "carla/geom/CubicPolynomial.h"

namespace carla {
namespace road {
namespace element {

  /// The surface of a lane may be offset from the plane defined by the
  /// reference line and the corresponding elevation and crossfall entries (e.g.
  /// pedestrian walkways are typically a few centimeters above road level). The
  /// height record provides a simplified method to describe this offset by
  /// setting an inner and outer offset from road level at discrete positions
  /// along the lane profile.
  class RoadInfoLaneHeight final : public RoadInfo {
  public:

    RoadInfoLaneHeight(
        float s, // start position relative to the position of the preceding lane section
        float inner,
        float outer)
      : RoadInfo(s),
        _inner(inner),
        _outer(outer) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    /// Inner offset from road level.
    float GetInner() const {
      return _inner;
    }

    /// Outer offset from road level.
    float GetOuter() const {
      return _outer;
    }

  private:

    const float _inner;

    const float _outer;
  };

} // namespace element
} // namespace road
} // namespace carla
