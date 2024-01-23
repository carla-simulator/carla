// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

namespace carla {
namespace road {

  /// Using OpenDRIVE 1.5M (6.10 Country Codes)
  ///
  class SignalType {
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

    static bool IsTrafficLight(const std::string &type);
  };

} // road
} // carla
