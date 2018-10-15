// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"

#include <cmath>

namespace carla {
namespace road {
namespace element {

  enum class GeometryType : unsigned int {
    LINE,
    ARC,
    SPIRAL
  };

  struct DirectedPoint {

    DirectedPoint()
      : location(0, 0, 0),
        tangent(0) {}
    DirectedPoint(geom::Location l, double t)
      : location(l),
        tangent(t) {}
    DirectedPoint(double x, double y, double z, double t)
      : location(x, y, z),
        tangent(t) {}

    geom::Location location = {0, 0, 0};
    double tangent = 0; // [radians]
    bool valid = true;

    static DirectedPoint Invalid() {
      DirectedPoint d;
      d.valid = false;
      return d;
    }

    friend bool operator==(const DirectedPoint &lhs, const DirectedPoint &rhs) {
      return (lhs.location == rhs.location) && (lhs.tangent == rhs.tangent);
    }
  };

  class Geometry {
  public:

    GeometryType GetType() {
      return _type;
    }
    double GetLength() {
      return _length;
    }
    double GetStartOffset() {
      return _start_position_offset;
    }
    double GetHeading() {
      return _heading;
    }
    geom::Location &GetStartPosition() {
      return _start_position;
    }
    virtual ~Geometry() = default;

    virtual DirectedPoint PosFromDist(double dist) const = 0;

  protected:

    Geometry(
        GeometryType type,
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos)
      : _type(type),
        _length(length),
        _start_position_offset(start_offset),
        _heading(heading),
        _start_position(start_pos)
    {}

  protected:

    GeometryType _type;             // geometry type
    double _length;                 // length of the road section [meters]

    double _start_position_offset;  // s-offset [meters]
    double _heading;                // start orientation [radians]

    geom::Location _start_position; // [meters]
  };

  class GeometryLine : public Geometry {
  public:

    GeometryLine(
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos)
      : Geometry(GeometryType::LINE, start_offset, length, heading, start_pos) {}

    DirectedPoint PosFromDist(const double dist) const override {
      DirectedPoint p(_start_position, _heading);
      p.location.x = dist * std::cos(_heading);
      p.location.y = dist * std::sin(_heading);
      return p;
    }
  };

  class GeometryArc : public Geometry {
  public:

    GeometryArc(
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos,
        double curv)
      : Geometry(GeometryType::ARC, start_offset, length, heading, start_pos),
        _curvature(curv) {}

    DirectedPoint PosFromDist(double dist) const override {
      (void) dist;
      return DirectedPoint();
      assert(_length > 0.0);
    }

    double GetCurvature() {
      return _curvature;
    }

  private:

    double _curvature;
  };

  class GeometrySpiral : public Geometry {
  public:

    GeometrySpiral(
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos,
        double curv_s,
        double curv_e)
      : Geometry(GeometryType::SPIRAL, start_offset, length, heading, start_pos),
        _curve_start(curv_s),
        _curve_end(curv_e) {}

    double GetCurveStart() {
      return _curve_start;
    }

    double GetCurveEnd() {
      return _curve_end;
    }

    DirectedPoint PosFromDist(double dist) const override {
      // to implement
      (void) dist;
      return DirectedPoint();
      assert(_length > 0.0);
    }

  private:

    double _curve_start;
    double _curve_end;
  };

} // namespace element
} // namespace road
} // namespace carla
