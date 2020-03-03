// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Landmark.h"

namespace carla {
namespace client {

  const std::string LandmarkType::Danger() {
    return "101";
  } // danger types from 101 to 151
  const std::string LandmarkType::LanesMerging() {
    return "121";
  } // = "121";
  const std::string LandmarkType::CautionPedestrian() {
    return "133";
  } // = "133";
  const std::string LandmarkType::CautionBicycle() {
    return "138";
  } // = "138";
  const std::string LandmarkType::LevelCrossing() {
    return "150";
  } // = "150";
  const std::string LandmarkType::YieldSign() {
    return "205";
  } // = "205";
  const std::string LandmarkType::StopSign() {
    return "206";
  } // = "206";
  const std::string LandmarkType::MandatoryTurnDirection() {
    return "209";
  } // = "209"; // Left, right or forward
  const std::string LandmarkType::MandatoryLeftRightDirection() {
    return "211";
  } // = "211";
  const std::string LandmarkType::TwoChoiceTurnDirection() {
    return "214";
  } // = "214"; // Forward-left, forward-right, left-right
  const std::string LandmarkType::Roundabout() {
    return "215";
  } // = "215";
  const std::string LandmarkType::PassRightLeft() {
    return "222";
  } // = "222";
  const std::string LandmarkType::AccessForbidden() {
    return "250";
  } // = "250";
  const std::string LandmarkType::AccessForbiddenMotorvehicles() {
    return "251";
  } // = "251";
  const std::string LandmarkType::AccessForbiddenTrucks() {
    return "253";
  } // = "253";
  const std::string LandmarkType::AccessForbiddenBicycle() {
    return "254";
  } // = "254";
  const std::string LandmarkType::AccessForbiddenWeight() {
    return "263";
  } // = "263";
  const std::string LandmarkType::AccessForbiddenWidth() {
    return "264";
  } // = "264";
  const std::string LandmarkType::AccessForbiddenHeight() {
    return "265";
  } // = "265";
  const std::string LandmarkType::AccessForbiddenWrongDirection() {
    return "267";
  } // = "267";
  const std::string LandmarkType::ForbiddenUTurn() {
    return "272";
  } // = "272";
  const std::string LandmarkType::MaximumSpeed() {
    return "274";
  } // = "274";
  const std::string LandmarkType::ForbiddenOvertakingMotorvehicles() {
    return "276";
  } // = "276";
  const std::string LandmarkType::ForbiddenOvertakingTrucks() {
    return "277";
  } // = "277";
  const std::string LandmarkType::AbsoluteNoStop() {
    return "283";
  } // = "283";
  const std::string LandmarkType::RestrictedStop() {
    return "286";
  } // = "286";
  const std::string LandmarkType::HasWayNextIntersection() {
    return "301";
  } // = "301";
  const std::string LandmarkType::PriorityWay() {
    return "306";
  } // = "306";
  const std::string LandmarkType::PriorityWayEnd() {
    return "307";
  } // = "307";
  const std::string LandmarkType::CityBegin() {
    return "310";
  } // = "310";
  const std::string LandmarkType::CityEnd() {
    return "311";
  } // = "311";
  const std::string LandmarkType::Highway() {
    return "330";
  } // = "330";
  const std::string LandmarkType::DeadEnd() {
    return "357";
  } // = "357";
  const std::string LandmarkType::RecomendedSpeed() {
    return "380";
  } // = "380";
  const std::string LandmarkType::RecomendedSpeedEnd() {
    return "381";
  } // = "381";


}
}
