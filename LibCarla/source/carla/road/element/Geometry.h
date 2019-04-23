// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/road/element/cephes/fresnel.h"

#include <cmath>
#include <stdexcept>

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
    DirectedPoint(const geom::Location &l, double t)
      : location(l),
        tangent(t) {}
    DirectedPoint(float x, float y, float z, double t)
      : location(x, y, z),
        tangent(t) {}

    geom::Location location = {0.0f, 0.0f, 0.0f};
    double tangent = 0.0; // [radians]
    double pitch = 0.0;   // [radians]

    void ApplyLateralOffset(double lateral_offset) {
      /// @todo Z axis??
      auto normal_x =  std::sin(tangent);
      auto normal_y = -std::cos(tangent);
      location.x += lateral_offset * normal_x;
      location.y += lateral_offset * normal_y;
    }

    friend bool operator==(const DirectedPoint &lhs, const DirectedPoint &rhs) {
      return (lhs.location == rhs.location) && (lhs.tangent == rhs.tangent);
    }
  };

  class Geometry {
  public:

    GeometryType GetType() const {
      return _type;
    }
    double GetLength() const {
      return _length;
    }
    double GetStartOffset() const {
      return _start_position_offset;
    }
    double GetHeading() const {
      return _heading;
    }

    const geom::Location &GetStartPosition() {
      return _start_position;
    }

    virtual ~Geometry() = default;

    virtual DirectedPoint PosFromDist(double dist) const = 0;

    virtual std::pair<float, float> DistanceTo(const geom::Location &p) const = 0;

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

  class GeometryLine final : public Geometry {
  public:

    GeometryLine(
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos)
      : Geometry(GeometryType::LINE, start_offset, length, heading, start_pos) {}

    DirectedPoint PosFromDist(double dist) const override {
      dist = geom::Math::Clamp(dist, 0.0, _length);
      DEBUG_ASSERT(_length > 0.0);
      DirectedPoint p(_start_position, _heading);
      p.location.x += dist * std::cos(p.tangent);
      p.location.y += dist * std::sin(p.tangent);
      return p;
    }

    /// Returns a pair containing:
    /// - @b first:  distance to the nearest point in this line from the
    ///              begining of the shape.
    /// - @b second: Euclidean distance from the nearest point in this line to
    /// p.
    ///   @param p point to calculate the distance
    std::pair<float, float> DistanceTo(const geom::Location &p) const override {
      return geom::Math::DistanceSegmentToPoint(
          p,
          _start_position,
          PosFromDist(_length).location);
    }

  };

  class GeometryArc final : public Geometry {
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
      dist = geom::Math::Clamp(dist, 0.0, _length);
      DEBUG_ASSERT(_length > 0.0);
      DEBUG_ASSERT(std::fabs(_curvature) > 1e-15);
      const double radius = 1.0 / _curvature;
      constexpr double pi_half = geom::Math::Pi<double>() / 2.0;
      DirectedPoint p(_start_position, _heading);
      p.location.x += radius * std::cos(p.tangent + pi_half);
      p.location.y += radius * std::sin(p.tangent + pi_half);
      p.tangent += dist * _curvature;
      p.location.x -= radius * std::cos(p.tangent + pi_half);
      p.location.y -= radius * std::sin(p.tangent + pi_half);
      return p;
    }

    /// Returns a pair containing:
    /// - @b first:  distance to the nearest point in this arc from the
    ///              begining of the shape.
    /// - @b second: Euclidean distance from the nearest point in this arc to p.
    ///   @param p point to calculate the distance
    std::pair<float, float> DistanceTo(const geom::Location &p) const override {
      return geom::Math::DistanceArcToPoint(
          p,
          _start_position,
          static_cast<float>(_length),
          static_cast<float>(_heading),
          static_cast<float>(_curvature));
    }

    double GetCurvature() const {
      return _curvature;
    }

  private:

    double _curvature;
  };

  class GeometrySpiral final : public Geometry {
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
      dist = geom::Math::Clamp(dist, 0.0, _length);
      DEBUG_ASSERT(_length > 0.0);
      DEBUG_ASSERT(std::fabs(_curve_end) > 1e-15);
      const double radius = 1.0 / _curve_end;
      const double extra_norm = 1.0 / std::sqrt(geom::Math::Pi<double>() / 2.0);
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

    /// @todo
    std::pair<float, float> DistanceTo(const geom::Location &) const override {
      throw_exception(std::runtime_error("not implemented"));
    }

  private:

    double _curve_start;
    double _curve_end;
  };

} // namespace element
} // namespace road
} // namespace carla
