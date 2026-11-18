// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <array>
#include <vector>

#include "Geometry.h"

namespace twinmodel {

  /// A vertex whose rounding would pull the curve further than this off the
  /// polyline becomes a corner instead; export/xodr.py's MAX_PLANVIEW_OFFSET.
  constexpr double kMaxPlanViewOffset = 0.25;

  struct Geom {
    double s = 0.0;
    double x = 0.0;
    double y = 0.0;
    double hdg = 0.0;
    double length = 0.0;
    bool is_line = true;
    /// aU bU cU dU aV bV cV dV, with aU = aV = bV = 0 by construction.
    std::array<double, 8> coeffs{};
  };

  /// Fit a polyline as a piecewise cubic paramPoly3 planview. Tangent
  /// directions are Catmull-Rom, magnitudes the chord length, each segment
  /// expressed in the frame rotated to its start tangent so hdg stays G1.
  std::vector<Geom> FitPlanView(const geom::Line3 &line,
                                double max_offset = kMaxPlanViewOffset);

} // namespace twinmodel
