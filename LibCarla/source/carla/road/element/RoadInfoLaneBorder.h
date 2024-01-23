// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include "carla/geom/CubicPolynomial.h"

namespace carla {
namespace road {
namespace element {

  /// Instead of describing lanes by their width entries and, thus, invariably
  /// depending on influences of inner lanes on outer lanes, it might be more
  /// convenient to just describe the outer border of each lane independent of
  /// any inner lanes’ parameters. Especially in cases where road data is
  /// derived from measurements, this type of definition will provide a more
  /// convenient method without the need to tesselate road sections into too
  /// many parts. Note. Lane borders and widths are mutually exclusive.
  class RoadInfoLaneBorder final : public RoadInfo {
  public:

    RoadInfoLaneBorder(
        double s,
        double a,
        double b,
        double c,
        double d)
      : RoadInfo(s),
        _border(a, b, c, d, s) {}

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    const geom::CubicPolynomial &GetPolynomial() const {
      return _border;
    }

  private:

    const geom::CubicPolynomial _border;
  };

} // namespace element
} // namespace road
} // namespace carla
