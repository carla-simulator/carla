// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Transform.h"
#include "carla/road/element/RoadInfoStencil.h"

#include <string>

namespace carla {
namespace client {

  /// Class containing a reference to RoadInfoStencil (exposed as RoadMark to users)
  class RoadMark {
  public:

    SharedPtr<Waypoint> GetWaypoint() const {
      return _waypoint;
    }

    const geom::Transform &GetTransform() const {
      return _stencil->GetStencil()->GetTransform();
    }

    road::RoadId GetRoadId() const {
      return _stencil->GetRoadId();
    }

    double GetDistance() const {
      return _distance_from_search;
    }

    double GetS() const {
      return _stencil->GetS();
    }

    double GetT() const {
      return _stencil->GetT();
    }

    std::string GetId() const {
      return _stencil->GetStencilId();
    }

    std::string GetName() const {
      return _stencil->GetStencil()->GetName();
    }

    road::StencilOrientation GetOrientation() const {
      return _stencil->GetOrientation();
    }

    double GetZOffset() const {
      return _stencil->GetStencil()->GetZOffset();
    }

    std::string GetType() const {
      return _stencil->GetStencil()->GetType();
    }

    double GetLength() const {
      return _stencil->GetStencil()->GetLength();
    }

    double GetWidth() const {
      return _stencil->GetStencil()->GetWidth();
    }

    double GetHeading() const {
      return _stencil->GetStencil()->GetHeading();
    }

    double GetPitch() const {
      return _stencil->GetStencil()->GetPitch();
    }

    double GetRoll() const {
      return _stencil->GetStencil()->GetRoll();
    }

    const auto &GetValidities() const {
      return _stencil->GetValidities();
    }

  private:

    friend Waypoint;
    friend Map;

    RoadMark(
        SharedPtr<Waypoint> waypoint,
        SharedPtr<const Map> parent,
        const road::element::RoadInfoStencil* stencil,
        double distance_from_search = 0)
      : _waypoint(waypoint),
        _parent(parent),
        _stencil(stencil),
        _distance_from_search(distance_from_search) {}

    /// waypoint where the road mark is affecting
    SharedPtr<Waypoint> _waypoint;

    SharedPtr<const Map> _parent;

    const road::element::RoadInfoStencil* _stencil;

    double _distance_from_search;
  };

} // client
} // carla

