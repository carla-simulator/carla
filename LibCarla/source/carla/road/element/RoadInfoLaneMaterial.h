// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h"
#include <string>

namespace carla {
namespace road {
namespace element {

  class RoadInfoLaneMaterial final : public RoadInfo {
  public:

    RoadInfoLaneMaterial(
        double s,   // start position relative to the position of the preceding
                    // lane section
        std::string surface,
        double friction,
        double roughness)
      : RoadInfo(s),
        _surface(std::move(surface)),
        _friction(friction),
        _roughness(roughness) {}

    void AcceptVisitor(RoadInfoVisitor &v) override final {
      v.Visit(*this);
    }

    const std::string &GetSurface() const {
      return _surface;
    }

    double GetFriction() const {
      return _friction;
    }

    double GetRoughness() const {
      return _roughness;
    }

  private:

    const std::string _surface;

    const double _friction;

    const double _roughness;
  };

} // namespace element
} // namespace road
} // namespace carla
