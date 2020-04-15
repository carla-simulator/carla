// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Memory.h"
#include "carla/client/Waypoint.h"
#include "carla/geom/Transform.h"
#include "carla/geom/BoundingBox.h"

#include "carla/road/element/RoadInfoSignal.h"

#include <string>

namespace carla {
namespace client {

  /// Class containing a reference to RoadInfoSignal
  class Landmark {
  public:

    SharedPtr<Waypoint> GetWaypoint() const {
      return _waypoint;
    }

    const geom::Transform &GetTransform() const {
      return _signal->GetSignal()->GetTransform();
    }

    road::RoadId GetRoadId() const {
      return _signal->GetRoadId();
    }

    double GetDistance() const {
      return _distance_from_search;
    }

    double GetS() const {
      return _signal->GetS();
    }

    double GetT() const {
      return _signal->GetT();
    }

    std::string GetId() const {
      return _signal->GetSignalId();
    }

    std::string GetName() const {
      return _signal->GetSignal()->GetName();
    }

    bool IsDynamic() const {
      return _signal->IsDynamic();
    }

    road::SignalOrientation GetOrientation() const {
      return _signal->GetOrientation();
    }

    double GetZOffset() const {
      return _signal->GetSignal()->GetZOffset();
    }

    std::string GetCountry() const {
      return _signal->GetSignal()->GetCountry();
    }

    std::string GetType() const {
      return _signal->GetSignal()->GetType();
    }

    std::string GetSubType() const {
      return _signal->GetSignal()->GetSubtype();
    }

    double GetValue() const {
      return _signal->GetSignal()->GetValue();
    }

    std::string GetUnit() const {
      return _signal->GetSignal()->GetUnit();
    }

    double GetHeight() const {
      return _signal->GetSignal()->GetHeight();
    }

    double GetWidth() const {
      return _signal->GetSignal()->GetWidth();
    }

    std::string GetText() const {
      return _signal->GetSignal()->GetText();
    }

    double GethOffset() const {
      return _signal->GetSignal()->GetHOffset();
    }

    double GetPitch() const {
      return _signal->GetSignal()->GetPitch();
    }

    double GetRoll() const {
      return _signal->GetSignal()->GetRoll();
    }

    const auto &GetValidities() const {
      return _signal->GetValidities();
    }

  private:

    friend Waypoint;
    friend Map;

    Landmark(
        SharedPtr<Waypoint> waypoint,
        SharedPtr<const Map> parent,
        const road::element::RoadInfoSignal* signal,
        double distance_from_search = 0)
      : _waypoint(waypoint),
        _parent(parent),
        _signal(signal),
        _distance_from_search(distance_from_search) {}

    /// waypoint where the signal is affecting
    SharedPtr<Waypoint> _waypoint;

    SharedPtr<const Map> _parent;

    const road::element::RoadInfoSignal* _signal;

    double _distance_from_search;
  };

} // client
} // carla
