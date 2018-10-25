// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "cephes/fresnel.h"

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

    void ApplyLateralOffset(double lateral_offset) {
      /// @todo Z axis??
      double normal_x = std::cos( tangent + carla::geom::Math::pi_half());
      double normal_y = std::sin(-tangent - carla::geom::Math::pi_half());

      location.x = location.x + (lateral_offset * normal_x);
      location.y = location.y + (lateral_offset * normal_y);
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
      assert(dist > 0);
      assert(_length > 0.0);

      DirectedPoint p(_start_position, _heading);

      p.location.x += dist * std::cos(p.tangent);
      p.location.y += dist * std::sin(p.tangent);

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
      assert(dist > 0);
      assert(_length > 0.0);
      assert(std::fabs(_curvature) > 1e-15);

      /*
      const double length = dist / _length;
      const double radius = 1.0 / _curvature;

      DirectedPoint p(_start_position, _heading);
      p.location.x -= radius * std::cos(p.tangent - geom::Math::pi_half());
      p.location.y -= radius * std::sin(p.tangent - geom::Math::pi_half());

      p.tangent += length * _curvature;
      p.location.x += radius * std::cos(p.tangent - geom::Math::pi_half());
      p.location.y += radius * std::sin(p.tangent - geom::Math::pi_half());
      return p;
      */

      /*
      DirectedPoint p;
      double start_angle = 0.0;

      if (_curvature > 0.0) start_angle = _heading - geom::Math::pi_half();
      else start_angle = _heading + geom::Math::pi_half();

      const double arc_radius = std::abs(1.0 / _curvature);
      const double stepAngle = dist / (1.0 / _curvature);

      double start_X = _start_position.x + std::cos(start_angle - geom::Math::pi()) * arc_radius;
      double start_Y = _start_position.y + std::sin(start_angle - geom::Math::pi()) * arc_radius;

      p.location.x = start_X + std::cos(start_angle + stepAngle) * arc_radius;
      p.location.y = start_Y + std::sin(start_angle + stepAngle) * arc_radius;

      p.tangent = start_angle + stepAngle + (_curvature <= 0.0 ? -geom::Math::pi_half() : geom::Math::pi_half());
      return p;
      */

      const double length = dist;
      const double radius = 1.0 / _curvature;
      DirectedPoint p(_start_position, _heading);

      p.location.x -= radius * std::cos(p.tangent - geom::Math::pi_half());
      p.location.y -= radius * std::sin(p.tangent - geom::Math::pi_half());

      p.tangent += length * _curvature;
      p.location.x += radius * std::cos(p.tangent - geom::Math::pi_half());
      p.location.y += radius * std::sin(p.tangent - geom::Math::pi_half());

      return p;
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
      // not working yet with negative values
      assert(dist > 0);
      assert(_length > 0.0);
      assert(std::fabs(_curve_end) > 1e-15);
      const double radius = 1.0 / _curve_end;
      const double extra_norm = 1.0 / std::sqrt(geom::Math::pi_half());
      const double norm = 1.0 / std::sqrt(2.0 * radius * _length);
      const double length = dist * norm;
      double S, C;
      fresnl(length * extra_norm, &S, &C);
      S /= (norm * extra_norm);
      C /= (norm * extra_norm);
      DirectedPoint p(_start_position, _heading);
      const double cos_a = std::cos(p.tangent);
      const double sin_a = std::sin(p.tangent);
      p.location.x += C * cos_a - S * sin_a;
      p.location.y += S * cos_a + C * sin_a;
      p.tangent += length * length;

      return p;
    }

  private:

    double _curve_start;
    double _curve_end;
  };

} // namespace element
} // namespace road
} // namespace carla
