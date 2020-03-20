// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy; see <https://opensource.org/licenses/MIT>.

#include "SignalType.h"

#include <vector>
#include <algorithm>

namespace carla {
namespace road {

  const std::string SignalType::Danger() {
    return "101";
  } // danger types from 101 to 151
  const std::string SignalType::LanesMerging() {
    return "121";
  } // = "121";
  const std::string SignalType::CautionPedestrian() {
    return "133";
  } // = "133";
  const std::string SignalType::CautionBicycle() {
    return "138";
  } // = "138";
  const std::string SignalType::LevelCrossing() {
    return "150";
  } // = "150";
  const std::string SignalType::YieldSign() {
    return "205";
  } // = "205";
  const std::string SignalType::StopSign() {
    return "206";
  } // = "206";
  const std::string SignalType::MandatoryTurnDirection() {
    return "209";
  } // = "209"; // Left, right or forward
  const std::string SignalType::MandatoryLeftRightDirection() {
    return "211";
  } // = "211";
  const std::string SignalType::TwoChoiceTurnDirection() {
    return "214";
  } // = "214"; // Forward-left, forward-right, left-right
  const std::string SignalType::Roundabout() {
    return "215";
  } // = "215";
  const std::string SignalType::PassRightLeft() {
    return "222";
  } // = "222";
  const std::string SignalType::AccessForbidden() {
    return "250";
  } // = "250";
  const std::string SignalType::AccessForbiddenMotorvehicles() {
    return "251";
  } // = "251";
  const std::string SignalType::AccessForbiddenTrucks() {
    return "253";
  } // = "253";
  const std::string SignalType::AccessForbiddenBicycle() {
    return "254";
  } // = "254";
  const std::string SignalType::AccessForbiddenWeight() {
    return "263";
  } // = "263";
  const std::string SignalType::AccessForbiddenWidth() {
    return "264";
  } // = "264";
  const std::string SignalType::AccessForbiddenHeight() {
    return "265";
  } // = "265";
  const std::string SignalType::AccessForbiddenWrongDirection() {
    return "267";
  } // = "267";
  const std::string SignalType::ForbiddenUTurn() {
    return "272";
  } // = "272";
  const std::string SignalType::MaximumSpeed() {
    return "274";
  } // = "274";
  const std::string SignalType::ForbiddenOvertakingMotorvehicles() {
    return "276";
  } // = "276";
  const std::string SignalType::ForbiddenOvertakingTrucks() {
    return "277";
  } // = "277";
  const std::string SignalType::AbsoluteNoStop() {
    return "283";
  } // = "283";
  const std::string SignalType::RestrictedStop() {
    return "286";
  } // = "286";
  const std::string SignalType::HasWayNextIntersection() {
    return "301";
  } // = "301";
  const std::string SignalType::PriorityWay() {
    return "306";
  } // = "306";
  const std::string SignalType::PriorityWayEnd() {
    return "307";
  } // = "307";
  const std::string SignalType::CityBegin() {
    return "310";
  } // = "310";
  const std::string SignalType::CityEnd() {
    return "311";
  } // = "311";
  const std::string SignalType::Highway() {
    return "330";
  } // = "330";
  const std::string SignalType::DeadEnd() {
    return "357";
  } // = "357";
  const std::string SignalType::RecomendedSpeed() {
    return "380";
  } // = "380";
  const std::string SignalType::RecomendedSpeedEnd() {
    return "381";
  } // = "381";

  bool SignalType::IsTrafficLight(const std::string &type) {
    // Types corresponding to traffic lights
    const std::vector<std::string> traffic_light_types =
        {"1000001", "1000002", "1000009", "1000010", "1000011",
         "1000007", "1000014", "1000015", "1000016", "1000017",
         "1000018", "1000019", "1000013", "1000020", "1000008",
         "1000012", "F", "W", "A"};

    auto it = std::find(
        traffic_light_types.begin(), traffic_light_types.end(), type);
    if (it != traffic_light_types.end()){
      return true;
    } else {
      return false;
    }
  }

}
}
