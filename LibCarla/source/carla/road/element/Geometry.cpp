// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/road/element/Geometry.h"

#include "carla/Debug.h"
#include "carla/Exception.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/geom/Vector2D.h"

#include <third-party/odrSpiral/odrSpiral.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <vector>

namespace carla {
namespace road {
namespace element {

  namespace {

    // Project each finite piece, preserving its longitudinal parameter rather
    // than interpreting chord length as distance along the road.
    void ProjectPiece(const Geometry &geometry, const geom::Location &query,
        double first_s, double last_s, double &best_s, double &best_distance) {
      const auto first = geometry.PosFromDist(first_s).location;
      const auto last = geometry.PosFromDist(last_s).location;
      const double dx = static_cast<double>(last.x) - first.x;
      const double dy = static_cast<double>(last.y) - first.y;
      const double squared_length = dx * dx + dy * dy;
      const double rate = squared_length == 0.0 ? 0.0 : geom::Math::Clamp(
          ((static_cast<double>(query.x) - first.x) * dx +
           (static_cast<double>(query.y) - first.y) * dy) / squared_length, 0.0, 1.0);
      const double s = first_s + rate * (last_s - first_s);
      const auto projected = geometry.PosFromDist(s).location;
      const double distance = std::hypot(
          static_cast<double>(query.x) - projected.x,
          static_cast<double>(query.y) - projected.y);
      if (distance < best_distance) {
        best_distance = distance;
        best_s = s;
      }
    }

    template <typename Elements>
    std::pair<float, float> DistanceToSpline(const Geometry &geometry,
        const geom::Location &query, const Elements &elements) {
      double best_s = 0.0;
      double best_distance = std::numeric_limits<double>::infinity();
      for (const auto &element : elements) {
        const double first_s = std::max(0.0, element.second.first.s);
        const double last_s = std::min(geometry.GetLength(), element.second.second.s);
        if (first_s <= last_s) {
          ProjectPiece(geometry, query, first_s, last_s, best_s, best_distance);
        }
      }
      return {static_cast<float>(best_s), static_cast<float>(best_distance)};
    }

  } // namespace

  void DirectedPoint::ApplyLateralOffset(float lateral_offset) {
    /// @todo Z axis??
    auto normal_x =  std::sin(static_cast<float>(tangent));
    auto normal_y = -std::cos(static_cast<float>(tangent));
    location.x += lateral_offset * normal_x;
    location.y += lateral_offset * normal_y;
  }

  DirectedPoint GeometryLine::PosFromDist(double dist) const {
    DEBUG_ASSERT(_length > 0.0);
    dist = geom::Math::Clamp(dist, 0.0, _length);
    DirectedPoint p(_start_position, _heading);
    p.location.x += static_cast<float>(dist * std::cos(p.tangent));
    p.location.y += static_cast<float>(dist * std::sin(p.tangent));
    return p;
  }

  DirectedPoint GeometryArc::PosFromDist(double dist) const {
    dist = geom::Math::Clamp(dist, 0.0, _length);
    DEBUG_ASSERT(_length > 0.0);
    DEBUG_ASSERT(std::fabs(_curvature) > 1e-15);
    // Form the chord before converting to float. Adding and subtracting the
    // circle centre loses small displacements when the radius is large.
    const double half_angle = 0.5 * dist * _curvature;
    const double sinc = half_angle == 0.0 ? 1.0 : std::sin(half_angle) / half_angle;
    const double chord = dist * sinc;
    DirectedPoint p(_start_position, _heading);
    p.location.x += static_cast<float>(chord * std::cos(_heading + half_angle));
    p.location.y += static_cast<float>(chord * std::sin(_heading + half_angle));
    p.tangent += dist * _curvature;
    return p;
  }

  // helper function for rotating points
  geom::Vector2D RotatebyAngle(double angle, double x, double y) {
    const double cos_a = std::cos(angle);
    const double sin_a = std::sin(angle);
    return geom::Vector2D(
    static_cast<float>(x * cos_a - y * sin_a),
    static_cast<float>(y * cos_a + x * sin_a));
  }

  DirectedPoint GeometrySpiral::PosFromDist(double dist) const {
    dist = geom::Math::Clamp(dist, 0.0, _length);
    DEBUG_ASSERT(_length > 0.0);
    DirectedPoint p(_start_position, _heading);

    const double curve_end = (_curve_end);
    const double curve_start = (_curve_start);
    const double curve_dot = (curve_end - curve_start) / (_length);
    const double s_o = curve_start / curve_dot;
    double s = s_o + dist;

    double x;
    double y;
    double t;
    odrSpiral(s, curve_dot, &x, &y, &t);

    double x_o;
    double y_o;
    double t_o;
    odrSpiral(s_o, curve_dot, &x_o, &y_o, &t_o);

    x = x - x_o;
    y = y - y_o;
    t = t - t_o;

    geom::Vector2D pos = RotatebyAngle(_heading - t_o, x, y);
    p.location.x += pos.x;
    p.location.y += pos.y;
    p.tangent = _heading + t;

    return p;
  }

  std::pair<float, float> GeometrySpiral::DistanceTo(const geom::Location &location) const {
    if (_curve_start == _curve_end) {
      if (_curve_start == 0.0) {
        return GeometryLine(0.0, _length, _heading, _start_position).DistanceTo(location);
      }
      return GeometryArc(0.0, _length, _heading, _start_position, _curve_start).DistanceTo(location);
    }
    // A unit-speed curve with |curvature| <= k differs from each chord by at
    // most k*h*h/8. Bound that error to 0.1 mm, then return the actual curve
    // point at the chord projection's s. Ignoring float roundoff, its distance
    // is within 0.2 mm of the global minimum over all pieces.
    const double max_curvature = std::max(std::abs(_curve_start), std::abs(_curve_end));
    const double step = std::min(0.5, std::sqrt(8e-4 / max_curvature));
    double best_s = 0.0;
    double best_distance = std::numeric_limits<double>::infinity();
    for (double first_s = 0.0; first_s < _length;) {
      const double last_s = std::min(_length, first_s + step);
      ProjectPiece(*this, location, first_s, last_s, best_s, best_distance);
      first_s = last_s;
    }
    return {static_cast<float>(best_s), static_cast<float>(best_distance)};
  }

  DirectedPoint GeometryPoly3::PosFromDist(double dist) const {
    auto result = _rtree.GetNearestNeighbours(
        Rtree::BPoint(static_cast<float>(dist))).front();

    auto &val1 = result.second.first;
    auto &val2 = result.second.second;

    double rate = (val2.s - dist) / (val2.s - val1.s);
    double u = rate * val1.u + (1.0 - rate) * val2.u;
    double v = rate * val1.v + (1.0 - rate) * val2.v;
    double tangent = atan((rate * val1.t + (1.0 - rate) * val2.t)); // ?

    geom::Vector2D pos = RotatebyAngle(_heading, u, v);
    DirectedPoint p(_start_position, _heading + tangent);
    p.location.x += pos.x;
    p.location.y += pos.y;
    return p;
  }

  std::pair<float, float> GeometryPoly3::DistanceTo(const geom::Location &location) const {
    return DistanceToSpline(*this, location,
        _rtree.GetNearestNeighbours(Rtree::BPoint(0.0f), _rtree.GetTreeSize()));
  }

  void GeometryPoly3::PreComputeSpline() {
    // Roughly the interval size in m
    constexpr double interval_size = 0.3;
    const double delta_u = interval_size; // interval between values of u
    double current_s = 0;
    double current_u = 0;
    double last_u = 0;
    double last_v = _poly.Evaluate(current_u);
    double last_s = 0;
    RtreeValue last_val{last_u, last_v, last_s, _poly.Tangent(current_u)};
    while (current_s < _length + delta_u) {
      current_u += delta_u;
      double current_v = _poly.Evaluate(current_u);
      double du = current_u - last_u;
      double dv = current_v - last_v;
      double ds = sqrt(du * du + dv * dv);
      current_s += ds;
      double current_t = _poly.Tangent(current_u);
      RtreeValue current_val{current_u, current_v, current_s, current_t};

      Rtree::BPoint p1(static_cast<float>(last_s));
      Rtree::BPoint p2(static_cast<float>(current_s));
      _rtree.InsertElement(Rtree::BSegment(p1, p2), last_val, current_val);

      last_u = current_u;
      last_v = current_v;
      last_s = current_s;
      last_val = current_val;

    }
  }

  DirectedPoint GeometryParamPoly3::PosFromDist(double dist) const {
    auto result = _rtree.GetNearestNeighbours(
        Rtree::BPoint(static_cast<float>(dist))).front();

    auto &val1 = result.second.first;
    auto &val2 = result.second.second;
    double rate = (val2.s - dist) / (val2.s - val1.s);
    double u = rate * val1.u + (1.0 - rate) * val2.u;
    double v = rate * val1.v + (1.0 - rate) * val2.v;
    double t_u = (rate * val1.t_u + (1.0 - rate) * val2.t_u);
    double t_v = (rate * val1.t_v + (1.0 - rate) * val2.t_v);
    double tangent = atan2(t_v, t_u); // ?

    geom::Vector2D pos = RotatebyAngle(_heading, u, v);
    DirectedPoint p(_start_position, _heading + tangent);
    p.location.x += pos.x;
    p.location.y += pos.y;
    return p;
  }
  std::pair<float, float> GeometryParamPoly3::DistanceTo(const geom::Location &location) const {
    return DistanceToSpline(*this, location,
        _rtree.GetNearestNeighbours(Rtree::BPoint(0.0f), _rtree.GetTreeSize()));
  }

  void GeometryParamPoly3::PreComputeSpline() {
    // Roughly the interval size in m
    constexpr double interval_size = 0.5;
    size_t number_intervals =
        std::max(static_cast<size_t>(_length / interval_size), size_t(5));
    double delta_p = 1.0 / static_cast<double>(number_intervals);
    if (_arcLength) {
        delta_p *= _length;
    }
    double param_p = 0;
    double current_s = 0;
    double last_u = _polyU.Evaluate(param_p);
    double last_v = _polyV.Evaluate(param_p);
    RtreeValue last_val{
        last_u,
        last_v,
        0.0,
        _polyU.Tangent(param_p),
        _polyV.Tangent(param_p) };
    std::vector<RtreeValue> values{last_val};
    values.reserve(number_intervals + 1u);
    for(size_t i = 0; i < number_intervals; ++i) {
      param_p += delta_p;
      double current_u = _polyU.Evaluate(param_p);
      double current_v = _polyV.Evaluate(param_p);
      double du = current_u - last_u;
      double dv = current_v - last_v;
      double ds = sqrt(du * du + dv * dv);
      current_s += ds;
      double current_t_u = _polyU.Tangent(param_p);
      double current_t_v = _polyV.Tangent(param_p);
      RtreeValue current_val{
          current_u,
          current_v,
          current_s,
          current_t_u,
          current_t_v };

      values.push_back(current_val);

      last_u = current_u;
      last_v = current_v;

      if(_arcLength && current_s > _length){
        break;
      }
    }
    // Normalized p covers the full interval [0, 1]. Chord sums underestimate
    // the curve's length; map them onto its declared s interval so the final
    // segment never extrapolates beyond p=1 at the geometry endpoint.
    const double scale = !_arcLength && current_s > 0.0 ? _length / current_s : 1.0;
    for (auto &value : values) {
      value.s *= scale;
    }
    for (size_t i = 1u; i < values.size(); ++i) {
      const auto &first = values[i - 1u];
      const auto &second = values[i];
      _rtree.InsertElement(Rtree::BSegment(
          Rtree::BPoint(static_cast<float>(first.s)),
          Rtree::BPoint(static_cast<float>(second.s))), first, second);
    }
  }
} // namespace element
} // namespace road
} // namespace carla
