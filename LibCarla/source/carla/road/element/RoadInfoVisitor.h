// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace carla {
namespace road {
namespace element {

  class RoadInfo;
  class RoadInfoElevation;
  class RoadInfoGeometry;
  class RoadInfoLane;
  class RoadInfoLaneAccess;
  class RoadInfoLaneBorder;
  class RoadInfoLaneHeight;
  class RoadInfoLaneMaterial;
  class RoadInfoLaneOffset;
  class RoadInfoLaneRule;
  class RoadInfoLaneVisibility;
  class RoadInfoLaneWidth;
  class RoadInfoMarkRecord;
  class RoadInfoMarkTypeLine;
  class RoadInfoSpeed;
  class RoadInfoCrosswalk;
  class RoadInfoSignal;
  class RoadInfoStencil;

  class RoadInfoVisitor {
  public:

    RoadInfoVisitor() = default;
    virtual ~RoadInfoVisitor() = default;

    RoadInfoVisitor(const RoadInfoVisitor &) = default;
    RoadInfoVisitor(RoadInfoVisitor &&) = default;

    RoadInfoVisitor &operator=(const RoadInfoVisitor &) = default;
    RoadInfoVisitor &operator=(RoadInfoVisitor &&) = default;

    virtual void Visit(RoadInfoElevation &) {}
    virtual void Visit(RoadInfoGeometry &) {}
    virtual void Visit(RoadInfoLane &) {}
    virtual void Visit(RoadInfoLaneAccess &) {}
    virtual void Visit(RoadInfoLaneBorder &) {}
    virtual void Visit(RoadInfoLaneHeight &) {}
    virtual void Visit(RoadInfoLaneMaterial &) {}
    virtual void Visit(RoadInfoLaneOffset &) {}
    virtual void Visit(RoadInfoLaneRule &) {}
    virtual void Visit(RoadInfoLaneVisibility &) {}
    virtual void Visit(RoadInfoLaneWidth &) {}
    virtual void Visit(RoadInfoMarkRecord &) {}
    virtual void Visit(RoadInfoMarkTypeLine &) {}
    virtual void Visit(RoadInfoSpeed &) {}
    virtual void Visit(RoadInfoCrosswalk &) {}
    virtual void Visit(RoadInfoSignal &) {}
    virtual void Visit(RoadInfoStencil &) {}
  };

} // namespace element
} // namespace road
} // namespace carla
