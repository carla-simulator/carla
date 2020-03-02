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

  /// Using OpenDRIVE 1.5M (6.10 Country Codes)
  ///
  class LandmarkType {
  public:
    static const std::string Danger();     // = "101" // danger types from 101 to 151
    static const std::string LanesMerging(); // = "121";
    static const std::string CautionPedestrian(); // = "133";
    static const std::string CautionBicycle(); // = "138";
    static const std::string LevelCrossing(); // = "150";
    static const std::string YieldSign(); // = "205";
    static const std::string StopSign(); // = "206";
    static const std::string MandatoryTurnDirection(); // = "209" // Left, right or forward
    static const std::string MandatoryLeftRightDirection(); // = "211";
    static const std::string TwoChoiceTurnDirection(); // = "214" // Forward-left, forward-right, left-right
    static const std::string Roundabout(); // = "215";
    static const std::string PassRightLeft(); // = "222";
    static const std::string AccessForbidden(); // = "250";
    static const std::string AccessForbiddenMotorvehicles(); // = "251";
    static const std::string AccessForbiddenTrucks(); // = "253";
    static const std::string AccessForbiddenBicycle(); // = "254";
    static const std::string AccessForbiddenWeight(); // = "263";
    static const std::string AccessForbiddenWidth(); // = "264";
    static const std::string AccessForbiddenHeight(); // = "265";
    static const std::string AccessForbiddenWrongDirection(); // = "267";
    static const std::string ForbiddenUTurn(); // = "272";
    static const std::string MaximumSpeed(); // = "274";
    static const std::string ForbiddenOvertakingMotorvehicles(); // = "276";
    static const std::string ForbiddenOvertakingTrucks(); // = "277";
    static const std::string AbsoluteNoStop(); // = "283";
    static const std::string RestrictedStop(); // = "286";
    static const std::string HasWayNextIntersection(); // = "301";
    static const std::string PriorityWay(); // = "306";
    static const std::string PriorityWayEnd(); // = "307";
    static const std::string CityBegin(); // = "310";
    static const std::string CityEnd(); // = "311";
    static const std::string Highway(); // = "330";
    static const std::string DeadEnd(); // = "357";
    static const std::string RecomendedSpeed(); // = "380";
    static const std::string RecomendedSpeedEnd(); // = "381";
  };

  /// Class containing a reference to RoadInfoSignal
  class Landmark : private MovableNonCopyable {
  public:

    SharedPtr<Waypoint> GetWaypoint() const {
      return _waypoint;
    }

    const geom::Transform &GetTransform() const {
      return _signal->GetSignal()->GetTransform();
    }

    road::RoadId GetRoadId() const {
      return _signal->GetSignal()->GetRoadId();
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

    Landmark(
        SharedPtr<Waypoint> waypoint,
        const road::element::RoadInfoSignal* signal,
        double distance_from_search = 0)
      : _waypoint(waypoint),
        _signal(signal),
        _distance_from_search(distance_from_search) {}

    SharedPtr<Waypoint> _waypoint; /// waypoint where the signal is affecting

    const road::element::RoadInfoSignal* _signal;

    double _distance_from_search;
  };

} // client
} // carla
