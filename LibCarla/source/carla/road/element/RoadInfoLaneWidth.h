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

  // Lane Width RecordEach lane within a road’scross section can be provided
  // with severalwidth entries. At least one entry must be defined for each
  // lane, except for the center lane which is, per convention, of zero width.
  // Each entry is valid until a new entry is defined. If multiple
  // entries are defined for a lane, they must be listed in ascendingorder.
  class RoadInfoLaneWidth : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    RoadInfoLaneWidth(
        double s,
        int lane_id,
        double a,
        double b,
        double c,
        double d)
      : RoadInfo(s),
        _lane_id(lane_id),
        _width(a, b, c, d, s) {}

    int GetLaneId() const {
      return _lane_id;
    }

    const geom::CubicPolynomial &GetPolynomial() const {
      return _width;
    }

  private:

    using signed_id = int;

    signed_id _lane_id = 0;

    geom::CubicPolynomial _width;

  };

} // namespace element
} // namespace road
} // namespace carla
