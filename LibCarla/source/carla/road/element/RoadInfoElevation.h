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

  class RoadInfoElevation final : public RoadInfo {
  public:

    RoadInfoElevation(
        double s,
        double a, // elevation
        double b, // slope
        double c, // vertical_curvature
        double d) // curvature_change
      : RoadInfo(s),
        _elevation(a, b, c, d, s) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    /// @todo unused? you can use the polynomial directly.
    double Evaluate(const double dist, double &out_tan) const {
      out_tan = _elevation.Tangent(dist);
      return _elevation.Evaluate(dist);
    }

    const geom::CubicPolynomial &GetPolynomial() const {
      return _elevation;
    }

  private:

    const geom::CubicPolynomial _elevation;
  };

} // namespace element
} // namespace road
} // namespace carla
