// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoGeometry : public RoadInfo {
  public:

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    RoadInfoGeometry(float s, std::unique_ptr<Geometry> &&geom)
      : RoadInfo(s),
        _geom(std::move(geom)) {}

    const std::unique_ptr<Geometry> _geom;

  };

} // namespace element
} // namespace road
} // namespace carla
