// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/geom/Math.h"

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

    void ApplyLateralOffset(float lateral_offset);

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

    DirectedPoint PosFromDist(double dist) const override;

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

    DirectedPoint PosFromDist(double dist) const override;

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

    DirectedPoint PosFromDist(double dist) const override;

    std::pair<float, float> DistanceTo(const geom::Location &) const override;

  private:

    double _curve_start;
    double _curve_end;
  };

} // namespace element
} // namespace road
} // namespace carla
