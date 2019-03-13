// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadElevationInfo : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    RoadElevationInfo(
        double d,
        double start_position,
        double elevation, // a
        double slope, // b
        double vertical_curvature,
        double curvature_change)
      : RoadInfo(d),
        _start_position(start_position),
        _elevation(elevation),
        _slope(slope),
        _vertical_curvature(vertical_curvature),
        _curvature_change(curvature_change) {}

    double Evaluate(const double dist, double *out_tan) const {
      const double t = dist - _start_position;
      const double pos = _elevation +
          _slope * t +
          _vertical_curvature * t * t +
          _curvature_change * t * t * t;

      if (out_tan) {
        *out_tan = _slope + t *
            (2 * _vertical_curvature + t * 3 * _curvature_change);
      }

      return pos;
    }

    double GetStartPosition() const {
      return _start_position;
    }

    double GetElevation() const {
      return _elevation;
    }

    double GetSlope() const {
      return _slope;
    }

    double GetVerticalCurvature() const {
      return _vertical_curvature;
    }

    double GetCurvatureChange() const {
      return _curvature_change;
    }

  private:

    double _start_position;      // (S) start position(s - offset)[meters]
    double _elevation;           // (A) elevation [meters]
    double _slope;               // (B)
    double _vertical_curvature;  // (C)
    double _curvature_change;    // (D)
  };

} // namespace element
} // namespace road
} // namespace carla
