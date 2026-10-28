// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "PlanView.h"

#include <algorithm>
#include <cmath>

namespace twinmodel {

  namespace {

    struct Vec2 {
      double x = 0.0;
      double y = 0.0;
    };

    Vec2 Normalise(Vec2 v) {
      const double n = std::hypot(v.x, v.y);
      return (n > 1e-12) ? Vec2{v.x / n, v.y / n} : Vec2{1.0, 0.0};
    }

    std::vector<Vec2> Dedupe(const geom::Line3 &line) {
      std::vector<Vec2> out;
      for (const geom::Point3 &p : line) {
        if (out.empty() || std::hypot(p.x - out.back().x, p.y - out.back().y) > 1e-9) {
          out.push_back(Vec2{p.x, p.y});
        }
      }
      return out;
    }

    /// Cubic Hermite from p0 (unit tangent t0) to p1 (unit tangent t1), tangent
    /// magnitudes the chord length, in the frame rotated to t0.
    void HermiteSegment(const Vec2 &p0, const Vec2 &p1, const Vec2 &t0, const Vec2 &t1,
                        double &hdg, double &chord, std::array<double, 8> &coeffs) {
      const Vec2 delta{p1.x - p0.x, p1.y - p0.y};
      chord = std::hypot(delta.x, delta.y);
      hdg = std::atan2(t0.y, t0.x);
      const double c = std::cos(hdg);
      const double s = std::sin(hdg);
      // world -> local, u along the start tangent
      const Vec2 p1l{c * delta.x + s * delta.y, -s * delta.x + c * delta.y};
      const Vec2 m0l{chord, 0.0};
      const Vec2 m1l{(c * t1.x + s * t1.y) * chord, (-s * t1.x + c * t1.y) * chord};
      const Vec2 cc{3.0 * p1l.x - 2.0 * m0l.x - m1l.x, 3.0 * p1l.y - 2.0 * m0l.y - m1l.y};
      const Vec2 d{-2.0 * p1l.x + m0l.x + m1l.x, -2.0 * p1l.y + m0l.y + m1l.y};
      coeffs = {0.0, m0l.x, cc.x, d.x, 0.0, m0l.y, cc.y, d.y};
    }

    void Evaluate(const std::array<double, 8> &c, double p, double &u, double &v) {
      const double p2 = p * p;
      const double p3 = p2 * p;
      u = c[0] + c[1] * p + c[2] * p2 + c[3] * p3;
      v = c[4] + c[5] * p + c[6] * p2 + c[7] * p3;
    }

    /// Largest distance of the fitted cubic from the chord it spans.
    double ChordOffset(const std::array<double, 8> &coeffs, int samples = 33) {
      double ex = 0.0;
      double ey = 0.0;
      Evaluate(coeffs, 1.0, ex, ey);
      const double length = std::hypot(ex, ey);
      double worst = 0.0;
      for (int i = 0; i < samples; ++i) {
        const double p = static_cast<double>(i) / static_cast<double>(samples - 1);
        double u = 0.0;
        double v = 0.0;
        Evaluate(coeffs, p, u, v);
        if (length < 1e-9) {
          worst = std::max(worst, std::hypot(u, v));
          continue;
        }
        double t = (u * ex + v * ey) / (length * length);
        t = std::clamp(t, 0.0, 1.0);
        worst = std::max(worst, std::hypot(u - t * ex, v - t * ey));
      }
      return worst;
    }

    double CubicLength(const std::array<double, 8> &coeffs, int steps) {
      double total = 0.0;
      double pu = 0.0;
      double pv = 0.0;
      Evaluate(coeffs, 0.0, pu, pv);
      for (int i = 1; i <= steps; ++i) {
        double u = 0.0;
        double v = 0.0;
        Evaluate(coeffs, static_cast<double>(i) / static_cast<double>(steps), u, v);
        total += std::hypot(u - pu, v - pv);
        pu = u;
        pv = v;
      }
      return total;
    }

  } // namespace

  std::vector<Geom> FitPlanView(const geom::Line3 &line, double max_offset) {
    std::vector<Geom> out;
    const std::vector<Vec2> pts = Dedupe(line);
    if (pts.size() < 2u) {
      return out;
    }
    if (pts.size() == 2u) {
      Geom g;
      g.x = pts[0].x;
      g.y = pts[0].y;
      g.hdg = std::atan2(pts[1].y - pts[0].y, pts[1].x - pts[0].x);
      g.length = std::hypot(pts[1].x - pts[0].x, pts[1].y - pts[0].y);
      g.is_line = true;
      out.push_back(g);
      return out;
    }

    const std::size_t n = pts.size();
    std::vector<Vec2> chord(n - 1u);
    for (std::size_t i = 0u; i + 1u < n; ++i) {
      chord[i] = Normalise(Vec2{pts[i + 1u].x - pts[i].x, pts[i + 1u].y - pts[i].y});
    }

    // Catmull-Rom tangent directions; the ends follow their own chord
    std::vector<Vec2> tangent(n);
    tangent[0] = chord.front();
    tangent[n - 1u] = chord.back();
    for (std::size_t i = 1u; i + 1u < n; ++i) {
      tangent[i] = Normalise(Vec2{pts[i + 1u].x - pts[i - 1u].x, pts[i + 1u].y - pts[i - 1u].y});
    }
    std::vector<Vec2> t_out = tangent;
    std::vector<Vec2> t_in = tangent;
    std::vector<char> corner(n, 0);
    corner.front() = 1;
    corner.back() = 1;

    auto turn_at = [&](std::size_t k) {
      const Vec2 &a = chord[k - 1u];
      const Vec2 &b = chord[k];
      return std::abs(std::atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y));
    };

    // lock the vertices whose rounding bows too far off the polyline
    for (int pass = 0; pass < 4; ++pass) {
      bool locked = false;
      for (std::size_t i = 0u; i + 1u < n; ++i) {
        double hdg = 0.0;
        double chord_length = 0.0;
        std::array<double, 8> coeffs{};
        HermiteSegment(pts[i], pts[i + 1u], t_out[i], t_in[i + 1u], hdg, chord_length, coeffs);
        if (ChordOffset(coeffs) <= max_offset) {
          continue;
        }
        std::vector<std::size_t> candidates;
        if (corner[i] == 0) candidates.push_back(i);
        if (corner[i + 1u] == 0) candidates.push_back(i + 1u);
        if (candidates.empty()) {
          continue;
        }
        std::size_t pick = candidates.front();
        for (std::size_t candidate : candidates) {
          if (turn_at(candidate) > turn_at(pick)) {
            pick = candidate;
          }
        }
        corner[pick] = 1;
        t_out[pick] = chord[pick];
        t_in[pick] = chord[pick - 1u];
        locked = true;
      }
      if (!locked) {
        break;
      }
    }

    double s = 0.0;
    for (std::size_t i = 0u; i + 1u < n; ++i) {
      double hdg = 0.0;
      double chord_length = 0.0;
      std::array<double, 8> coeffs{};
      HermiteSegment(pts[i], pts[i + 1u], t_out[i], t_in[i + 1u], hdg, chord_length, coeffs);
      Geom g;
      g.s = s;
      g.x = pts[i].x;
      g.y = pts[i].y;
      g.hdg = hdg;
      g.coeffs = coeffs;
      if (corner[i] != 0 && corner[i + 1u] != 0) {
        g.is_line = true;
        g.length = chord_length;
      } else {
        g.is_line = false;
        g.length = CubicLength(coeffs, std::max(64, static_cast<int>(chord_length / 0.05)));
      }
      s += g.length;
      out.push_back(g);
    }
    return out;
  }

} // namespace twinmodel
