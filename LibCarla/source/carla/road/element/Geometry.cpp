// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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

#include <boost/array.hpp>
#include <boost/math/tools/rational.hpp>

#include <odrSpiral/odrSpiral.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace carla {
namespace road {
namespace element {

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
    const double radius = 1.0 / _curvature;
    constexpr double pi_half = geom::Math::Pi<double>() / 2.0;
    DirectedPoint p(_start_position, _heading);
    p.location.x += static_cast<float>(radius * std::cos(p.tangent + pi_half));
    p.location.y += static_cast<float>(radius * std::sin(p.tangent + pi_half));
    p.tangent += dist * _curvature;
    p.location.x -= static_cast<float>(radius * std::cos(p.tangent + pi_half));
    p.location.y -= static_cast<float>(radius * std::sin(p.tangent + pi_half));
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

  /// @todo
  std::pair<float, float> GeometrySpiral::DistanceTo(const geom::Location &location) const {
    // Not analytic, discretize and find nearest point
    // throw_exception(std::runtime_error("not implemented"));
    return {location.x - _start_position.x, location.y - _start_position.y};
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

  std::pair<float, float> GeometryPoly3::DistanceTo(const geom::Location & /*p*/) const {
    // No analytical expression (Newton-Raphson?/point search)
    // throw_exception(std::runtime_error("not implemented"));
    return {_start_position.x, _start_position.y};
  }

  void GeometryPoly3::PreComputeSpline() {
    const double delta_u = 0.01; // interval between values of u
    double current_s = 0;
    double current_u = 0;
    double last_u = 0;
    double last_v = 0;
    double last_s = 0;
    RtreeValue last_val;
    while (current_s < _length + delta_u) {
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

      current_u += delta_u;
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
  std::pair<float, float> GeometryParamPoly3::DistanceTo(const geom::Location &) const {
    // No analytical expression (Newton-Raphson?/point search)
    // throw_exception(std::runtime_error("not implemented"));
    return {_start_position.x, _start_position.y};
  }

  void GeometryParamPoly3::PreComputeSpline() {
    size_t number_intervals = 1000;
    double delta_p = 1.0 / number_intervals;
    if (_arcLength) {
        delta_p *= _length;
    }
    double param_p = 0;
    double current_s = 0;
    double last_u = 0;
    double last_v = 0;
    double last_s = 0;
    RtreeValue last_val;
    for(size_t i = 0; i < number_intervals; ++i) {
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

      Rtree::BPoint p1(static_cast<float>(last_s));
      Rtree::BPoint p2(static_cast<float>(current_s));
      _rtree.InsertElement(Rtree::BSegment(p1, p2), last_val, current_val);

      last_u = current_u;
      last_v = current_v;
      last_s = current_s;
      last_val = current_val;

      param_p += delta_p;
      if(current_s > _length){
        break;
      }
    }
  }
} // namespace element
} // namespace road
} // namespace carla
