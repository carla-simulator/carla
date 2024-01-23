// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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

  /// Lane Width RecordEach lane within a road’scross section can be provided
  /// with severalwidth entries. At least one entry must be defined for each
  /// lane, except for the center lane which is, per convention, of zero width.
  /// Each entry is valid until a new entry is defined. If multiple entries are
  /// defined for a lane, they must be listed in ascendingorder.
  class RoadInfoLaneWidth final : public RoadInfo {
  public:

    RoadInfoLaneWidth(
        double s,
        double a,
        double b,
        double c,
        double d)
      : RoadInfo(s),
        _width(a, b, c, d, s) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    const geom::CubicPolynomial &GetPolynomial() const {
      return _width;
    }

  private:

    const geom::CubicPolynomial _width;
  };

} // namespace element
} // namespace road
} // namespace carla
