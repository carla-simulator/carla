// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/Waypoint.h"

#include "carla/geom/Transform.h"
#include "carla/geom/BoundingBox.h"

#include <string>

namespace carla {
namespace client {

  /// Using OpenDRIVE 1.5M (6.10 Country Codes)
  ///
  enum class LandmarkType : uint32_t {
    Unknown,
    Danger = 101, // danger types from 101 to 151
    LanesMerging = 121,
    CautionPedestrian = 133,
    CautionBicycle = 138,
    LevelCrossing = 150,
    Yield = 205,
    Stop = 206,
    MandatoryTurnDirection = 209, // Left, right or forward
    MandatoryLeftRightDirection = 211,
    TwoChoiceTurnDirection = 214, // Forward-left, forward-right, left-right
    Roundabout = 215,
    PassRightLeft = 222,
    AccessForbidden = 250,
    AccessForbiddenMotorvehicles = 251,
    AccessForbiddenTrucks = 253,
    AccessForbiddenBicycle = 254,
    AccessForbiddenWeight = 263,
    AccessForbiddenWidth = 264,
    AccessForbiddenHeight = 265,
    AccessForbiddenWrongDirection = 267,
    ForbiddenUTurn = 272,
    MaximumSpeed = 274,
    ForbiddenOvertakingMotorvehicles = 276,
    ForbiddenOvertakingTrucks = 277,
    AbsoluteNoStop = 283,
    RestrictedStop = 286,
    HasWayNextIntersection = 301,
    PriorityWay = 306,
    PriorityWayEnd = 307,
    CityBegin = 310,
    CityEnd = 311,
    Highway = 330,
    DeadEnd = 357,
    RecomendedSpeed = 380,
    RecomendedSpeedEnd = 381
  };

  class Landmark {
  public:

    SharedPtr<const Waypoint> GetWaypoint() const {
      return _waypoint;
    }

    const geom::Transform &GetTransform() const {
      return _transform;
    }

    LandmarkType GetType() const {
      return LandmarkType::Unknown;
    }

    uint32_t GetSubType() const {
      return 0;
    }

    std::string GetCountryCode() const {
      return "OpenDRIVE";
    }

    double GetValue() const {
      return 0.0;
    }

  private:

    friend Waypoint;

    Landmark(geom::Transform transform, SharedPtr<const Waypoint> waypoint/*, road::Landmarkinfo...*/)
        : _transform(transform), _waypoint(waypoint) {}

    geom::Transform _transform;

    SharedPtr<const Waypoint> _waypoint;

    // road::Landmarkinfo...
  };

} // client
} // carla
