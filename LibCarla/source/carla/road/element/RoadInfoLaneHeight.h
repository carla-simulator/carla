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

  // The surface of a lane may be offset from the plane defined by the reference line and the
  // corresponding elevation and crossfall entries (e.g. pedestrian walkways are typically a few centimeters
  // above road level). The height record provides a simplified method to describe this offset by setting an
  // inner and outer offset from road level at discrete positions along the lane profile.
  class RoadInfoLaneHeight : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    RoadInfoLaneHeight(
        float s, // start position relative to the position of the preceding lane section
        float inner,
        float outer)
      : RoadInfo(s),
        _inner(inner),
        _outer(outer) {}

    float GetInner() const {
      return _inner;
    }

    float GetOuter() const {
      return _outer;
    }

  private:

    float _inner; // inner offset from road level

    float _outer; // outer offset from road level

  };

} // namespace element
} // namespace road
} // namespace carla
