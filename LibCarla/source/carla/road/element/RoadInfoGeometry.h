// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfo.h"

namespace carla {
namespace road {
namespace element {

  class RoadInfoGeometry final : public RoadInfo {
  public:

    RoadInfoGeometry(double s, std::unique_ptr<Geometry> &&geom)
      : RoadInfo(s),
        _geom(std::move(geom)) {
      DEBUG_ASSERT(_geom != nullptr);
    }

    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this);
    }

    const Geometry &GetGeometry() const {
      return *_geom;
    }

  private:

    const std::unique_ptr<const Geometry> _geom;
  };

} // namespace element
} // namespace road
} // namespace carla
