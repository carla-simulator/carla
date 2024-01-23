// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/CubicPolynomial.h"
#include "carla/geom/Rtree.h"

namespace carla {
namespace road {
namespace element {

  enum class GeometryType : unsigned int {
    LINE,
    ARC,
    SPIRAL,
    POLY3,
    POLY3PARAM
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
        _start_position(start_pos) {}

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
    ///              beginning of the shape.
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
    ///              beginning of the shape.
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

  class GeometryPoly3 final : public Geometry {
  public:

    GeometryPoly3(
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos,
        double a,
        double b,
        double c,
        double d)
      : Geometry(GeometryType::POLY3, start_offset, length, heading, start_pos),
        _a(a),
        _b(b),
        _c(c),
        _d(d) {
      _poly.Set(a, b, c, d);
      PreComputeSpline();
    }

    double Geta() const {
      return _a;
    }
    double Getb() const {
      return _b;
    }
    double Getc() const {
      return _c;
    }
    double Getd() const {
      return _d;
    }

    DirectedPoint PosFromDist(double dist) const override;

    std::pair<float, float> DistanceTo(const geom::Location &) const override;

  private:

    geom::CubicPolynomial _poly;

    double _a;
    double _b;
    double _c;
    double _d;

    struct RtreeValue {
      double u = 0;
      double v = 0;
      double s = 0;
      double t = 0;
    };
    using Rtree = geom::SegmentCloudRtree<RtreeValue, 1>;
    using TreeElement = Rtree::TreeElement;
    Rtree _rtree;
    void PreComputeSpline();
  };

  class GeometryParamPoly3 final : public Geometry {
  public:

    GeometryParamPoly3(
        double start_offset,
        double length,
        double heading,
        const geom::Location &start_pos,
        double aU,
        double bU,
        double cU,
        double dU,
        double aV,
        double bV,
        double cV,
        double dV,
        bool arcLength)
      : Geometry(GeometryType::POLY3PARAM, start_offset, length, heading, start_pos),
        _aU(aU),
        _bU(bU),
        _cU(cU),
        _dU(dU),
        _aV(aV),
        _bV(bV),
        _cV(cV),
        _dV(dV),
        _arcLength(arcLength) {
        _polyU.Set(aU, bU, cU, dU);
        _polyV.Set(aV, bV, cV, dV);
        PreComputeSpline();
    }

    double GetaU() const {
      return _aU;
    }
    double GetbU() const {
      return _bU;
    }
    double GetcU() const {
      return _cU;
    }
    double GetdU() const {
      return _dU;
    }
    double GetaV() const {
      return _aV;
    }
    double GetbV() const {
      return _bV;
    }
    double GetcV() const {
      return _cV;
    }
    double GetdV() const {
      return _dV;
    }

    DirectedPoint PosFromDist(double dist) const override;

    std::pair<float, float> DistanceTo(const geom::Location &) const override;

  private:

    geom::CubicPolynomial _polyU;
    geom::CubicPolynomial _polyV;
    double _aU;
    double _bU;
    double _cU;
    double _dU;
    double _aV;
    double _bV;
    double _cV;
    double _dV;
    bool _arcLength;

    struct RtreeValue {
      double u = 0;
      double v = 0;
      double s = 0;
      double t_u = 0;
      double t_v = 0;
    };
    using Rtree = geom::SegmentCloudRtree<RtreeValue, 1>;
    using TreeElement = Rtree::TreeElement;
    Rtree _rtree;
    void PreComputeSpline();
  };

} // namespace element
} // namespace road
} // namespace carla
