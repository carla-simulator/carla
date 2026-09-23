// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
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

  /// One <lateralProfile><shape> record: at station s, the road surface
  /// height relative to the elevation profile is a cubic polynomial in
  /// (t - t_record), valid from t_record until the next record at the same
  /// station. Together the records describe the road cross-section (crown,
  /// banking); reconstructed real-world maps rely on them for crossing
  /// roads to agree on surface height inside junctions.
  class RoadInfoLateralShape final : public RoadInfo {
  public:

    RoadInfoLateralShape(
        double s,
        double t,
        double a,
        double b,
        double c,
        double d)
      : RoadInfo(s),
        _t(t),
        _shape(a, b, c, d, t) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    double GetT() const {
      return _t;
    }

    /// Evaluate the surface height offset at lateral position t (the
    /// polynomial is defined over t - t_record).
    double Evaluate(const double t) const {
      return _shape.Evaluate(t);
    }

    const geom::CubicPolynomial &GetPolynomial() const {
      return _shape;
    }

  private:

    const double _t;

    const geom::CubicPolynomial _shape;
  };

} // namespace element
} // namespace road
} // namespace carla
