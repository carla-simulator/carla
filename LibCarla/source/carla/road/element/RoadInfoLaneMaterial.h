// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
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
        float s,   // start position relative to the position of the preceding
                   // lane section
        std::string surface,
        float friction,
        float roughness)
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

    float GetFriction() const {
      return _friction;
    }

    float GetRoughness() const {
      return _roughness;
    }

  private:

    const std::string _surface;

    const float _friction;

    const float _roughness;
  };

} // namespace element
} // namespace road
} // namespace carla
