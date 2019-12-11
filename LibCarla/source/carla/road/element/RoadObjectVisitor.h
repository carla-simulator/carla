// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace road {
namespace element {

  class RoadObject;
  class RoadObjectCrosswalk;

  class RoadObjectVisitor {
  public:

    RoadObjectVisitor() = default;
    virtual ~RoadObjectVisitor() = default;

    RoadObjectVisitor(const RoadObjectVisitor &) = default;
    RoadObjectVisitor(RoadObjectVisitor &&) = default;

    RoadObjectVisitor &operator=(const RoadObjectVisitor &) = default;
    RoadObjectVisitor &operator=(RoadObjectVisitor &&) = default;

    virtual void Visit(RoadObjectCrosswalk &) {}
  };

} // namespace element
} // namespace road
} // namespace carla
